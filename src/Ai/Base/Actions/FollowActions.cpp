/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FollowActions.h"

#include <cstddef>
#include <algorithm>
#include <unordered_map>

#include "Event.h"
#include "Formations.h"
#include "LastMovementValue.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Transport.h"
#include "Map.h"
#include <cmath>
#include <array>

namespace
{
    // Static transports (elevators/platforms) need extra care: flags/pointers can desync and MMaps miss their geometry.
    // Keep this logic conservative to avoid impacting boats/zeppelins.
    constexpr uint32 STATIC_TRANSPORT_REMEMBER_MS = 12 * IN_MILLISECONDS;
    constexpr float STATIC_TRANSPORT_REMEMBER_MIN_DZ = 8.0f;
    constexpr float STATIC_TRANSPORT_REMEMBER_MAX_MASTER_DIST_2D = 160.0f;
    constexpr float STATIC_TRANSPORT_REMEMBER_MAX_BOT_DIST_2D = 120.0f;

    constexpr float STATIC_TRANSPORT_SNAP_Z_THRESHOLD = 2.5f;
    constexpr float STATIC_TRANSPORT_WAIT_Z_THRESHOLD = 4.0f;
    constexpr float TIGHT_STATIC_TRANSPORT_MAX_SIZE = 1.25f;

    float Distance2D(float ax, float ay, float bx, float by)
    {
        float const dx = ax - bx;
        float const dy = ay - by;
        return std::sqrt(dx * dx + dy * dy);
    }

    Transport* GetTransportForPosTolerant(Map* map, WorldObject* ref, uint32 phaseMask, float x, float y, float z)
    {
        if (!map || !ref)
            return nullptr;

        std::array<float, 4> const probes = { z, z + 0.5f, z + 1.5f, z - 0.5f };
        for (float const pz : probes)
        {
            if (Transport* t = map->GetTransportForPos(phaseMask, x, y, pz, ref))
                return t;
        }

        return nullptr;
    }

    // Attempts to find a point on the leader's transport that is closer to the bot,
    // by probing along the segment from master -> bot and returning the last point
    // that is still detected as being on the expected transport.
    bool FindBoardingPointOnTransport(Map* map, Transport* expectedTransport, WorldObject* ref,
        float masterX, float masterY, float masterZ,
        float botX, float botY, float botZ,
        float& outX, float& outY, float& outZ)
    {
        if (!map || !expectedTransport || !ref)
            return false;

        uint32 const phaseMask = ref->GetPhaseMask();

        // Ensure master is actually detected on that transport (tolerant).
        if (GetTransportForPosTolerant(map, ref, phaseMask, masterX, masterY, masterZ) != expectedTransport)
            return false;

        // The raycast in GetTransportForPos starts at (z + 2). Probe with a safe Z.
        float const probeZ = std::max(masterZ, botZ);

        // Adaptive step count: small platforms need tighter sampling.
        float const dx2 = botX - masterX;
        float const dy2 = botY - masterY;
        float const dist2d = std::sqrt(dx2 * dx2 + dy2 * dy2);
        int32 const steps = std::clamp(static_cast<int32>(dist2d / 0.75f), 10, 28);

        float const dx = (botX - masterX) / static_cast<float>(steps);
        float const dy = (botY - masterY) / static_cast<float>(steps);

        // Master must actually be on the expected transport for this to work.
        if (map->GetTransportForPos(ref->GetPhaseMask(), masterX, masterY, probeZ, ref) != expectedTransport)
            return false;

        float lastX = masterX;
        float lastY = masterY;
        bool found = false;

        for (int32 i = 1; i <= steps; ++i)
        {
            float const px = masterX + dx * i;
            float const py = masterY + dy * i;

            Transport* const t = GetTransportForPosTolerant(map, ref, phaseMask, px, py, probeZ);
            if (t != expectedTransport)
                break;

            lastX = px;
            lastY = py;
            found = true;
        }

        if (!found)
            return false;

        outX = lastX;
        outY = lastY;
        outZ = masterZ; // keep deck-level Z to encourage stepping onto the platform/boat
        return true;
    }

