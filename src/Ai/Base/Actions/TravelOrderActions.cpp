/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TravelOrderActions.h"

#include <iomanip>
#include <sstream>

#include "Event.h"
#include "LastMovementValue.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Timer.h"
#include "Transport.h"
#include "TravelNode.h"
#include "TravelOrderValue.h"

namespace
{
    // Arrival is declared strictly above the funnel's short-stop
    // (targetPosRecalcDistance, default 0.1y) so the driver, not the
    // funnel, always terminates the order.
    constexpr float ARRIVE_DISTANCE = 10.0f;
    // Progress must improve by this much to reset the stuck timer.
    constexpr float PROGRESS_EPSILON = 5.0f;
    // Give up after this long without progress (dock waits can near 3
    // minutes, so the timer is generous and paused while riding/flying).
    constexpr uint32 STUCK_TIMEOUT_MS = 5 * MINUTE * IN_MILLISECONDS;
    // Give up after this many consecutive empty resolves (no route).
    constexpr uint32 MAX_FAILED_RESOLVES = 10;
}

bool TravelCommandAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    TravelOrder& order = AI_VALUE(TravelOrder&, "travel order");
    std::string const param = event.getParam();

    if (param == "stop" || param == "cancel")
    {
        if (!order.active)
        {
            botAI->TellMasterNoFacing("I have no travel order to cancel.");
            return true;
        }

        if (order.enabledDebugMove)
            botAI->ChangeStrategy("-debug move", BOT_STATE_NON_COMBAT);
        order.Clear();
        botAI->ChangeStrategy("-travel order", BOT_STATE_NON_COMBAT);
        bot->StopMoving();
        botAI->TellMasterNoFacing("Travel order canceled.");
        return true;
    }

    if (param == "?" || param == "status")
    {
        if (!order.active)
        {
            botAI->TellMasterNoFacing(
                "No travel order. Whisper 'travel <map> <x> <y> <z>', 'travel <x> <y> <z>' or 'travel stop'.");
            return true;
        }

        std::ostringstream out;
        out << "Traveling to map " << order.dest.GetMapId() << " (" << std::fixed << std::setprecision(1)
            << order.dest.GetPositionX() << "," << order.dest.GetPositionY() << "," << order.dest.GetPositionZ()
            << ")";
        if (bot->GetMapId() != order.dest.GetMapId())
            out << " dist=cross-map";
        else
            out << " dist=" << bot->GetExactDist(order.dest) << "y best=" << order.bestDist << "y";
        out << " elapsed=" << (GetMSTimeDiffToNow(order.startedMs) / IN_MILLISECONDS) << "s";
        if (bot->GetTransport())
            out << " [on transport]";
        if (bot->IsInFlight())
            out << " [in flight]";
        botAI->TellMasterNoFacing(out.str());
        return true;
    }

    // Parse "<map> <x> <y> <z>" or "<x> <y> <z>" (current map).
    std::istringstream in(param);
    std::vector<float> vals;
    float v;
    while (in >> v)
        vals.push_back(v);

    uint32 mapId;
    float x, y, z;
    if (vals.size() == 4)
    {
        mapId = (uint32)vals[0];
        x = vals[1];
        y = vals[2];
        z = vals[3];
    }
    else if (vals.size() == 3)
    {
        mapId = bot->GetMapId();
        x = vals[0];
        y = vals[1];
        z = vals[2];
    }
    else
    {
        botAI->TellMasterNoFacing(
            "Usage: 'travel <map> <x> <y> <z>', 'travel <x> <y> <z>', 'travel status' or 'travel stop'.");
        return false;
    }

    WorldPosition dest(mapId, x, y, z);
    WorldPosition botPos(bot);

    // Cross-map orders need the travel-node graph (transitions live there)
    // and both endpoints on overworld continents — same rules the funnel
    // enforces in ResolveMovePath, surfaced here as a chat reply instead
    // of a silent failure.
    if (dest.GetMapId() != botPos.GetMapId())
    {
        if (!botPos.isOverworld() || !dest.isOverworld())
        {
            botAI->TellMasterNoFacing("I can't route cross-map into or out of an instance.");
            return false;
        }

        if (sTravelNodeMap.getNodes().empty())
        {
            botAI->TellMasterNoFacing(
                "I can't route cross-map: travel nodes are disabled or not loaded (AiPlayerbot.EnableTravelNodes).");
            return false;
        }
    }

    uint32 const now = getMSTime();
    order.Clear();
    order.dest = dest;
    order.active = true;
    order.bestDist = botPos.distance(dest);
    order.lastProgressMs = now;
    order.startedMs = now;
    order.lastMapId = bot->GetMapId();
    order.lastTransportEntry = bot->GetTransport() ? bot->GetTransport()->GetEntry() : 0;
    order.wasInFlight = bot->IsInFlight();

    if (sPlayerbotAIConfig.travelCommandDebugMove && !botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("+debug move", BOT_STATE_NON_COMBAT);
        order.enabledDebugMove = true;
    }
    botAI->ChangeStrategy("+travel order", BOT_STATE_NON_COMBAT);

    bool const crossMap = dest.GetMapId() != botPos.GetMapId();
    bool const longMove = crossMap || order.bestDist > sPlayerbotAIConfig.sightDistance;
    std::ostringstream out;
    out << "Traveling to map " << dest.GetMapId() << " (" << std::fixed << std::setprecision(1) << x << "," << y
        << "," << z << "), ";
    // Cross-map distance is a sentinel (200000), not a real yardage.
    if (crossMap)
        out << "cross-map, via ";
    else
        out << order.bestDist << "y away, via ";
    out << (longMove ? (sTravelNodeMap.getNodes().empty() ? "direct pathing (no nodes)" : "travel nodes")
                     : "direct pathing");
    botAI->TellMasterNoFacing(out.str());
    return true;
}

