/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FollowActions.h"

#include <algorithm>
#include <cmath>
#include <array>

#include "Event.h"
#include "Formations.h"
#include "LastMovementValue.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Transport.h"
#include "Map.h"

// Transport helpers (GetTransportForPosTolerant, FindBoardingPointOnTransport,
// BoardTransport) are now on MovementAction — inherited by FollowAction.

bool FollowAction::Execute(Event /*event*/)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();

    // Transport handling for moving transports only (boats/zeppelins).
    Player* master = botAI->GetMaster();
    if (master && master->IsInWorld() && bot->IsInWorld() && bot->GetMapId() == master->GetMapId())
    {
        Map* map = master->GetMap();
        uint32 const mapId = bot->GetMapId();
        Transport* transport = nullptr;
        bool masterOnTransport = false;

        if (master->GetTransport())
        {
            transport = master->GetTransport();
            masterOnTransport = true;
        }
        else if (map)
        {
            transport = GetTransportForPosTolerant(map, master, master->GetPhaseMask(),
                master->GetPositionX(), master->GetPositionY(), master->GetPositionZ());
            masterOnTransport = (transport != nullptr);
        }

        // Ignore static transports (elevators/trams): only keep boats/zeppelins here.
        if (transport && transport->IsStaticTransport())
            transport = nullptr;

        if (transport && map && bot->GetTransport() != transport)
        {
            float const botProbeZ = std::max(bot->GetPositionZ(), transport->GetPositionZ());
            Transport* botSurfaceTransport = GetTransportForPosTolerant(map, bot, bot->GetPhaseMask(),
                bot->GetPositionX(), bot->GetPositionY(), botProbeZ);

            if (botSurfaceTransport == transport)
            {
                transport->AddPassenger(bot, true);
                bot->StopMovingOnCurrentPos();
                return true;
            }

            float const boardingAssistDistance = 60.0f;
            float const dist2d = ServerFacade::instance().GetDistance2d(bot, master);
            bool const inAssist = ServerFacade::instance().IsDistanceLessOrEqualThan(dist2d, boardingAssistDistance);

            if (inAssist)
            {
                float destX = masterOnTransport ? master->GetPositionX() : transport->GetPositionX();
                float destY = masterOnTransport ? master->GetPositionY() : transport->GetPositionY();
                float destZ = masterOnTransport ? master->GetPositionZ() : transport->GetPositionZ();
                float edgeX = 0.0f;
                float edgeY = 0.0f;
                float edgeZ = 0.0f;

                if (masterOnTransport &&
                    FindBoardingPointOnTransport(map, transport, master,
                        master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        edgeX, edgeY, edgeZ))
                {
                    destX = edgeX;
                    destY = edgeY;
                    destZ = edgeZ;
                }

                MovementPriority const priority = botAI->GetState() == BOT_STATE_COMBAT
                    ? MovementPriority::MOVEMENT_COMBAT
                    : MovementPriority::MOVEMENT_NORMAL;

                bool const movingAllowed = IsMovingAllowed();
                bool const dupMove = IsDuplicateMove(destX, destY, destZ);
                bool const waiting = IsWaitingForLastMove(priority);

                if (movingAllowed && !dupMove && !waiting)
                {
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
                            /*generatePath*/ false,
                            /*forceDestination*/ false);
                    }
                    else
                        return false;

                    float delay = 1000.0f * MoveDelay(bot->GetExactDist(destX, destY, destZ));
                    delay = std::clamp(delay, 0.0f, static_cast<float>(sPlayerbotAIConfig.maxWaitForMove));

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

bool FleeToGroupLeaderAction::Execute(Event /*event*/)
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