    // Select a staging point near the elevator shaft while waiting for a static transport (elevator) to reach the bot's Z.
    // The goal is to keep the bot close enough to board quickly, without aiming directly for the platform center (which can be blocked by shaft walls).
    void FindStagingPointOutsideTransport(Transport const* transport, float botX, float botY, float botZ, float& outX, float& outY, float& outZ)
    {
        if (!transport)
        {
            outX = botX;
            outY = botY;
            outZ = botZ;
            return;
        }

        float const centerX = transport->GetPositionX();
        float const centerY = transport->GetPositionY();

        float vx = botX - centerX;
        float vy = botY - centerY;

        float const distSq = vx * vx + vy * vy;

        if (distSq > 0.0001f)
        {
            float const invDist = 1.0f / std::sqrt(distSq);
            vx *= invDist;
            vy *= invDist;
        }
        else
        {
            vx = 1.0f;
            vy = 0.0f;
        }

        // Elevator models can be very small. Use a conservative minimum radius.
        float const radius = std::max(transport->GetObjectSize() + 0.8f, 1.8f);

        outX = centerX + vx * radius;
        outY = centerY + vy * radius;
        outZ = botZ;
    }

    // Sufficiently small static transports (elevators, tiny platforms) need special handling:
    // object size often reflects only model radius and can be extremely small for elevators.
    bool IsTightStaticTransport(Transport const* transport)
    {
        if (!transport || !transport->IsStaticTransport())
            return false;

        float const size = transport->GetObjectSize();
        return size > 0.0f && size <= TIGHT_STATIC_TRANSPORT_MAX_SIZE;
    }

    float GetTightTransportClusterDist(Transport const* transport)
    {
        float const size = std::max(transport ? transport->GetObjectSize() : 0.0f, 0.01f);
        return std::clamp(size * 3.0f, 0.8f, 1.8f);
    }

    struct RecentStaticTransportInfo
    {
        uint32 transportLow = 0;
        uint32 mapId = 0;
        uint32 lastSeenMs = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    // Cache the last static transport the leader was actually riding (e.g. elevator platforms).
    // This helps bots keep boarding even if the leader steps off the platform immediately.
    std::unordered_map<uint32, RecentStaticTransportInfo> s_recentStaticTransportByMaster;

    void RememberMasterStaticTransport(Player* master, Transport* transport, uint32 nowMs)
    {
        if (!master || !transport || !transport->IsStaticTransport())
            return;

        uint32 const masterLow = master->GetGUID().GetCounter();

        RecentStaticTransportInfo& info = s_recentStaticTransportByMaster[masterLow];
        info.transportLow = transport->GetGUID().GetCounter();
        info.mapId = master->GetMapId();
        info.lastSeenMs = nowMs;
        info.x = transport->GetPositionX();
        info.y = transport->GetPositionY();
        info.z = transport->GetPositionZ();
    }