bool DriveTravelOrderAction::isUseful() { return AI_VALUE(TravelOrder&, "travel order").active; }

bool DriveTravelOrderAction::Execute(Event /*event*/)
{
    TravelOrder& order = AI_VALUE(TravelOrder&, "travel order");
    if (!order.active)
        return false;

    uint32 const now = getMSTime();

    // --- Milestone whispers -------------------------------------------------
    if (bot->GetMapId() != order.lastMapId)
    {
        std::ostringstream out;
        out << "Crossed to map " << bot->GetMapId() << ", " << std::fixed << std::setprecision(1)
            << bot->GetExactDist(order.dest) << "y to destination.";
        botAI->TellMasterNoFacing(out.str());
        order.lastMapId = bot->GetMapId();
        // A map change is progress even if raw distance didn't shrink yet.
        order.lastProgressMs = now;
    }

    uint32 const transportEntry = bot->GetTransport() ? bot->GetTransport()->GetEntry() : 0;
    if (transportEntry != order.lastTransportEntry)
    {
        std::ostringstream out;
        if (transportEntry)
            out << "Boarded transport " << bot->GetTransport()->GetGOInfo()->name << " (" << transportEntry << ").";
        else
            out << "Disembarked.";
        botAI->TellMasterNoFacing(out.str());
        order.lastTransportEntry = transportEntry;
        order.lastProgressMs = now;
    }

    bool const inFlight = bot->IsInFlight();
    if (inFlight != order.wasInFlight)
    {
        botAI->TellMasterNoFacing(inFlight ? "Taking a flight." : "Landed.");
        order.wasInFlight = inFlight;
        order.lastProgressMs = now;
    }

    // --- Arrival ------------------------------------------------------------
    float const dist = bot->GetExactDist(order.dest);
    if (bot->GetMapId() == order.dest.GetMapId() && dist < ARRIVE_DISTANCE)
    {
        std::ostringstream out;
        out << "Arrived (" << std::fixed << std::setprecision(1) << dist << "y from target) after "
            << (GetMSTimeDiffToNow(order.startedMs) / IN_MILLISECONDS) << "s.";
        botAI->TellMasterNoFacing(out.str());
        Finish();
        return true;
    }

    // --- Progress / give-up -------------------------------------------------
    bool const waiting = transportEntry != 0 || inFlight;
    if (dist + PROGRESS_EPSILON < order.bestDist)
    {
        order.bestDist = dist;
        order.lastProgressMs = now;
        order.failedResolves = 0;
    }
    else if (!waiting && GetMSTimeDiffToNow(order.lastProgressMs) > STUCK_TIMEOUT_MS)
    {
        std::ostringstream out;
        out << "Giving up: no progress for " << (STUCK_TIMEOUT_MS / IN_MILLISECONDS) << "s (best " << std::fixed
            << std::setprecision(1) << order.bestDist << "y, now " << dist << "y).";
        botAI->TellMasterNoFacing(out.str());
        Finish();
        return false;
    }

    // --- Drive one funnel tick ---------------------------------------------
    bool const moved = MoveTo2(order.dest);

    if (moved && !order.routeAnnounced)
    {
        LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
        if (!lastMove.lastPath.empty())
        {
            uint32 transports = 0;
            uint32 flights = 0;
            uint32 portals = 0;
            for (auto const& p : lastMove.lastPath.GetPathRef())
            {
                if (p.type == PathNodeType::NODE_TRANSPORT)
                    ++transports;
                else if (p.type == PathNodeType::NODE_FLIGHTPATH)
                    ++flights;
                else if (p.type == PathNodeType::NODE_STATIC_PORTAL || p.type == PathNodeType::NODE_AREA_TRIGGER)
                    ++portals;
            }

            std::ostringstream out;
            out << "Route resolved: " << lastMove.lastPath.size() << " points";
            if (transports)
                out << ", " << transports << " transport pts";
            if (flights)
                out << ", " << flights << " flight pts";
            if (portals)
                out << ", " << portals << " portal pts";
            botAI->TellMasterNoFacing(out.str());
            order.routeAnnounced = true;
        }
    }

    if (!moved && !waiting)
    {
        if (++order.failedResolves >= MAX_FAILED_RESOLVES)
        {
            botAI->TellMasterNoFacing("Giving up: no route to the destination.");
            Finish();
            return false;
        }
    }

    return moved;
}

void DriveTravelOrderAction::Finish(bool removeStrategy)
{
    TravelOrder& order = AI_VALUE(TravelOrder&, "travel order");
    if (order.enabledDebugMove)
        botAI->ChangeStrategy("-debug move", BOT_STATE_NON_COMBAT);
    order.Clear();
    if (removeStrategy)
        botAI->ChangeStrategy("-travel order", BOT_STATE_NON_COMBAT);
}

bool TravelOrderActiveTrigger::IsActive() { return AI_VALUE(TravelOrder&, "travel order").active; }

void TravelOrderStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Above RPG/idle bands so the order overrides background behavior;
    // combat still preempts via the engine's COMBAT state.
    triggers.push_back(new TriggerNode("travel order active", { NextAction("drive travel order", 60.0f) }));
}
