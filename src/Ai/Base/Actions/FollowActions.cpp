/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FollowActions.h"

#include <cstddef>

#include "Event.h"
#include "Formations.h"
#include "LastMovementValue.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Transport.h"
#include "Map.h"

bool FollowAction::Execute(Event event)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();
    /*
    // Transport (Zep + Boats) Fix
    // TODO: Smooth out bot boarding on transports (boats, zeppelins, tram).
    // TODO: Handle bot's pets
    Player* master = botAI->GetMaster();
    if (master && master->GetTransport())
    {
        Transport* transport = master->GetTransport();

        // Vérification de map : le bot doit être sur la même map que le maître
        if (bot->GetMap() != master->GetMap())
            return false;
        // If the bot is not already a passenger on this transport
        if (bot->GetTransport() != transport)
        {
            // Offset to avoid landing under the floor
            float offsetX = 1.0f, offsetY = 0.5f, offsetZ = 0.2f;
            float x = master->GetPositionX() + offsetX;
            float y = master->GetPositionY() + offsetY;
            float z = master->GetPositionZ() + offsetZ;

            // Teleports to a position close to the master (world coordinates)
            bot->TeleportTo(transport->GetMapId(), x, y, z, master->GetOrientation());

            // Add as a passenger (server-side)
            transport->AddPassenger(bot, true);

            // Force complete cleanup of the movement and flags to avoid
            // wrestling between MoveSpline/MotionMaster and the transport driver.
            bot->StopMoving();
            // MotionMaster current clear (true to force) then idle to stabilize
            bot->GetMotionMaster()->Clear(true);
            bot->GetMotionMaster()->MoveIdle();

            // Ensure that the march/mounted flags are no longer active
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);

            // Allow a slightly longer verification period; during this time
            // the server will update the transport status and relative positions.
            botAI->SetNextCheckDelay(urand(1000, 2500));

            LOG_DEBUG("playerbots", "Bot {} boarded transport {} near master {} at {:.2f},{:.2f},{:.2f}",
                     bot->GetName(), transport->GetGUID().GetCounter(), master->GetName(), x, y, z);

            return true;
        }
    } // End Transport (Zep + Boats) Fix
    */
    // Unified Transport Handling (boats, zeppelins, elevators, platforms)
    Player* master = botAI->GetMaster();
    if (master && master->IsInWorld())
    {
        Map* map = master->GetMap();
        if (map && bot->GetMap() == map)
        {
            // Detect any kind of moving transport the master is currently standing on
            Transport* transport = master->GetTransport();

            // Some moving platforms/elevators do not expose a direct Transport* on the master.
            // In that case we fall back to a positional lookup on the map while the master
            // has the ONTRANSPORT movement flag.
            if (!transport && master->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
            {
                transport = map->GetTransportForPos(master->GetPhaseMask(),
                                                    master->GetPositionX(),
                                                    master->GetPositionY(),
                                                    master->GetPositionZ(),
                                                    master);
            }

            // If we found a transport and the bot is not yet attached, board it.
            if (transport && bot->GetTransport() != transport)
            {
                float const offsetX = 1.0f;
                float const offsetY = 0.5f;
                float const offsetZ = 0.2f;

                float const x = master->GetPositionX() + offsetX;
                float const y = master->GetPositionY() + offsetY;
                float const z = master->GetPositionZ() + offsetZ;

                // Teleport close to the master (world coordinates) and attach as passenger (server-side).
                bot->TeleportTo(transport->GetMapId(), x, y, z, master->GetOrientation());
                transport->AddPassenger(bot, true);

                // Reset movement state so the transport driver fully takes over.
                bot->StopMoving();
                bot->GetMotionMaster()->Clear(true);
                bot->GetMotionMaster()->MoveIdle();
                bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
                bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);

                botAI->SetNextCheckDelay(urand(1000, 2500));
                LOG_INFO("playerbots",
                          "Bot {} boarded transport {} near master {} at {:.2f},{:.2f},{:.2f}",
                          bot->GetName(), transport->GetGUID().GetCounter(), master->GetName(), x, y, z);
                return true;
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