    Transport* ResolveRememberedStaticTransport(Player* master, Player* bot, Map* map, uint32 nowMs)
    {
        if (!master || !bot || !map)
            return nullptr;

        uint32 const masterLow = master->GetGUID().GetCounter();

        auto const it = s_recentStaticTransportByMaster.find(masterLow);
        if (it == s_recentStaticTransportByMaster.end())
            return nullptr;

        RecentStaticTransportInfo const& info = it->second;

        // Expire quickly: elevators/platforms are local interactions, not long-term routing hints.
        uint32 const elapsed = getMSTimeDiff(info.lastSeenMs, nowMs);
        if (elapsed > STATIC_TRANSPORT_REMEMBER_MS)
            return nullptr;

        // Must be same map to avoid stale pointers / false positives.
        if (info.mapId != master->GetMapId() || info.mapId != bot->GetMapId())
            return nullptr;

        // Only useful when there is a meaningful vertical separation.
        float const dz = std::fabs(master->GetPositionZ() - bot->GetPositionZ());
        if (dz < STATIC_TRANSPORT_REMEMBER_MIN_DZ)
            return nullptr;

        // Avoid locking bots onto the cache when the group has clearly moved on.
        if (Distance2D(master->GetPositionX(), master->GetPositionY(), info.x, info.y) > STATIC_TRANSPORT_REMEMBER_MAX_MASTER_DIST_2D)
            return nullptr;

        if (Distance2D(bot->GetPositionX(), bot->GetPositionY(), info.x, info.y) > STATIC_TRANSPORT_REMEMBER_MAX_BOT_DIST_2D)
            return nullptr;

        uint32 const phaseMask = master->GetPhaseMask();
        float const zMin = std::min(bot->GetPositionZ(), master->GetPositionZ()) - 2.0f;
        float const zMax = std::max(bot->GetPositionZ(), master->GetPositionZ()) + 2.0f;

        // Scan along Z at the elevator X/Y to find the platform wherever it currently is.
        // Keep the scan small: this runs only when transport detection already failed.
        int32 const steps = 6;
        for (int32 i = 0; i <= steps; ++i)
        {
            float const probeZ = zMin + (zMax - zMin) * (static_cast<float>(i) / static_cast<float>(steps));

            if (Transport* const t = GetTransportForPosTolerant(map, master, phaseMask, info.x, info.y, probeZ))
            {
                if (t->IsStaticTransport() && t->GetGUID().GetCounter() == info.transportLow)
                    return t;
            }
        }

        // Also try the last known platform Z as a final cheap probe (covers small oscillations).
        if (Transport* const t = GetTransportForPosTolerant(map, master, phaseMask, info.x, info.y, info.z))
        {
            if (t->IsStaticTransport() && t->GetGUID().GetCounter() == info.transportLow)
                return t;
        }

        return nullptr;
    }
}

bool FollowAction::Execute(Event event)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();
    uint32 const nowMs = getMSTime();

