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

    // Transport Handling (boats, zeppelins, elevators, platforms)
    // Let normal follow movement bring the bot close to the master
    // Attach the bot as a passenger once it is on (or very close to) the same transport
    // Support classic transports (boats, zeppelins, tram) and moving platforms/elevators
    Player* master = botAI->GetMaster();
    if (master && master->IsInWorld() && bot->IsInWorld())
    {
        // check master/map
        if (bot->GetMapId() == master->GetMapId())
        {
            Map* map = master->GetMap();
            Transport* transport = nullptr;

            // Prefer the direct pointer from the master if available
            if (master->GetTransport())
            {
                transport = master->GetTransport();
            }
            // Some elevators & moving platforms set ONTRANSPORT without exposing a direct Transport* on the master.
            // In that case use the same helper Map API used in PlayerbotAI::UpdateAI to find the active transport.
            else if (map && master->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
            {
                transport = map->GetTransportForPos(master->GetPhaseMask(),
                                                    master->GetPositionX(),
                                                    master->GetPositionY(),
                                                    master->GetPositionZ(),
                                                    master);
            }

            // If the master is on a transport we could identify and the bot is not yet attached:
            if (transport && bot->GetTransport() != transport)
            {
                // Attach the bot when it is already close enough to the master.
                // Follow movement is responsible for bringing the bot into range.
                float const attachDistance = 5.0f;
                if (bot->IsWithinDistInMap(master, attachDistance))
                {
                    transport->AddPassenger(bot, true);
                    bot->StopMovingOnCurrentPos();

                    LOG_DEBUG("playerbots", "Bot {} attached to master's transport {} at follow time",
                        bot->GetName(), transport->GetGUID().GetCounter());

                    // Return true to indicate that follow did something this tick.
                    // Bot is now locked to the transport and will move with it via core logic.
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