    // Transport Handling (boats, zeppelins, elevators, platforms)
    // TODO andle Stormwind TRAM
    Player* master = botAI->GetMaster();
    if (master && master->IsInWorld() && bot->IsInWorld() && bot->GetMapId() == master->GetMapId())
    {
        Map* map = master->GetMap();
        uint32 const mapId = bot->GetMapId();
        Transport* transport = nullptr;
        bool masterOnTransport = false;

        // Prefer the direct pointer from the master if available.
        // On elevators, MOVEMENTFLAG_ONTRANSPORT can be briefly desynced; the pointer is still authoritative.
        if (master->GetTransport())
        {
            transport = master->GetTransport();
            masterOnTransport = true;
        }
        // Fallback: use the same geometry-based detection the core uses (raycast on transport models).
        // This is important for some transports/platforms where GetTransport() may lag behind movement updates.
        else if (map)
        {
            transport = GetTransportForPosTolerant(map, master, master->GetPhaseMask(),
                master->GetPositionX(), master->GetPositionY(), master->GetPositionZ());
            masterOnTransport = (transport != nullptr);
        }

        // Static transports: the leader can step off instantly (top/bottom platforms), leaving GetTransport() null.
        // Keep a short "grace period" so bots still board the elevator instead of pathing around the shaft.
        if (!transport && map)
            transport = ResolveRememberedStaticTransport(master, bot, map, nowMs);

        if (transport && masterOnTransport && transport->IsStaticTransport())
            RememberMasterStaticTransport(master, transport, nowMs);

        auto const PrepareBotForMove = [&]()
        {
            if (bot->IsSitState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            if (bot->IsNonMeleeSpellCast(true))
            {
                bot->CastStop();
                botAI->InterruptSpell();
            }
        };

        auto const reseatOnStaticTransportDeck = [&](float deckZ)
        {
            if (!transport || !transport->IsStaticTransport())
                return;

            // Re-seat: detach then attach again at the correct Z so the transport offset is rebuilt.
            if (bot->GetTransport() == transport)
                transport->RemovePassenger(bot);

            bot->NearTeleportTo(bot->GetPositionX(), bot->GetPositionY(), deckZ, bot->GetOrientation());
            transport->AddPassenger(bot, true);
        };

        // If both master and bot are already on the same transport, do not try to maintain a tight formation
        // on static transports (elevators). Small splines can easily push the bot off the platform and detach.
        auto const handleStaticTransportPassenger = [&]() -> bool
        {
            if (!transport || !map)
                return false;

            if (bot->GetTransport() != transport)
                return false;

            if (!transport->IsStaticTransport() || !masterOnTransport)
                return false;

            float const dist2d = sServerFacade->GetDistance2d(bot, master);

            // Tiny elevators: pack bots near the master and bypass formation offsets while riding.
            if (IsTightStaticTransport(transport))
            {
                float const clusterDist = GetTightTransportClusterDist(transport);

                // Static transports (elevators): if a bot got attached below the platform, keep its XY but snap it back onto the deck Z,
                // otherwise it will stay "glued" underneath while the transport moves.
                float const dzDeck = std::fabs(bot->GetPositionZ() - master->GetPositionZ());
                if (dzDeck > STATIC_TRANSPORT_SNAP_Z_THRESHOLD)
                {
           
                    reseatOnStaticTransportDeck(master->GetPositionZ());
                }

                if (sServerFacade->IsDistanceLessOrEqualThan(dist2d, clusterDist))
                {
                    bot->StopMovingOnCurrentPos();
                    return true;
                }

                float const destX = master->GetPositionX();
                float const destY = master->GetPositionY();
                float const destZ = transport->GetPositionZ();

                MovementPriority const priority = botAI->GetState() == BOT_STATE_COMBAT
                    ? MovementPriority::MOVEMENT_COMBAT
                    : MovementPriority::MOVEMENT_NORMAL;

                bool const movingAllowed = IsMovingAllowed(mapId, destX, destY, destZ);
                bool const dupMove = IsDuplicateMove(mapId, destX, destY, destZ);
                bool const waiting = IsWaitingForLastMove(priority);

                // Still bypass formation logic on elevators, even if duplicate/waiting.
                if (!movingAllowed || dupMove || waiting)
                    return true;

                PrepareBotForMove();

                MotionMaster* mm = bot->GetMotionMaster();
                if (!mm)
                    return false;

                mm->MovePoint(
                    /*id*/ 0,
                    /*coords*/ destX, destY, destZ,
                    /*forcedMovement*/ FORCED_MOVEMENT_NONE,
                    /*speed*/ 0.0f,
                    /*orientation*/ 0.0f,
                    /*generatePath*/ false, // already riding: keep direct movement
                    /*forceDestination*/ false);

                float delay = 1000.0f * MoveDelay(bot->GetExactDist(destX, destY, destZ));
                delay = std::clamp(delay, 0.0f, static_cast<float>(sPlayerbotAIConfig->maxWaitForMove));
                delay = std::min(delay, 750.0f);

                AI_VALUE(LastMovement&, "last movement")
                    .Set(mapId, destX, destY, destZ, bot->GetOrientation(), delay, priority);
                ClearIdleState();

                return true;
            }

            float const safeDist = std::max(formation ? formation->GetMaxDistance() : 0.0f, 6.0f);
            if (!sServerFacade->IsDistanceLessOrEqualThan(dist2d, safeDist))
                return false;

            bot->StopMovingOnCurrentPos();

            return true;
        };

        if (handleStaticTransportPassenger())
            return true;

        if (transport && map && bot->GetTransport() != transport)
        {
            // If the bot is already on the transport geometry, attach immediately (no need to be near the master).
            float const botProbeZ = std::max(bot->GetPositionZ(), transport->GetPositionZ());
            Transport* botSurfaceTransport = GetTransportForPosTolerant(map, bot, bot->GetPhaseMask(),
                bot->GetPositionX(), bot->GetPositionY(), botProbeZ);

            if (botSurfaceTransport == transport)
            {
                transport->AddPassenger(bot, true);
                bot->StopMovingOnCurrentPos();

                return true;
            }

            bool const isStaticTransport = transport->IsStaticTransport();

            // Boarding assistance: within a reasonable distance, use a direct MovePoint (no MMaps)
            // so bots don't try to navigate around the moving platform.
            float const boardingAssistDistance = 60.0f;
            float const dist2d = sServerFacade->GetDistance2d(bot, master);
            bool inAssist = sServerFacade->IsDistanceLessOrEqualThan(dist2d, boardingAssistDistance);

            // If the leader already stepped off a static transport (elevator), dist2d(master) can quickly exceed the
            // assist radius, causing bots to path around the shaft instead of boarding. When we recovered a static
            // transport from cache, allow boarding assistance based on distance to the platform itself.
            if (!inAssist && isStaticTransport && !masterOnTransport)
            {
                float const distToTransport = Distance2D(
                    bot->GetPositionX(), bot->GetPositionY(),
                    transport->GetPositionX(), transport->GetPositionY());
                inAssist = sServerFacade->IsDistanceLessOrEqualThan(distToTransport, boardingAssistDistance + 30.0f);
            }

            if (inAssist)
            {
                // Static transports (elevators/platforms) move fast on Z. If the platform is not at the bot's level,
                // chasing the leader's Z produces unreachable MovePoint targets and long "waiting" delays.
                auto const handleStaticTransportBoardingWindow = [&]() -> bool
                {
                    if (!isStaticTransport)
                        return false;

                    float const dzToPlatform = std::fabs(transport->GetPositionZ() - bot->GetPositionZ());
					

                    // If the leader is already on the platform and the bot is close in XY but far in Z,
                    // we prefer snapping/attaching onto the deck instead of pushing the bot outside.
                    if (masterOnTransport)
                    {
                        float const safety = 2.0f + bot->GetObjectSize() + transport->GetObjectSize();
                        if (sServerFacade->IsDistanceLessOrEqualThan(dist2d, safety) && dzToPlatform > STATIC_TRANSPORT_SNAP_Z_THRESHOLD)
                        {

                            reseatOnStaticTransportDeck(master->GetPositionZ());
                            return true;
                        }
                    }
                    // When the platform is far away in Z, do not move into the shaft.
                    if (dzToPlatform <= STATIC_TRANSPORT_WAIT_Z_THRESHOLD)
                        return false;

                    float const botX = bot->GetPositionX();
                    float const botY = bot->GetPositionY();
                    float const botZ = bot->GetPositionZ();

                    // When platform is far away in Z, wait at transport X/Y, but keep bot's current.
                    float waitX = transport->GetPositionX();
                    float waitY = transport->GetPositionY();
                    float waitZ = botZ;

                    // Keep bots just outside the lift footprint while it's not at their Z.
                    FindStagingPointOutsideTransport(transport, botX, botY, botZ,
                        waitX, waitY, waitZ);

                    MovementPriority const priority = botAI->GetState() == BOT_STATE_COMBAT
                        ? MovementPriority::MOVEMENT_COMBAT
                        : MovementPriority::MOVEMENT_NORMAL;

                    bool const movingAllowed = IsMovingAllowed(mapId, waitX, waitY, waitZ);
                    bool const dupMove = IsDuplicateMove(mapId, waitX, waitY, waitZ);

                    if (movingAllowed && !dupMove)
                    {
                        PrepareBotForMove();

                        if (MotionMaster* mm = bot->GetMotionMaster())
                        {
                            mm->MovePoint(
                                /*id*/ 0,
                                /*coords*/ waitX, waitY, waitZ,
                                /*forcedMovement*/ FORCED_MOVEMENT_NONE,
                                /*speed*/ 0.0f,
                                /*orientation*/ 0.0f,
                                /*generatePath*/ true,
                                /*forceDestination*/ false);
                        }

                        float delay = 1000.0f * MoveDelay(bot->GetExactDist(waitX, waitY, waitZ));
                        delay = std::clamp(delay, 0.0f, 250.0f);
                        AI_VALUE(LastMovement&, "last movement")
                            .Set(mapId, waitX, waitY, waitZ, bot->GetOrientation(), delay, priority);
                        ClearIdleState();
                    }

                    return true;
                };

                if (handleStaticTransportBoardingWindow())
                    return true;

                float destX = 0.0f;
                float destY = 0.0f;
                float destZ = 0.0f;
 
                // Always target the platform center to avoid edge collisions,
                // and to keep followers clustered where they will remain on the platform when it starts moving quickly.
                if (isStaticTransport)
                {
                    destX = transport->GetPositionX();
                    destY = transport->GetPositionY();
                    destZ = transport->GetPositionZ();
                }
 
                // If the leader already left the elevator platform, use the transport position as target.
                // Otherwise, use the leader position to keep the group clustered while boarding.
                else if (masterOnTransport)
                {
                    destX = master->GetPositionX();
                    destY = master->GetPositionY();
                    destZ = master->GetPositionZ();
                }
                else
                {
                    destX = transport->GetPositionX();
                    destY = transport->GetPositionY();
                    destZ = transport->GetPositionZ();
                }

                float const masterX = master->GetPositionX();
                float const masterY = master->GetPositionY();
                float const masterZ = master->GetPositionZ();
 
                float const botX = bot->GetPositionX();
                float const botY = bot->GetPositionY();
                float const botZ = bot->GetPositionZ();

                // When the leader is already on the transport, moving to their exact position is often unreachable
                // from MMaps. Try to move to the nearest "edge" point
                // on the same transport along the segment leader -> bot.
                float edgeX = 0.0f;
                float edgeY = 0.0f;
                float edgeZ = 0.0f;
                // Boarding-point logic is useful for moving transports (boats/zeppelins),
                // but hurts elevators.
                if (masterOnTransport && !isStaticTransport &&
                    FindBoardingPointOnTransport(map, transport, master,
                        masterX, masterY, masterZ,
                        botX, botY, botZ,
                        edgeX, edgeY, edgeZ))
                {
                    destX = edgeX;
                    destY = edgeY;
                    destZ = edgeZ;
                }

                MovementPriority const priority = botAI->GetState() == BOT_STATE_COMBAT
                    ? MovementPriority::MOVEMENT_COMBAT
                    : MovementPriority::MOVEMENT_NORMAL;

                bool const movingAllowed = IsMovingAllowed(mapId, destX, destY, destZ);
                bool const dupMove = IsDuplicateMove(mapId, destX, destY, destZ);
                bool waiting = IsWaitingForLastMove(priority);
                if (masterOnTransport && isStaticTransport)
                    waiting = false;

                if (movingAllowed && !dupMove && !waiting)
                {
                    bool const generatePath = isStaticTransport;

                    if (bot->IsSitState())
                        bot->SetStandState(UNIT_STAND_STATE_STAND);

                    if (bot->IsNonMeleeSpellCast(true))
                    {
                        bot->CastStop();
                        botAI->InterruptSpell();
                    }

                    if (MotionMaster* mm = bot->GetMotionMaster())
                    {
                        mm->MovePoint(
                            /*id*/ 0,
                            /*coords*/ destX, destY, destZ,
                            /*forcedMovement*/ FORCED_MOVEMENT_NONE,
                            /*speed*/ 0.0f,
                            /*orientation*/ 0.0f,
                            /*generatePath*/ generatePath,
                            /*forceDestination*/ false);
                    }
                    else
                    {
                        return false;
                    }
 
                    float delay = 1000.0f * MoveDelay(bot->GetExactDist(destX, destY, destZ));
                    delay = std::clamp(delay, 0.0f, static_cast<float>(sPlayerbotAIConfig->maxWaitForMove));
                    if (isStaticTransport)
                    {
                        delay = std::min(delay, 750.0f);
                        if (masterOnTransport)
                            delay = std::min(delay, 200.0f);
                    }
                    AI_VALUE(LastMovement&, "last movement")
                        .Set(mapId, destX, destY, destZ, bot->GetOrientation(), delay, priority);
                    ClearIdleState();
                    return true;
                }
            }
        }
    }
    // end unified transport handling

    bool moved = false;
    if (!target.empty())
    {
        moved = Follow(AI_VALUE(Unit*, target));
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
            return false;

        MovementPriority priority = botAI->GetState() == BOT_STATE_COMBAT ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL;
        moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), false, false, false,
                       true, priority, true);
    }

    // This section has been commented out because it was forcing the pet to
    // follow the bot on every "follow" action tick, overriding any attack or
    // stay commands that might have been issued by the player.
    // if (Pet* pet = bot->GetPet())
    // {
    //     botAI->PetFollow();
    // }
    // if (moved)
    // botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);

    return moved;
}

bool FollowAction::isUseful()
{
    // move from group takes priority over follow as it's added and removed automatically
    // (without removing/adding follow)
    if (botAI->HasStrategy("move from group", BOT_STATE_COMBAT) ||
        botAI->HasStrategy("move from group", BOT_STATE_NON_COMBAT))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Formation* formation = AI_VALUE(Formation*, "formation");
    if (!formation)
        return false;

    std::string const target = formation->GetTargetName();

    Unit* fTarget = nullptr;
    if (!target.empty())
        fTarget = AI_VALUE(Unit*, target);
    else
        fTarget = AI_VALUE(Unit*, "group leader");

    if (fTarget)
    {
        if (fTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return false;

        if (!CanDeadFollow(fTarget))
            return false;

        if (fTarget->GetGUID() == bot->GetGUID())
            return false;
    }

    float distance = 0.f;
    if (!target.empty())
    {
        distance = AI_VALUE2(float, "distance", target);
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || bot->GetMapId() != loc.GetMapId())
            return false;

        distance = bot->GetDistance(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
    }
    if (botAI->HasStrategy("master fishing", BOT_STATE_NON_COMBAT))
        return ServerFacade::instance().IsDistanceGreaterThan(distance, sPlayerbotAIConfig.fishingDistanceFromMaster);

    return ServerFacade::instance().IsDistanceGreaterThan(distance, formation->GetMaxDistance());
}

bool FollowAction::CanDeadFollow(Unit* target)
{
    // In battleground, wait for spirit healer
    if (bot->InBattleground() && !bot->IsAlive())
        return false;

    // Move to corpse when dead and player is alive or not a ghost.
    if (!bot->IsAlive() && (target->IsAlive() || !target->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)))
        return false;

    return true;
}

bool FleeToGroupLeaderAction::Execute(Event event)
{
    Unit* fTarget = AI_VALUE(Unit*, "group leader");
    bool canFollow = Follow(fTarget);
    if (!canFollow)
    {
        // botAI->SetNextCheckDelay(5000);
        return false;
    }

    WorldPosition targetPos(fTarget);
    WorldPosition bosPos(bot);
    float distance = bosPos.fDist(targetPos);

    if (distance < sPlayerbotAIConfig.reactDistance * 3)
    {
        if (!urand(0, 3))
            botAI->TellMaster("I am close, wait for me!");
    }
    else if (distance < 1000)
    {
        if (!urand(0, 10))
            botAI->TellMaster("I heading to your position.");
    }
    else if (!urand(0, 20))
        botAI->TellMaster("I am traveling to your position.");

    botAI->SetNextCheckDelay(3000);

    return true;
}

bool FleeToGroupLeaderAction::isUseful()
{
    if (!botAI->GetGroupLeader())
        return false;

    if (botAI->GetGroupLeader() == bot)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && botAI->GetGroupLeader()->GetTarget() == target->GetGUID())
        return false;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;

    Unit* fTarget = AI_VALUE(Unit*, "group leader");

    if (!CanDeadFollow(fTarget))
        return false;

    return true;
}
