/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MovementActions.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>

#include "Corpse.h"
#include "Event.h"
#include "FleeManager.h"
#include "G3D/Vector3.h"
#include "GameObject.h"
#include "LastMovementValue.h"
#include "LootObjectStack.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInitArgs.h"
#include "MovementGenerator.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PathGenerator.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "PositionValue.h"
#include "Random.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "Transport.h"
#include "TravelNode.h"
#include "Unit.h"
#include "WaypointMovementGenerator.h"

MovementAction::MovementAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name)
{
    bot = botAI->GetBot();
}

void MovementAction::EmitDebugMove(char const* method, char const* generator, float x, float y, float z, char const* extra)
{
    if (!botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
        return;

    auto resolveName = [&](ObjectGuid guid) -> std::string
    {
        if (!guid)
            return "";
        if (WorldObject* obj = botAI->GetWorldObject(guid))
            return obj->GetName();
        return "";
    };

    NewRpgInfo& info = botAI->rpgInfo;
    NewRpgStatus status = info.GetStatus();
    bool const inCombat = botAI->GetState() == BOT_STATE_COMBAT;
    char const* statusName =
        inCombat ? "combat" :
        status == RPG_IDLE ? "idle" :
        status == RPG_GO_GRIND ? "go-grind" :
        status == RPG_GO_CAMP ? "go-camp" :
        status == RPG_WANDER_NPC ? "wander-npc" :
        status == RPG_WANDER_RANDOM ? "wander-random" :
        status == RPG_REST ? "rest" :
        status == RPG_DO_QUEST ? "do-quest" :
        status == RPG_TRAVEL_FLIGHT ? "travel-flight" :
        status == RPG_OUTDOOR_PVP ? "outdoor-pvp" : "?";

    // Resolve a human-readable target name. In combat, the bot is
    // actively engaging an enemy that is unrelated to the RPG state's
    // target — show that enemy instead of the now-stale RPG goal.
    // Out of combat, fall back to the RPG context: quest objective,
    // wander NPC, flight master, etc. Names are far more useful than
    // coordinates; loc=(x,y,z) only when nothing nameable applies.
    std::string targetName;
    if (inCombat)
    {
        Unit* current = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
        Unit* enemyPlayer = *botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
        Unit* enemy = current ? current : enemyPlayer;
        if (enemy)
            targetName = std::string("vs:") + enemy->GetName();
    }
    else switch (status)
    {
        case RPG_DO_QUEST:
            if (auto* data = std::get_if<NewRpgInfo::DoQuest>(&info.data))
            {
                if (data->quest)
                {
                    bool turnIn = data->questId &&
                        bot->GetQuestStatus(data->questId) == QUEST_STATUS_COMPLETE;
                    if (turnIn)
                    {
                        std::ostringstream t;
                        t << "turn-in:" << data->quest->GetTitle() << "(" << data->questId << ")";
                        targetName = t.str();
                    }
                    else
                    {
                        Quest const* q = data->quest;
                        QuestStatusData const& qs = bot->getQuestStatusMap().at(data->questId);
                        std::string goal;
                        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                        {
                            int32 entry = q->RequiredNpcOrGo[i];
                            if (entry != 0 && qs.CreatureOrGOCount[i] < q->RequiredNpcOrGoCount[i])
                            {
                                if (entry > 0)
                                {
                                    if (CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(entry))
                                        goal = "mob:" + ct->Name;
                                }
                                else
                                {
                                    if (GameObjectTemplate const* gt = sObjectMgr->GetGameObjectTemplate(-entry))
                                        goal = "go:" + gt->name;
                                }
                                break;
                            }
                            uint32 item = q->RequiredItemId[i];
                            if (item && bot->GetItemCount(item, true) < q->RequiredItemCount[i])
                            {
                                if (ItemTemplate const* it = sObjectMgr->GetItemTemplate(item))
                                    goal = "item:" + it->Name1;
                                break;
                            }
                        }
                        if (goal.empty())
                        {
                            std::ostringstream t;
                            t << "quest:" << q->GetTitle() << "(" << data->questId << ")";
                            goal = t.str();
                        }
                        targetName = goal;
                    }
                }
            }
            break;
        case RPG_WANDER_NPC:
            if (auto* data = std::get_if<NewRpgInfo::WanderNpc>(&info.data))
            {
                std::string n = resolveName(data->npcOrGo);
                if (!n.empty())
                    targetName = "npc:" + n;
            }
            break;
        case RPG_TRAVEL_FLIGHT:
            if (auto* data = std::get_if<NewRpgInfo::TravelFlight>(&info.data))
            {
                if (CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(data->flightMasterEntry))
                    targetName = "flightmaster:" + ct->Name;
            }
            break;
        case RPG_GO_GRIND: targetName = "grind-pos"; break;
        case RPG_GO_CAMP: targetName = "camp-pos"; break;
        case RPG_WANDER_RANDOM: targetName = "wander-random"; break;
        default: break;
    }

    float dis = bot->GetExactDist(x, y, z);
    std::ostringstream out;
    out << "[M] | " << method
        << " | " << (generator && *generator ? generator : "-")
        << " | " << statusName
        << " | " << std::fixed << std::setprecision(2) << dis << " yard"
        << " | " << (targetName.empty() ? "-" : targetName.c_str());
    if (extra && *extra)
        out << " | " << extra;
    botAI->TellMasterNoFacing(out);
}


void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.reactDelay;

    // if (!important)
    // delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    botAI->GetBot()->AddAura(246, wpCreature);

    if (!important)
        wpCreature->SetObjectScale(0.5f);
}

bool MovementAction::JumpTo(uint32 mapId, float x, float y, float z, MovementPriority priority)
{
    UpdateMovementState();
    if (!IsMovingAllowed())
        return false;

    if (IsDuplicateMove(x, y, z))
        return false;

    float speed = bot->GetSpeed(MOVE_RUN);
    MotionMaster& mm = *bot->GetMotionMaster();
    mm.Clear();
    mm.MoveJump(x, y, z, speed, speed, 1);
    AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), 1000, priority);
    return true;
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    float angle = GetFollowAngle();
    EmitDebugMove("MoveNear", "mmap", x, y, z);
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z, false, false, false, false, priority);
}

bool MovementAction::MoveNear(WorldObject* target, float distance, MovementPriority priority)
{
    if (!target)
        return false;

    float const followAngle = GetFollowAngle();
    float const followRange = botAI->GetRange("follow");

    for (float angle = followAngle; angle <= followAngle + static_cast<float>(2 * M_PI);
         angle += static_cast<float>(M_PI / 4.f))
    {
        // GetNearPoint is engine-aware: snaps to walkable terrain,
        // avoids collision geometry, clamps Z. Drops the raw cos/sin
        // offset that could land the bot inside walls or on ledges.
        // Matches the reference's MoveNear shape.
        float x = target->GetPositionX();
        float y = target->GetPositionY();
        float z = target->GetPositionZ();
        float const dist = distance + target->GetObjectSize();
        target->GetNearPoint(bot, x, y, z, bot->GetObjectSize(),
                             std::min(dist, followRange), angle);

        // LOS test at eye-level (collision-height above feet) ignoring
        // M2 models — large M2 trees/canopies otherwise block the test
        // even when the bot can walk to the spot.
        if (!bot->IsWithinLOS(x, y, z + bot->GetCollisionHeight(),
                              VMAP::ModelIgnoreFlags::M2))
            continue;

        bool moved = MoveTo(target->GetMapId(), x, y, z, false, false, false, false, priority);
        if (moved)
            return true;
    }

    return false;
}

bool MovementAction::MoveToLOS(WorldObject* target, bool ranged)
{
    if (!target)
        return false;

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    EmitDebugMove("MoveToLOS", "mmap", x, y, z);

    // Use standard PathGenerator to find a route.
    PathResult path = GeneratePath(x, y, z, DEFAULT_PATH_ACCEPT_MASK, false);
    if (!path.reachable)
        return false;

    if (!ranged)
        return MoveTo((Unit*)target, target->GetCombatReach());

    float dist = FLT_MAX;
    PositionInfo dest;

    if (!path.points.empty())
    {
        for (auto& point : path.points)
        {
            if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
                CreateWp(bot, point.x, point.y, point.z, 0.0, 2334);

            float distPoint = target->GetDistance(point.x, point.y, point.z);
            if (distPoint < dist && target->IsWithinLOS(point.x, point.y, point.z + bot->GetCollisionHeight()))
            {
                dist = distPoint;
                dest.Set(point.x, point.y, point.z, target->GetMapId());

                if (ranged)
                    break;
            }
        }
    }

    if (dest.isSet())
        return MoveTo(dest.mapId, dest.x, dest.y, dest.z);
    else
        botAI->TellError("All paths not in LOS");

    return false;
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react,
                            [[maybe_unused]] bool normal_only,
                            bool exact_waypoint, MovementPriority priority, bool lessDelay,
                            bool backwards, bool ignoreEnemyTargets)
{
    UpdateMovementState();
    if (!IsMovingAllowed())
        return false;
    if (IsDuplicateMove(x, y, z))
        return false;

    bool const generatePath = !bot->IsFlying() && !bot->isSwimming();
    bool const disableMoveSplinePath =
        sPlayerbotAIConfig.disableMoveSplinePath >= 2 ||
        (sPlayerbotAIConfig.disableMoveSplinePath == 1 && bot->InBattleground());

    // Intentional bypass — skip the path-aware pipeline and dispatch
    // straight to DoMovePoint. Cases:
    //   exact_waypoint: caller wants the raw target, no clipping
    //   disableMoveSplinePath: config-driven engine fallback
    //   flying/swimming: pathfinding via engine MovePoint, not mmap probe
    //   backwards: AC-specific back-shuffle; no parity in MoveTo2
    if (exact_waypoint || disableMoveSplinePath || !generatePath || backwards)
    {
        float distance = bot->GetExactDist(x, y, z);
        if (distance > 0.01f)
        {
            if (!bot->IsStandState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            DoMovePoint(bot, x, y, z, generatePath, backwards);
            float delay = 1000.0f * MoveDelay(distance, backwards);
            if (lessDelay)
                delay -= botAI->GetReactDelay();
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig.maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement")
                .Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
        return false;
    }

    // Path-aware funnel: ResolveMovePath → makeShortCut →
    // UpcommingSpecialMovement/HandleSpecialMovement → ClipPath →
    // DispatchMovement. Matches the reference's MoveTo2 flow.
    return MoveTo2(WorldPosition(mapId, x, y, z),
                   idle, react, false, ignoreEnemyTargets, priority, lessDelay);
}

bool MovementAction::MoveTo(WorldObject* target, float distance, MovementPriority priority)
{
    if (!IsMovingAllowed(target))
        return false;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    float tz = target->GetPositionZ();

    float distanceToTarget = bot->GetDistance(target);
    float angle = bot->GetAngle(target);
    float needToGo = distanceToTarget - distance;

    float maxDistance = sPlayerbotAIConfig.spellDistance;
    if (needToGo > 0 && needToGo > maxDistance)
        needToGo = maxDistance;
    else if (needToGo < 0 && needToGo < -maxDistance)
        needToGo = -maxDistance;

    float dx = cos(angle) * needToGo + bx;
    float dy = sin(angle) * needToGo + by;
    // Start from a seed Z between bot and target, then clamp to the
    // terrain under (dx,dy). Linear interpolation alone ignores hills
    // between the two units and fed PointMovementGenerator a Z that
    // could be well above/below ground, triggering straight-line
    // fallbacks through walls.
    float dz;
    if (distanceToTarget > CONTACT_DISTANCE)
        dz = bz + (tz - bz) * (needToGo / distanceToTarget);
    else
        dz = tz;
    bot->UpdateAllowedPositionZ(dx, dy, dz);
    return MoveTo(target->GetMapId(), dx, dy, dz, false, false, false, false, priority);
}

bool MovementAction::ReachCombatTo(Unit* target, float distance)
{
    if (!IsMovingAllowed(target))
        return false;

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    float targetOrientation = target->GetOrientation();

    float deltaAngle = Position::NormalizeOrientation(targetOrientation - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI;  // -PI..PI
    // if target is moving forward and moving far away, predict the position
    bool behind = fabs(deltaAngle) > M_PI_2;
    if (target->HasUnitMovementFlag(MOVEMENTFLAG_FORWARD) && behind)
    {
        float predictDis = std::min(3.0f, target->GetObjectSize() * 2);
        tx += cos(target->GetOrientation()) * predictDis;
        ty += sin(target->GetOrientation()) * predictDis;
        if (!target->GetMap()->CheckCollisionAndGetValidCoords(target, target->GetPositionX(), target->GetPositionY(),
                                                               target->GetPositionZ(), tx, ty, tz))
        {
            tx = target->GetPositionX();
            ty = target->GetPositionY();
            tz = target->GetPositionZ();
        }
    }
    float combatDistance = bot->GetCombatReach() + target->GetCombatReach();
    distance += combatDistance;

    if (bot->GetExactDist(tx, ty, tz) <= distance)
        return false;

    PathGenerator path(bot);
    // Soft bias: STEEP / WATER are reachable but de-prioritised so the
    // bot picks normal ground when an alternative exists.
    path.SetNavTerrainCost(NAV_GROUND_STEEP, 5.0f);
    path.SetNavTerrainCost(NAV_WATER, 10.0f);
    path.CalculatePath(tx, ty, tz, false);
    PathType type = path.GetPathType();
    int typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT;
    if (!(type & typeOk))
        return false;
    float shortenTo = distance;

    // Avoid walking too far when moving towards each other
    float disToGo = bot->GetExactDist(tx, ty, tz) - distance;
    if (disToGo >= 6.0f)
        shortenTo = disToGo / 2 + distance;

    // if (bot->GetExactDist(tx, ty, tz) <= shortenTo)
    //     return false;

    path.ShortenPathUntilDist(G3D::Vector3(tx, ty, tz), shortenTo);
    G3D::Vector3 endPos = path.GetPath().back();
    // Combat callers pass ignoreEnemyTargets=true so ClipPath doesn't
    // halt the chase at an intermediate hostile when funnelling through
    // MoveTo2 — the chase target itself is the enemy we want to reach.
    // react=true skips the end-of-dispatch WaitForReach so the bot keeps
    // re-evaluating mid-chase instead of waiting for the spline to play
    // out (which would suspend combat reactions for seconds at a time).
    bool moved = MoveTo(target->GetMapId(), endPos.x, endPos.y, endPos.z, /*idle*/false, /*react*/true, false, false,
                        MovementPriority::MOVEMENT_COMBAT, /*lessDelay*/true, false, /*ignoreEnemyTargets*/true);
    // Only emit on a successful new commit — combat ticks call this
    // many times per second and MoveTo internally suppresses while a
    // prior spline is still playing. Emitting before the suppression
    // check produces per-tick whisper spam.
    if (moved)
        EmitDebugMove("ReachCombatTo", "mmap", endPos.x, endPos.y, endPos.z);
    return moved;
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group)
        return 0.0f;

    // Prevent bots with orphaned raid groups from dividing by 0, which freezes the server.
    uint32 memberCount = group->GetMembersCount();
    if (memberCount <= 1)
        return 0.0f;

    uint32 index = 1;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->GetSource() == master)
            continue;

        if (ref->GetSource() == bot)
            return 2 * M_PI / (memberCount - 1) * index;

        ++index;
    }

    return 0;
}

bool MovementAction::IsMovingAllowed(WorldObject* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    // float distance = ServerFacade::instance().GetDistance2d(bot, target);
    // if (!bot->InBattleground() && distance > sPlayerbotAIConfig.reactDistance)
    //     return false;

    return IsMovingAllowed();
}

bool MovementAction::IsDuplicateMove(float x, float y, float z)
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    // heuristic 5s
    if (lastMove.msTime + sPlayerbotAIConfig.maxWaitForMove < getMSTime() ||
        lastMove.lastMoveShort.GetExactDist(x, y, z) > 0.01f)
        return false;

    return true;
}

bool MovementAction::IsMovingAllowed()
{
    return botAI->CanMove();
}

bool MovementAction::Follow(Unit* target, float distance) { return Follow(target, distance, GetFollowAngle()); }

void MovementAction::UpdateMovementState()
{
    const bool isCurrentlyRestricted =  // see if the bot is currently slowed, rooted, or otherwise unable to move
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->IsRooted() || bot->isFrozen() || bot->IsPolymorphed();

    // no update movement flags while movement is current restricted.
    if (!isCurrentlyRestricted && bot->IsAlive())
    {
        // state flags
        const auto master = botAI ? botAI->GetMaster() : nullptr;
        const auto liquidState = bot->GetLiquidData().Status;
        const float gZ = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        const bool onGroundZ = bot->GetPositionZ() < gZ + 1.f;
        const bool wantsSwim = liquidState == LIQUID_MAP_IN_WATER || liquidState == LIQUID_MAP_UNDER_WATER;
        const bool wantsFly = bot->HasIncreaseMountedFlightSpeedAura() || bot->HasFlyAura();
        const bool canWaterWalk = bot->HasWaterWalkAura();
        const bool isMasterFlying = master ? master->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) : true;
        const bool isMasterSwimming = master ? master->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) : true;
        const bool isFlying = bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING);
        const bool isSwimming = bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
        const bool isWaterWalking = bot->HasUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
        const bool hasGravityDisabled = bot->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
        bool movementFlagsUpdated = false;

        // handle water (fragile logic do not alter without testing every detail, animation and transition)
        if (liquidState != LIQUID_MAP_NO_WATER && !isFlying)
        {
            if (canWaterWalk && !isMasterSwimming && !isWaterWalking)
            {
                bot->SetSwim(false);
                bot->AddUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
                movementFlagsUpdated = true;
            }
            else if ((!canWaterWalk || isMasterSwimming) && isWaterWalking)
            {
                bot->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
                if (wantsSwim)
                    bot->SetSwim(true);
                movementFlagsUpdated = true;
            }
            else if (!wantsSwim && isSwimming)
            {
                bot->SetSwim(false);
                movementFlagsUpdated = true;
            }
        }

        // reset when not around water while swimming or water walking
        if (liquidState == LIQUID_MAP_NO_WATER && (isSwimming || isWaterWalking))
        {
            bot->SetSwim(false);
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
            movementFlagsUpdated = true;
        }

        // handle flying
        if (wantsFly && !isFlying && isMasterFlying)
        {
            bot->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
            bot->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
            bot->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            movementFlagsUpdated = true;
        }
        else if (!wantsFly && !isWaterWalking && (isFlying || hasGravityDisabled))
        {
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
            movementFlagsUpdated = true;
        }
        else if (!isMasterFlying && isFlying && onGroundZ)
        {
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
            movementFlagsUpdated = true;
        }

        // detect if movement/CC restrictions have been ended, refresh movement state for animations.
        if (wasMovementRestricted)
            movementFlagsUpdated = true;

        // movement flags should only be updated between state changes, if not it will break certain effects.
        if (movementFlagsUpdated)
            bot->SendMovementFlagUpdate();
    }

    // Save current state for the next check
    wasMovementRestricted = isCurrentlyRestricted;

    // Temporary speed increase in group
    // if (botAI->HasRealPlayerMaster())
    // {
    //     bot->SetSpeedRate(MOVE_RUN, 1.1f);
    // }
    // else
    // {
    //     bot->SetSpeedRate(MOVE_RUN, 1.0f);
    // }
    // check if target is not reachable
    // if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE && bot->CanNotReachTarget() &&
    // !bot->InBattleground())
    // {
    //     if (Unit* pTarget = ServerFacade::instance().GetChaseTarget(bot))
    //     {
    //         if (!bot->IsWithinMeleeRange(pTarget) && pTarget->IsCreature())
    //         {
    //             float angle = bot->GetAngle(pTarget);
    //             float distance = 5.0f;
    //             float x = bot->GetPositionX() + cos(angle) * distance;
    //             float y = bot->GetPositionY() + sin(angle) * distance;
    //             float z = bot->GetPositionZ();

    //             z += CONTACT_DISTANCE;
    //             bot->UpdateAllowedPositionZ(x, y, z);

    //             bot->StopMoving();
    //             bot->GetMotionMaster()->Clear();
    //             bot->NearTeleportTo(x, y, z, bot->GetOrientation());
    //             //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
    //             return;
    //             /*if (pTarget->IsCreature() && !bot->isMoving() && bot->IsWithinDist(pTarget, 10.0f))
    //             {
    //                 // Cheating to prevent getting stuck because of bad mmaps.
    //                 bot->StopMoving();
    //                 bot->GetMotionMaster()->Clear();
    //                 bot->GetMotionMaster()->MovePoint(bot->GetMapId(), pTarget->GetPosition(), FORCED_MOVEMENT_RUN,
    //                 false); return;
    //             }*/
    //         }
    //     }
    // }

    // if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE ||
    //     bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE) && bot->CanNotReachTarget()
    //     && !bot->InBattleground())
    // {
    //     if (Unit* pTarget = ServerFacade::instance().GetChaseTarget(bot))
    //     {
    //         if (pTarget != botAI->GetGroupLeader())
    //             return;

    //         if (!bot->IsWithinMeleeRange(pTarget))
    //         {
    //             if (!bot->isMoving() && bot->IsWithinDist(pTarget, 10.0f))
    //             {
    //                 // Cheating to prevent getting stuck because of bad mmaps.
    //                 float angle = bot->GetAngle(pTarget);
    //                 float distance = 5.0f;
    //                 float x = bot->GetPositionX() + cos(angle) * distance;
    //                 float y = bot->GetPositionY() + sin(angle) * distance;
    //                 float z = bot->GetPositionZ();

    //                 z += CONTACT_DISTANCE;
    //                 bot->UpdateAllowedPositionZ(x, y, z);

    //                 bot->StopMoving();
    //                 bot->GetMotionMaster()->Clear();
    //                 bot->NearTeleportTo(x, y, z, bot->GetOrientation());
    //                 //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
    //                 return;
    //             }
    //         }
    //     }
    // }
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    if (!target)
        return false;

    // Unsafe target (cross-faction / phased / leaving) — fall through to
    // a generic MoveTo so the bot at least heads in their direction
    // instead of refusing to move.
    if (!botAI->IsSafe(target))
        return MoveTo(target, distance);

    // Subtract the target's hitbox so we end up at the requested
    // standoff from its edge, not from its centre.
    distance = distance <= target->GetObjectSize()
        ? 0.0f
        : distance - target->GetObjectSize();

    UpdateMovementState();

    // Cross-transport follow: if bot and target are on different
    // transports (or only one is on a transport) and within sight,
    // disembark/teleport/board to match. Handled here before the
    // distance gates so a bot on a stationary boat following a
    // master who just boarded doesn't get stuck at "no need to follow".
    if (FollowOnTransport(target))
        return true;

    if (!bot->InBattleground() && ServerFacade::instance().IsDistanceLessOrEqualThan(ServerFacade::instance().GetDistance2d(bot, target),
                                                                           sPlayerbotAIConfig.followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }


    // Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetGUID().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition botPos(bot);
            WorldPosition cPos(corpse);

            if (botPos.fDist(cPos) > sPlayerbotAIConfig.spellDistance)
                return MoveTo(cPos.GetMapId(), cPos.GetPositionX(), cPos.GetPositionY(), cPos.GetPositionZ());
        }
    }

    if (ServerFacade::instance().IsDistanceGreaterOrEqualThan(ServerFacade::instance().GetDistance2d(bot, target),
                                                    sPlayerbotAIConfig.sightDistance))
    {
        EmitDebugMove("Follow", "mmap", target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

        if (target->GetGUID().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            PlayerbotAI* targetBotAI = GET_PLAYERBOT_AI(pTarget);
            if (targetBotAI)  // Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove =
                    targetBotAI->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < static_cast<float>(M_PI) / 2) ||
                        target->HasUnitState(UNIT_STATE_IN_FLIGHT))
                    {
                        return MoveTo(longMove.GetMapId(), longMove.GetPositionX(), longMove.GetPositionY(), longMove.GetPositionZ());
                    }
                }
            }
            else
            {
                if (pTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))  // Move to where the player is flying to.
                {
                    TaxiPathNodeList const& tMap =
                        static_cast<FlightPathMovementGenerator*>(pTarget->GetMotionMaster()->top())->GetPath();
                    if (!tMap.empty())
                    {
                        auto tEnd = tMap.back();
                        if (tEnd)
                            return MoveTo(tEnd->mapid, tEnd->x, tEnd->y, tEnd->z);
                    }
                }
            }
        }

        if (!target->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return MoveTo(target, sPlayerbotAIConfig.followDistance);
    }

    if (ServerFacade::instance().IsDistanceLessOrEqualThan(ServerFacade::instance().GetDistance2d(bot, target),
                                                 sPlayerbotAIConfig.followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    if (target->IsFriendlyTo(bot) && bot->IsMounted() && AI_VALUE(GuidVector, "all targets").empty())
        distance += angle;

    if (!bot->InBattleground() && ServerFacade::instance().IsDistanceLessOrEqualThan(ServerFacade::instance().GetDistance2d(bot, target),
                                                                           sPlayerbotAIConfig.followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    bot->HandleEmoteCommand(0);

    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    // AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = ServerFacade::instance().GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetGUID() == target->GetGUID())
            return false;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        bot->GetMotionMaster()->Clear();

    EmitDebugMove("Follow", "follow", target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
    bot->GetMotionMaster()->MoveFollow(target, distance, angle);
    return true;
}

bool MovementAction::FollowOnTransport(Unit* target)
{
    if (!target)
        return false;

    bool const onDifferentTransports =
        bot->m_movementInfo.transport.guid != target->m_movementInfo.transport.guid;
    if (!onDifferentTransports)
        return false;

    if (!ServerFacade::instance().IsDistanceLessOrEqualThan(
            ServerFacade::instance().GetDistance2d(bot, target),
            sPlayerbotAIConfig.sightDistance))
        return false;

    bot->StopMoving();

    // Disembark from our current transport (if any) before relocating.
    if (Transport* myTransport = bot->GetTransport())
        myTransport->RemovePassenger(bot);

    // NearTeleportTo is the AC equivalent of cmangos's Relocate+
    // SendHeartBeat sequence: it relocates the bot AND broadcasts the
    // movement update so server-side state stays consistent.
    bot->NearTeleportTo(target->GetPositionX(),
                        target->GetPositionY(),
                        target->GetPositionZ(),
                        bot->GetOrientation());

    // Board target's transport (if any).
    if (Transport* hisTransport = target->GetTransport())
        hisTransport->AddPassenger(bot);

    return true;
}

bool MovementAction::ChaseTo(WorldObject* obj, float distance)
{
    if (!IsMovingAllowed())
        return false;

    if (obj)
        EmitDebugMove("ChaseTo", "chase", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());

    UpdateMovementState();

    // Drop any looping emote (sit/dance/etc.) before the chase, matching
    // the reference pre-dispatch normalization.
    bot->ClearEmoteState();

    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    // Try a chained mmap probe first — for targets behind obstacles
    // this routes the bot around terrain instead of straight-charging
    // into a wall. Falls back to engine MoveChase for short/clear
    // chases where target tracking matters more than path routing.
    float const targetDist = bot->GetExactDist(obj);
    if (targetDist > distance + 3.0f)
    {
        float const angle = obj->GetAngle(bot);
        float x = obj->GetPositionX();
        float y = obj->GetPositionY();
        float z = obj->GetPositionZ();
        obj->GetNearPoint(bot, x, y, z, bot->GetObjectSize(), distance, angle);

        PathGenerator path(bot);
        path.CalculatePath(x, y, z, false);
        PathType type = path.GetPathType();
        if ((type & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT)) &&
            path.GetPath().size() >= 2)
        {
            Movement::PointsArray points = path.GetPath();
            bot->GetMotionMaster()->Clear();
            bot->GetMotionMaster()->MoveSplinePath(&points, FORCED_MOVEMENT_RUN);
            WaitForReach(targetDist - distance);
            return true;
        }
    }

    bot->GetMotionMaster()->MoveChase((Unit*)obj, distance);
    WaitForReach(bot->GetExactDist2d(obj) - distance);
    return true;
}

float MovementAction::MoveDelay(float distance, bool backwards)
{
    float speed;
    if (bot->isSwimming())
    {
        speed = backwards ? bot->GetSpeed(MOVE_SWIM_BACK) : bot->GetSpeed(MOVE_SWIM);
    }
    else if (bot->IsFlying())
    {
        speed = backwards ? bot->GetSpeed(MOVE_FLIGHT_BACK) : bot->GetSpeed(MOVE_FLIGHT);
    }
    else
    {
        speed = backwards ? bot->GetSpeed(MOVE_RUN_BACK) : bot->GetSpeed(MOVE_RUN);
    }
    float delay = distance / speed;
    return delay;
}

void MovementAction::WaitForReach(float distance)
{
    // Reference formula: 1000 * MoveDelay(distance) + reactDelay. The
    // reactDelay gives the bot a small slack at the end of a move so the
    // next action sees the bot at rest rather than mid-spline.
    float delay = 1000.0f * MoveDelay(distance) + sPlayerbotAIConfig.reactDelay;

    if (delay > sPlayerbotAIConfig.maxWaitForMove)
        delay = sPlayerbotAIConfig.maxWaitForMove;

    // Combat clamp deliberately disabled (reference comments it out):
    // clamping to globalCoolDown in combat caused bots to re-evaluate
    // mid-pull and abandon a chase before reaching attack range. The
    // commented block is preserved verbatim should we need to revisit.
    /*
    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && delay > sPlayerbotAIConfig.globalCoolDown)
        delay = sPlayerbotAIConfig.globalCoolDown;
    */

    if (delay < 0)
        delay = 0;

    // Reference uses SetDuration on the Action; AC's equivalent is
    // SetNextCheckDelay on the AI loop. Same outcome: re-evaluation is
    // postponed by `delay` ms.
    botAI->SetNextCheckDelay((uint32)delay);
}

void MovementAction::WaitForReach(Movement::PointsArray const& path)
{
    float distance = 0.0f;
    if (!path.empty())
    {
        G3D::Vector3 const* previousPoint = &path[0];
        for (auto it = path.begin() + 1; it != path.end(); ++it)
        {
            G3D::Vector3 const& pathPoint = (*it);
            distance += (*previousPoint - pathPoint).length();
            previousPoint = &pathPoint;
        }
    }
    WaitForReach(distance);
}

// similiar to botAI->SetNextCheckDelay() but only stops movement
void MovementAction::SetNextMovementDelay(float delayMillis)
{
    AI_VALUE(LastMovement&, "last movement")
        .Set(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation(),
             delayMillis, MovementPriority::MOVEMENT_FORCED);
}

bool MovementAction::Flee(Unit* target)
{
    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!sPlayerbotAIConfig.fleeingEnabled)
        return false;

    EmitDebugMove("Flee", "flee", target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

    if (!IsMovingAllowed())
    {
        botAI->TellError("I am stuck while fleeing");
        return false;
    }

    bool foundFlee = false;
    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
    time_t now = time(0);
    uint32 fleeDelay = urand(2, sPlayerbotAIConfig.returnDelay / 1000);

    if (lastFlee)
    {
        if ((now - lastFlee) <= fleeDelay)
        {
            return false;
        }
    }

    Unit* currentVictim = target->GetThreatMgr().GetCurrentVictim();
    if (currentVictim && currentVictim == bot)  // bot is target - try to flee to tank or master
    {
        if (Group* group = bot->GetGroup())
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = sPlayerbotAIConfig.sightDistance;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if (botAI->IsTank(player))
                {
                    float distanceToTank = ServerFacade::instance().GetDistance2d(bot, player);
                    if (distanceToTank < fleeDistance)
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToTank;
                    }
                }
            }

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master)
            {
                foundFlee = MoveNear(master);
            }
        }
    }
    else  // bot is not targeted, try to flee dps/healers
    {
        bool isHealer = botAI->IsHeal(bot);
        bool needHealer = !isHealer && AI_VALUE2(uint8, "health", "self target") < 50;
        bool isRanged = botAI->IsRanged(bot);

        Group* group = bot->GetGroup();
        if (group)
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = botAI->GetRange("shoot") * 1.5f;
            Unit* spareTarget = nullptr;
            float spareDistance = botAI->GetRange("shoot") * 2.0f;
            std::vector<Unit*> possibleTargets;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if ((isHealer && botAI->IsHeal(player)) || needHealer)
                {
                    float distanceToHealer = ServerFacade::instance().GetDistance2d(bot, player);
                    float distanceToTarget = ServerFacade::instance().GetDistance2d(player, target);
                    if (distanceToHealer < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig.followDistance) &&
                        (needHealer || player->IsWithinLOSInMap(target)))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToHealer;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                else if (isRanged && botAI->IsRanged(player))
                {
                    float distanceToRanged = ServerFacade::instance().GetDistance2d(bot, player);
                    float distanceToTarget = ServerFacade::instance().GetDistance2d(player, target);
                    if (distanceToRanged < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig.followDistance) &&
                        player->IsWithinLOSInMap(target))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToRanged;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                // remember any group member in case no one else found
                float distanceToFlee = ServerFacade::instance().GetDistance2d(bot, player);
                float distanceToTarget = ServerFacade::instance().GetDistance2d(player, target);
                if (distanceToFlee < spareDistance &&
                    distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig.followDistance) &&
                    player->IsWithinLOSInMap(target))
                {
                    spareTarget = player;
                    spareDistance = distanceToFlee;
                    possibleTargets.push_back(fleeTarget);
                }
            }

            if (!possibleTargets.empty())
                fleeTarget = possibleTargets[urand(0, possibleTargets.size() - 1)];

            if (!fleeTarget)
                fleeTarget = spareTarget;

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master && master->IsAlive() && master->IsWithinLOSInMap(target))
            {
                float distanceToTarget = ServerFacade::instance().GetDistance2d(master, target);
                if (distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig.followDistance))
                    foundFlee = MoveNear(master);
            }
        }
    }

    if ((foundFlee || lastFlee) && bot->GetGroup())
    {
        if (!lastFlee)
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = now;
        }
        else
        {
            if ((now - lastFlee) > fleeDelay)
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
            }
            else
                return false;
        }
    }

    FleeManager manager(bot, botAI->GetRange("flee"), bot->GetAngle(target) + M_PI);
    if (!manager.isUseful())
        return false;

    float rx, ry, rz;
    if (!manager.CalculateDestination(&rx, &ry, &rz))
    {
        botAI->TellError("Nowhere to flee");
        return false;
    }

    bool result = MoveTo(target->GetMapId(), rx, ry, rz);

    if (result)
        AI_VALUE(LastMovement&, "last movement").lastFlee = time(nullptr);

    return result;
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<PositionMap&>("position")->Get()["random"].Reset();
}

bool MovementAction::MoveAway(Unit* target, float distance, bool backwards)
{
    if (!target)
    {
        return false;
    }
    EmitDebugMove("MoveAway", "mmap", target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
    float init_angle = target->GetAngle(bot);
    for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8)
    {
        float angle = init_angle + delta;
        float dx = bot->GetPositionX() + cos(angle) * distance;
        float dy = bot->GetPositionY() + sin(angle) * distance;
        float dz = bot->GetPositionZ();
        bool exact = true;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false,
                   backwards))
        {
            return true;
        }
        if (delta == 0)
        {
            continue;
        }
        exact = true;
        angle = init_angle - delta;
        dx = bot->GetPositionX() + cos(angle) * distance;
        dy = bot->GetPositionY() + sin(angle) * distance;
        dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false,
                   backwards))
        {
            return true;
        }
    }
    return false;
}

// just calculates average position of group and runs away from that position
bool MovementAction::MoveFromGroup(float distance)
{
    if (Group* group = bot->GetGroup())
    {
        uint32 mapId = bot->GetMapId();
        float closestDist = FLT_MAX;
        float x = 0.0f;
        float y = 0.0f;
        uint32 count = 0;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (!player || player == bot || !player->IsAlive() || player->GetMapId() != mapId)
                continue;
            float dist = bot->GetDistance2d(player);
            if (closestDist > dist)
                closestDist = dist;
            x += player->GetPositionX();
            y += player->GetPositionY();
            count++;
        }

        if (count && closestDist < distance)
        {
            x /= count;
            y /= count;
            // x and y are now average position of the group members
            float angle = bot->GetAngle(x, y) + M_PI;
            EmitDebugMove("MoveFromGroup", "mmap", x, y, bot->GetPositionZ());
            return Move(angle, distance - closestDist);
        }
    }
    return false;
}

bool MovementAction::Move(float angle, float distance)
{
    float x = bot->GetPositionX() + cos(angle) * distance;
    float y = bot->GetPositionY() + sin(angle) * distance;

    // TODO do we need GetMapWaterOrGroundLevel() if we're using CheckCollisionAndGetValidCoords() ?
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z == -100000.0f || z == -200000.0f)
        z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z, false))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z);
}

bool MovementAction::MoveInside(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    if (bot->GetDistance2d(x, y) <= distance)
    {
        return false;
    }
    EmitDebugMove("MoveInside", "mmap", x, y, z);
    return MoveNear(mapId, x, y, z, distance, priority);
}

// float MovementAction::SearchBestGroundZForPath(float x, float y, float z, bool generatePath, float range, bool
// normal_only, float step)
// {
//     if (!generatePath)
//     {
//         return z;
//     }
//     float min_length = 100000.0f;
//     float current_z = INVALID_HEIGHT;
//     float modified_z;
//     float delta;
//     for (delta = 0.0f; delta <= range / 2; delta += step) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length)
//         {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f)
//             {
//                 return current_z;
//             }
//         }
//     }
//     for (delta = -step; delta >= -range / 2; delta -= step) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length)
//         {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f)
//                 return current_z;
//         }
//     }
//     for (delta = range / 2 + step; delta <= range; delta += 2) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length)
//         {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f)
//             {
//                 return current_z;
//             }
//         }
//     }
//     if (current_z == INVALID_HEIGHT && normal_only)
//     {
//         return INVALID_HEIGHT;
//     }
//     if (current_z == INVALID_HEIGHT && !normal_only)
//     {
//         return z;
//     }
//     return current_z;
// }

PathResult MovementAction::GeneratePath(float x, float y, float z, uint32 acceptMask, bool forceDestination)
{
    PathResult result;
    PathGenerator gen(bot);
    gen.SetNavTerrainCost(NAV_GROUND_STEEP, 5.0f);
    gen.SetNavTerrainCost(NAV_WATER, 10.0f);
    gen.CalculatePath(x, y, z, forceDestination);
    result.pathType = gen.GetPathType();
    result.reachable = !(result.pathType & (~acceptMask));
    result.points = gen.GetPath();
    result.actualEnd = gen.GetActualEndPosition();
    result.end = gen.GetEndPosition();
    return result;
}

void MovementAction::DoMovePoint(Unit* unit, float x, float y, float z, bool generatePath, bool backwards)
{
    if (!unit)
        return;

    MotionMaster* mm = unit->GetMotionMaster();
    if (!mm)
        return;

    // bot water collision correction
    if (unit->HasUnitMovementFlag(MOVEMENTFLAG_WATERWALKING) && unit->HasWaterWalkAura())
    {
        float gZ = unit->GetMapWaterOrGroundLevel(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
        unit->UpdatePosition(unit->GetPositionX(), unit->GetPositionY(), gZ, false);
    }

    mm->Clear();
    if (backwards)
    {
        mm->MovePointBackwards(
            /*id*/ 0,
            /*coords*/ x, y, z,
            /*generatePath*/ generatePath,
            /*forceDestination*/ false);
        return;
    }
    else
    {
        mm->MovePoint(
            /*id*/ 0,
            /*coords*/ x, y, z,
            /*forcedMovement*/ FORCED_MOVEMENT_NONE,
            /*speed*/ 0.f,
            /*orientation*/ 0.f,
            /*generatePath*/ generatePath,  // true => terrain path (2d mmap); false => straight spline (3d vmap)
            /*forceDestination*/ false);
    }
}

bool FleeAction::Execute(Event /*event*/)
{
    return MoveAway(AI_VALUE(Unit*, "current target"), sPlayerbotAIConfig.fleeDistance, true);
}

bool FleeAction::isUseful()
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsInWorld() && !bot->IsWithinMeleeRange(target))
        return false;

    return true;
}

bool FleeWithPetAction::Execute(Event /*event*/)
{
    if (bot->GetPet())
        botAI->PetFollow();

    return Flee(AI_VALUE(Unit*, "current target"));
}

bool AvoidAoeAction::isUseful()
{
    if (getMSTime() - moveInterval < uint32(lastMoveTimer))
        return false;

    GuidVector traps = AI_VALUE(GuidVector, "nearest trap with damage");
    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    return AI_VALUE(Aura*, "area debuff") || !traps.empty() || !triggers.empty();
}

bool AvoidAoeAction::Execute(Event /*event*/)
{
    // Case #1: Aura with dynamic object (e.g. rain of fire)
    if (AvoidAuraWithDynamicObj())
    {
        return true;
    }
    // Case #2: Trap game object with spell (e.g. lava bomb)
    if (AvoidGameObjectWithDamage())
    {
        return true;
    }
    // Case #3: Trigger npc (e.g. Lesser shadow fissure)
    if (AvoidUnitWithDamageAura())
    {
        return true;
    }
    return false;
}

bool AvoidAoeAction::AvoidAuraWithDynamicObj()
{
    Aura* aura = AI_VALUE(Aura*, "area debuff");
    if (!aura || aura->IsRemoved() || aura->IsExpired())
    {
        return false;
    }
    if (!aura->GetOwner() || !aura->GetOwner()->IsInWorld())
    {
        return false;
    }
    // Crash fix: maybe change owner due to check interval
    if (aura->GetType() != DYNOBJ_AURA_TYPE)
    {
        return false;
    }
    const SpellInfo* spellInfo = aura->GetSpellInfo();
    if (!spellInfo)
    {
        return false;
    }
    if (sPlayerbotAIConfig.aoeAvoidSpellWhitelist.find(spellInfo->Id) !=
        sPlayerbotAIConfig.aoeAvoidSpellWhitelist.end())
        return false;

    DynamicObject* dynOwner = aura->GetDynobjOwner();
    if (!dynOwner || !dynOwner->IsInWorld())
    {
        return false;
    }
    float radius = dynOwner->GetRadius();
    if (!radius || radius > sPlayerbotAIConfig.maxAoeAvoidRadius)
        return false;
    if (bot->GetDistance(dynOwner) > radius)
    {
        return false;
    }
    std::ostringstream name;
    name << spellInfo->SpellName[LOCALE_enUS];  // << "] (aura)";
    if (FleePosition(dynOwner->GetPosition(), radius))
    {
        if (sPlayerbotAIConfig.tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
        {
            lastTellTimer = time(NULL);
            lastMoveTimer = getMSTime();
            std::ostringstream out;
            out << "I'm avoiding " << name.str() << " (" << spellInfo->Id << ")" << " Radius " << radius << " - [Aura]";
            bot->Say(out.str(), LANG_UNIVERSAL);
        }
        return true;
    }
    return false;
}

bool AvoidAoeAction::AvoidGameObjectWithDamage()
{
    GuidVector traps = AI_VALUE(GuidVector, "nearest trap with damage");
    if (traps.empty())
    {
        return false;
    }
    for (ObjectGuid& guid : traps)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || !go->IsInWorld())
        {
            continue;
        }
        if (go->GetGoType() != GAMEOBJECT_TYPE_TRAP)
        {
            continue;
        }
        const GameObjectTemplate* goInfo = go->GetGOInfo();
        if (!goInfo)
        {
            continue;
        }
        // 0 trap with no despawn after cast. 1 trap despawns after cast. 2 bomb casts on spawn.
        if (goInfo->trap.type != 0)
            continue;

        uint32 spellId = goInfo->trap.spellId;
        if (!spellId)
        {
            continue;
        }

        if (sPlayerbotAIConfig.aoeAvoidSpellWhitelist.find(spellId) !=
            sPlayerbotAIConfig.aoeAvoidSpellWhitelist.end())
            continue;

        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPositive())
        {
            continue;
        }

        float radius = (float)goInfo->trap.diameter / 2 + go->GetCombatReach();
        if (!radius || radius > sPlayerbotAIConfig.maxAoeAvoidRadius)
            continue;

        if (bot->GetDistance(go) > radius)
        {
            continue;
        }
        std::ostringstream name;
        name << spellInfo->SpellName[LOCALE_enUS];  // << "] (object)";
        if (FleePosition(go->GetPosition(), radius))
        {
            if (sPlayerbotAIConfig.tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
            {
                lastTellTimer = time(NULL);
                lastMoveTimer = getMSTime();
                std::ostringstream out;
                out << "I'm avoiding " << name.str() << " (" << spellInfo->Id << ")" << " Radius " << radius
                    << " - [Trap]";
                bot->Say(out.str(), LANG_UNIVERSAL);
            }
            return true;
        }
    }
    return false;
}

bool AvoidAoeAction::AvoidUnitWithDamageAura()
{
    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    if (triggers.empty())
    {
        return false;
    }
    for (ObjectGuid& guid : triggers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld())
        {
            continue;
        }
        if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            return false;
        }
        Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        for (const Unit::AuraEffectList& list : {aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell})
        {
            for (auto i = list.begin(); i != list.end(); ++i)
            {
                AuraEffect* aurEff = *i;
                const SpellInfo* spellInfo = aurEff->GetSpellInfo();
                if (!spellInfo)
                    continue;
                const SpellInfo* triggerSpellInfo =
                    sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
                if (!triggerSpellInfo)
                    continue;
                if (sPlayerbotAIConfig.aoeAvoidSpellWhitelist.find(triggerSpellInfo->Id) !=
                    sPlayerbotAIConfig.aoeAvoidSpellWhitelist.end())
                    return false;
                for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
                {
                    if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                    {
                        float radius = triggerSpellInfo->Effects[j].CalcRadius();
                        if (bot->GetDistance(unit) > radius)
                        {
                            break;
                        }
                        if (!radius || radius > sPlayerbotAIConfig.maxAoeAvoidRadius)
                            continue;
                        std::ostringstream name;
                        name << triggerSpellInfo->SpellName[LOCALE_enUS];  //<< "] (unit)";
                        if (FleePosition(unit->GetPosition(), radius))
                        {
                            if (sPlayerbotAIConfig.tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
                            {
                                lastTellTimer = time(NULL);
                                lastMoveTimer = getMSTime();
                                std::ostringstream out;
                                out << "I'm avoiding " << name.str() << " (" << triggerSpellInfo->Id << ")"
                                    << " Radius " << radius << " - [Unit Trigger]";
                                bot->Say(out.str(), LANG_UNIVERSAL);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

Position MovementAction::BestPositionForMeleeToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        bool isTanking = (!currentTarget->isFrozen()
            && !currentTarget->HasRootAura()) && (currentTarget->GetVictim() == bot);
        float angle = bot->GetAngle(currentTarget);
        float angleLeft = angle + (float)M_PI / 2;
        float angleRight = angle - (float)M_PI / 2;
        possibleAngles.push_back({angleLeft, false});
        possibleAngles.push_back({angleRight, false});
        possibleAngles.push_back({angle, true});
        if (isTanking)
        {
            possibleAngles.push_back({angle + (float)M_PI, false});
            possibleAngles.push_back({bot->GetAngle(&pos) - (float)M_PI, false});
        }
    }
    else
    {
        float angleTo = bot->GetAngle(&pos) - (float)M_PI;
        possibleAngles.push_back({angleTo, false});
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig.fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{dx, dy, dz};
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() >
                sPlayerbotAIConfig.tooCloseDistance &&
            bot->IsWithinMeleeRange(currentTarget))
        {
            continue;
        }
        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

Position MovementAction::BestPositionForRangedToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    float angleToTarget = 0.0f;
    float angleFleeFromCenter = bot->GetAngle(&pos) - (float)M_PI;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        angleToTarget = bot->GetAngle(currentTarget);
        float angleLeft = angleToTarget + (float)M_PI / 2;
        float angleRight = angleToTarget - (float)M_PI / 2;
        possibleAngles.push_back({angleLeft, false});
        possibleAngles.push_back({angleRight, false});
        possibleAngles.push_back({angleToTarget + (float)M_PI, true});
        possibleAngles.push_back({angleToTarget, true});
        possibleAngles.push_back({angleFleeFromCenter, true});
    }
    else
    {
        possibleAngles.push_back({angleFleeFromCenter, false});
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig.fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{dx, dy, dz};
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() > sPlayerbotAIConfig.spellDistance)
        {
            continue;
        }
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() <
                (sPlayerbotAIConfig.tooCloseDistance))
        {
            continue;
        }

        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

bool MovementAction::FleePosition(Position pos, float radius, uint32 minInterval)
{
    std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");

    if (!infoList.empty() && infoList.back().timestamp + minInterval > getMSTime())
        return false;

    Position bestPos;
    if (botAI->IsMelee(bot))
    {
        bestPos = BestPositionForMeleeToFlee(pos, radius);
    }
    else
    {
        bestPos = BestPositionForRangedToFlee(pos, radius);
    }
    if (bestPos != Position())
    {
        EmitDebugMove("FleePosition", "mmap", bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ());
        if (MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), false,
                   false, true, false, MovementPriority::MOVEMENT_COMBAT))
        {
            uint32 curTS = getMSTime();
            while (!infoList.empty())
            {
                if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
                {
                    infoList.pop_front();
                }
                else
                {
                    break;
                }
            }
            infoList.push_back({pos, radius, bot->GetAngle(&bestPos), curTS});
            return true;
        }
    }
    return false;
}

bool MovementAction::CheckLastFlee(float curAngle, std::list<FleeInfo>& infoList)
{
    uint32 curTS = getMSTime();
    curAngle = Position::NormalizeOrientation(curAngle);
    while (!infoList.empty())
    {
        if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
        {
            infoList.pop_front();
        }
        else
        {
            break;
        }
    }
    for (FleeInfo& info : infoList)
    {
        // more than 5 sec
        if (info.timestamp + 5000 < curTS)
        {
            continue;
        }
        float revAngle = Position::NormalizeOrientation(info.angle + M_PI);
        // angle too close
        if (fabs(revAngle - curAngle) < M_PI / 4)
        {
            return false;
        }
    }
    return true;
}

bool CombatFormationMoveAction::isUseful()
{
    if (getMSTime() - moveInterval < uint32(lastMoveTimer))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    return true;
}

bool CombatFormationMoveAction::Execute(Event /*event*/)
{
    float dis = AI_VALUE(float, "disperse distance");
    if (dis <= 0.0f || (!bot->IsInCombat() && botAI->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT)) ||
        (bot->IsInCombat() && botAI->HasStrategy("stay", BotState::BOT_STATE_COMBAT)))
        return false;
    Player* playerToLeave = NearestGroupMember(dis);
    if (playerToLeave && bot->GetExactDist(playerToLeave) < dis)
    {
        if (FleePosition(playerToLeave->GetPosition(), dis))
        {
            lastMoveTimer = getMSTime();
        }
    }
    return false;
}

Position CombatFormationMoveAction::AverageGroupPos(float dis, bool ranged, bool self)
{
    float averageX = 0, averageY = 0, averageZ = 0;
    int cnt = 0;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return Position();
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerbotAI::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            ServerFacade::instance().GetDistance2d(bot, member) > dis)
            continue;

        averageX += member->GetPositionX();
        averageY += member->GetPositionY();
        averageZ += member->GetPositionZ();
    }
    averageX /= cnt;
    averageY /= cnt;
    averageZ /= cnt;
    return Position(averageX, averageY, averageZ);
}

float CombatFormationMoveAction::AverageGroupAngle(Unit* from, bool ranged, bool self)
{
    Group* group = bot->GetGroup();
    if (!from || !group)
    {
        return 0.0f;
    }
    // float average = 0.0f;
    float sumX = 0.0f;
    float sumY = 0.0f;
    int cnt = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerbotAI::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            ServerFacade::instance().GetDistance2d(bot, member) > sPlayerbotAIConfig.sightDistance)
            continue;

        cnt++;
        sumX += member->GetPositionX() - from->GetPositionX();
        sumY += member->GetPositionY() - from->GetPositionY();
    }
    if (cnt == 0)
        return 0.0f;

    // unnecessary division
    // sumX /= cnt;
    // sumY /= cnt;

    return atan2(sumY, sumX);
}

Position CombatFormationMoveAction::GetNearestPosition(const std::vector<Position>& positions)
{
    Position result;
    for (const Position& pos : positions)
    {
        if (bot->GetExactDist(pos) < bot->GetExactDist(result))
            result = pos;
    }
    return result;
}

Player* CombatFormationMoveAction::NearestGroupMember(float dis)
{
    float nearestDis = 10000.0f;
    Player* result = nullptr;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return result;
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive() || member == bot || member->GetMapId() != bot->GetMapId() ||
            member->IsCharmed() || ServerFacade::instance().GetDistance2d(bot, member) > dis)
            continue;
        if (nearestDis > bot->GetExactDist(member))
        {
            result = member;
            nearestDis = bot->GetExactDist(member);
        }
    }
    return result;
}

bool TankFaceAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->GetGroup())
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    if (!AI_VALUE2(bool, "has aggro", "current target"))
        return false;

    float averageAngle = AverageGroupAngle(target, true);

    if (averageAngle == 0.0f)
        return false;

    float deltaAngle = Position::NormalizeOrientation(averageAngle - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI;  // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(averageAngle + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(averageAngle - M_PI * 3 / 5);

    // if dist < bot->GetMeleeRange(target) / 2, target will move backward
    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() -
                 target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), x, y, z))
    {
        /// @todo: movement control now is a mess, prepare to rewrite
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool RearFlankAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    // Need to double the front angle check to account for mirrored angle.
    bool inFront = target->HasInArc(2.f * minAngle, bot);
    // Rear check does not need to double this angle as the logic is inverted
    // and we are subtracting from 2pi.
    bool inRear = !target->HasInArc((2.f * M_PI) - maxAngle, bot);

    return inFront || inRear;
}

bool RearFlankAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = frand(minAngle, maxAngle);
    float baseDistance = bot->GetMeleeRange(target) * 0.5f;
    Position leftFlank = target->GetPosition();
    Position rightFlank = target->GetPosition();
    Position* destination = nullptr;
    leftFlank.RelocatePolarOffset(angle, baseDistance + distance);
    rightFlank.RelocatePolarOffset(-angle, baseDistance + distance);

    if (bot->GetExactDist2d(leftFlank) < bot->GetExactDist2d(rightFlank))
    {
        destination = &leftFlank;
    }
    else
    {
        destination = &rightFlank;
    }

    return MoveTo(bot->GetMapId(), destination->GetPositionX(), destination->GetPositionY(),
                  destination->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool DisperseSetAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "disable")
    {
        RESET_AI_VALUE(float, "disperse distance");
        botAI->TellMasterNoFacing("Disable disperse");
        return true;
    }
    if (text == "enable" || text == "reset")
    {
        if (botAI->IsMelee(bot))
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_MELEE);
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_RANGED);
        }
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        out << "Enable disperse distance " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "increase")
    {
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        if (dis <= 0.0f)
        {
            out << "Enable disperse first";
            botAI->TellMasterNoFacing(out.str());
            return true;
        }
        dis += 1.0f;
        SET_AI_VALUE(float, "disperse distance", dis);
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "decrease")
    {
        float dis = AI_VALUE(float, "disperse distance");
        dis -= 1.0f;
        if (dis <= 0.0f)
        {
            dis += 1.0f;
        }
        SET_AI_VALUE(float, "disperse distance", dis);
        std::ostringstream out;
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text.starts_with("set"))
    {
        float dis = -1.0f;
        ;
        sscanf(text.c_str(), "set %f", &dis);
        std::ostringstream out;
        if (dis < 0 || dis > 100.0f)
        {
            out << "Invalid disperse distance " << std::setprecision(2) << dis;
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", dis);
            out << "Set disperse distance to " << std::setprecision(2) << dis;
        }
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    std::ostringstream out;
    out << "Usage: disperse [enable | disable | increase | decrease | set {distance}]";
    float dis = AI_VALUE(float, "disperse distance");
    if (dis > 0.0f)
    {
        out << "(Current disperse distance: " << std::setprecision(2) << dis << ")";
    }
    botAI->TellMasterNoFacing(out.str());
    return true;
}

bool RunAwayAction::Execute(Event /*event*/) { return Flee(AI_VALUE(Unit*, "group leader")); }

bool MoveToLootAction::Execute(Event /*event*/)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return MoveNear(loot.GetWorldObject(bot), sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful() { return AI_VALUE2(bool, "inside target", "current target"); }

bool SetFacingTargetAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return true;

    ServerFacade::instance().SetFacingTo(bot, target);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
    return true;
}

bool SetFacingTargetAction::isUseful() { return !AI_VALUE2(bool, "facing", "current target"); }

bool SetFacingTargetAction::isPossible()
{
    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() || bot->HasConfuseAura() || bot->IsCharmed() || bot->HasStunAura() ||
        bot->IsInFlight() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (target->GetVictim() == bot)
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    float deltaAngle = Position::NormalizeOrientation(target->GetOrientation() - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI;  // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(target->GetOrientation() + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(target->GetOrientation() - M_PI * 3 / 5);

    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() -
                 target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), x, y, z))
    {
        /// @todo: movement control now is a mess, prepare to rewrite
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MoveOutOfCollisionAction::Execute(Event /*event*/)
{
    float angle = M_PI * 2000 / frand(1.f, 1000.f);
    float distance = sPlayerbotAIConfig.followDistance;
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance,
                  bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    // do not avoid collision on vehicle
    if (botAI->IsInVehicle())
        return false;

    return AI_VALUE2(bool, "collision", "self target") &&
           botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get().size() < 15;
}

bool MoveRandomAction::Execute(Event /*event*/)
{
    float distance = sPlayerbotAIConfig.tooCloseDistance + urand(10, 30);

    Map* map = bot->GetMap();
    for (int i = 0; i < 3; ++i)
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = bot->GetPositionZ();
        float angle = (float)rand_norm() * static_cast<float>(M_PI);
        x += urand(0, distance) * cos(angle);
        y += urand(0, distance) * sin(angle);

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;
        if (map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
            continue;

        bool moved = MoveTo(bot->GetMapId(), x, y, z, false, false, false, true);
        if (moved)
            return true;
    }

    return false;
}

bool MoveRandomAction::isUseful() { return !AI_VALUE(GuidPosition, "rpg target"); }

bool MoveInsideAction::Execute(Event /*event*/) { return MoveInside(bot->GetMapId(), x, y, bot->GetPositionZ(), distance); }

bool RotateAroundTheCenterPointAction::Execute(Event /*event*/)
{
    uint32 next_point = GetCurrWaypoint();
    if (MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ(), false,
               false, false, false, MovementPriority::MOVEMENT_COMBAT))
    {
        call_counters += 1;
        return true;
    }
    return false;
}

bool MoveFromGroupAction::Execute(Event event)
{
    float distance = atoi(event.getParam().c_str());
    if (!distance)
        distance = 20.0f;  // flee distance from config is too small for this
    return MoveFromGroup(distance);
}

bool MoveAwayFromCreatureAction::Execute(Event /*event*/)
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");

    // Find all creatures with the specified Id
    std::vector<Unit*> creatures;
    for (auto const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && (alive && unit->IsAlive()) && unit->GetEntry() == creatureId)
        {
            creatures.push_back(unit);
        }
    }

    if (creatures.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= 30.0f; distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromCreatures = true;
            float minCreatureDist = std::numeric_limits<float>::max();
            for (Unit* creature : creatures)
            {
                float dist = creature->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromCreatures = false;
                    break;
                }
                if (dist < minCreatureDist)
                {
                    minCreatureDist = dist;
                }
            }

            if (isSafeFromCreatures && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any creature. Higher is better.
                if (minCreatureDist > maxSafetyScore)
                {
                    maxSafetyScore = minCreatureDist;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool MoveAwayFromCreatureAction::isPossible() { return bot->CanFreeMove(); }

bool MoveAwayFromPlayerWithDebuffAction::Execute(Event /*event*/)
{
    Group* const group = bot->GetGroup();

    if (!group)
        return false;

    std::vector<Player*> debuffedPlayers;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (player && player->IsAlive() && player->HasAura(spellId))
        {
            debuffedPlayers.push_back(player);
        }
    }

    if (debuffedPlayers.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= (range + 5.0f); distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromDebuffedPlayer = true;
            float minDebuffedPlayerDistance = std::numeric_limits<float>::max();
            for (Unit* debuffedPlayer : debuffedPlayers)
            {
                float dist = debuffedPlayer->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromDebuffedPlayer = false;
                    break;
                }
                if (dist < minDebuffedPlayerDistance)
                {
                    minDebuffedPlayerDistance = dist;
                }
            }

            if (isSafeFromDebuffedPlayer && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any debuffed player. Higher is better.
                if (minDebuffedPlayerDistance > maxSafetyScore)
                {
                    maxSafetyScore = minDebuffedPlayerDistance;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool MoveAwayFromPlayerWithDebuffAction::isPossible()
{
    return bot->CanFreeMove();
}


TravelPath MovementAction::ResolveMovePath(WorldPosition startPos,
                                           WorldPosition endPos,
                                           LastMovement& lastMove)
{
    // Invariant: a Detour pathfind must only ever run on the bot's CURRENT map.
    // The travel-node graph bridges maps via transition links (portals/flights/
    // boats) and exists ONLY on the overworld continents (0/1/530/571). If either
    // endpoint of a cross-map move is an instance, there is no graph route — the
    // only way to "reach" it would be to project the bot onto another map and live-
    // pathfind that map's navmesh query from this thread, which races that map's own
    // update thread and can corrupt the shared dtNavMeshQuery node pool into an
    // infinite loop (world-thread freeze). Refuse instead: MoveTo2 returns false on
    // an empty path, so MoveFarTo fails and the RPG layer drops the objective.
    if (startPos.GetMapId() != endPos.GetMapId() &&
        (!startPos.isOverworld() || !endPos.isOverworld()))
    {
        LOG_DEBUG("playerbots",
                  "[TravelGate] {} {} refused cross-map path: map {} ({:.0f},{:.0f},{:.0f}) "
                  "-> map {} ({:.0f},{:.0f},{:.0f}) (instance endpoint, no graph route)",
                  bot->GetName(), bot->GetGUID().ToString(),
                  startPos.GetMapId(), startPos.GetPositionX(),
                  startPos.GetPositionY(), startPos.GetPositionZ(),
                  endPos.GetMapId(), endPos.GetPositionX(),
                  endPos.GetPositionY(), endPos.GetPositionZ());
        return {};
    }

    float const totalDistance = startPos.distance(endPos);
    float const maxDistChange = totalDistance * 0.1f;

    // 10% reuse: cached path's tail close enough to new dest? Return as-is.
    if (!lastMove.lastPath.empty() &&
        lastMove.lastPath.getBack().distance(endPos) < maxDistChange)
        return lastMove.lastPath;

    // Long path = cross-map or beyond sight; otherwise pure mmap probe.
    // Map 609 (Ebon Hold, DK starter) special-case: the area is stacked
    // vertically, so a horizontally-close target on a different floor
    // needs graph routing through the spiral stairs even when within
    // sight distance.
    bool const needsLongPath =
        startPos.GetMapId() != endPos.GetMapId() ||
        totalDistance > sPlayerbotAIConfig.sightDistance ||
        (startPos.GetMapId() == 609 &&
         std::fabs(startPos.GetPositionZ() - endPos.GetPositionZ()) > 20.0f);

    TravelPath out;

    bool const usedGraph = needsLongPath &&
                           !sTravelNodeMap.getNodes().empty() && !bot->InBattleground();
    if (usedGraph)
    {
        out = sTravelNodeMap.GetFullPath(startPos, endPos, bot);
    }
    else
    {
        std::vector<WorldPosition> probe = startPos.getPathTo(endPos, bot);
        out.addPath(probe);
    }

    // Regression guard: if cached path's tail is no worse than the new
    // path's tail, keep the cached one (catches probes blocked by geometry).
    if (!lastMove.lastPath.empty() && !out.empty() &&
        lastMove.lastPath.getBack().distance(endPos) <=
            out.getBack().distance(endPos))
        out = lastMove.lastPath;

    // Last-ditch fallback: a single point at the destination, so the
    // caller has at least something to dispatch.
    if (out.empty())
    {
        // Tracking: the chosen pathfinder produced nothing, so we beeline a single
        // point at the destination. For a far target this means the travel-node graph
        // could not route there (sparse coverage / disconnected) after we declined a
        // live long-distance Detour — the exact case to watch when tuning the gates.
        // Only logged for non-trivial distances; short in-zone beelines are normal.
        // Graph-route failures are classified by [TravelFail] inside GetFullPath;
        // here we only note the direct-probe (non-graph) beelines.
        if (!usedGraph && totalDistance > sPlayerbotAIConfig.sightDistance)
            LOG_DEBUG("playerbots",
                      "[TravelGate] {} {} no path, beelining {:.0f}y: map {} "
                      "({:.0f},{:.0f},{:.0f}) -> map {} ({:.0f},{:.0f},{:.0f}) (mmap probe failed)",
                      bot->GetName(), bot->GetGUID().ToString(), totalDistance,
                      startPos.GetMapId(), startPos.GetPositionX(),
                      startPos.GetPositionY(), startPos.GetPositionZ(),
                      endPos.GetMapId(), endPos.GetPositionX(),
                      endPos.GetPositionY(), endPos.GetPositionZ());
        out.addPoint(endPos);
    }

    return out;
}

bool MovementAction::WaitForTransport()
{
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
    if (!lastMove.lastTransportEntry)
        return false;

    // Combined gate (matches reference exactly): all of these must hold
    // for us to be considered "mid-ride" on the recorded transport.
    Transport* transport = bot->GetTransport();
    if (!transport ||
        transport->GetEntry() != lastMove.lastTransportEntry ||
        lastMove.lastPath.empty() ||
        lastMove.lastPath[0].type != PathNodeType::NODE_TRANSPORT ||
        lastMove.lastPath[0].entry != lastMove.lastTransportEntry)
    {
        lastMove.lastTransportEntry = 0;
        return false;
    }

    // Run UpcommingSpecialMovement on the cached path with maxDist=0 to
    // see if the head segment is a disembark-ready special (reference
    // pattern). No special → still mid-ride, return false to let
    // MoveFarTo continue normally.
    TravelPath path = lastMove.lastPath;
    if (!path.UpcommingSpecialMovement(WorldPosition(bot), 0.0f, /*onTransport=*/true))
        return false;

    // Disembark: head is the transport node where we should get off,
    // next is the world-position dock to land at.
    if (path.size() < 2)
        return true;  // no telePoint to land at; keep waiting
    PathNodePoint const& tele = path[1];

    transport->RemovePassenger(bot);
    bot->StopMovingOnCurrentPos();
    bool const teleported = bot->TeleportTo(tele.point.GetMapId(),
                                            tele.point.GetPositionX(),
                                            tele.point.GetPositionY(),
                                            tele.point.GetPositionZ(),
                                            bot->GetOrientation());
    if (!teleported)
        return true;  // try again next tick

    lastMove.lastTransportEntry = 0;
    return false;
}

bool MovementAction::HandleSpecialMovement(TravelPath& path)
{
    if (path.empty())
        return false;

    PathNodePoint const& cur = path[0];
    bool const hasNext = path.size() > 1;

    // Head is special — dispatch based on the head segment's type.
    switch (cur.type)
    {
        case PathNodeType::NODE_STATIC_PORTAL:
        {
            if (!cur.entry)
                return false;

            // Validate the GO template is actually a teleport spellcaster.
            // Rejects mis-labeled portal entries before we waste a CMSG.
            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(cur.entry);
            if (!goInfo || (goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER &&
                            goInfo->type != GAMEOBJECT_TYPE_GOOBER))
                return false;

            uint32 const spellId = goInfo->spellcaster.spellId;
            SpellInfo const* spellInfo = SpellMgr::instance()->GetSpellInfo(spellId);
            if (!spellInfo)
                return false;

            // EffectTriggerSpell indirection: some portal GOs cast a
            // spell that triggers ANOTHER spell which is the actual
            // teleport. Follow the chain once before the TELEPORT_UNITS
            // check, matching reference behaviour.
            if (spellInfo->Effects[0].TriggerSpell)
            {
                if (SpellInfo const* triggered =
                        sSpellMgr->GetSpellInfo(spellInfo->Effects[0].TriggerSpell))
                    spellInfo = triggered;
            }

            if (!spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_UNITS))
                return false;

            // Mounted handling: dismount if not flying, or if flying low
            // enough that the drop is safe (reference threshold: 10y
            // above ground). Refuse the interact otherwise.
            if (bot->IsMounted())
            {
                if (bot->IsFlying())
                {
                    float const groundZ = bot->GetMap()->GetHeight(
                        bot->GetPhaseMask(),
                        bot->GetPositionX(),
                        bot->GetPositionY(),
                        bot->GetPositionZ());
                    if (bot->GetPositionZ() - groundZ > 10.0f)
                        return false;
                }
                bot->Dismount();
            }
            // AC-side defensive addition (no reference parallel): some
            // shapeshift forms block GO interact. Harmless when not in
            // form.
            botAI->RemoveShapeshift();

            GuidVector nearGOs = AI_VALUE(GuidVector, "nearest game objects");
            for (ObjectGuid const& guid : nearGOs)
            {
                GameObject* go = botAI->GetGameObject(guid);
                if (!go || go->GetEntry() != cur.entry)
                    continue;
                // AC's GetGameObjectIfCanInteractWith does a strict
                // type-equality check (no "any type" sentinel like the
                // reference's MAX_GAMEOBJECT_TYPE). Pass the GO's
                // actual type so both SPELLCASTER and GOOBER portals
                // (accepted at the goInfo->type check above) pass the
                // range + non-"Point" interactability gate here.
                if (!bot->GetGameObjectIfCanInteractWith(guid,
                        static_cast<GameobjectTypes>(go->GetGoType())))
                    continue;

                WorldPacket packet(CMSG_GAMEOBJ_USE);
                packet << guid;
                bot->GetSession()->QueuePacket(new WorldPacket(packet));
                return true;
            }
            return false;
        }

        case PathNodeType::NODE_AREA_TRIGGER:
        {
            if (cur.entry)
            {
                // Marker for the trigger we're walking into; server-side
                // collision handles the actual teleport. Caller still
                // dispatches the walk this tick.
                AI_VALUE(LastMovement&, "last movement").lastAreaTrigger = cur.entry;
                return false;
            }
            // No entry: direct teleport to next-point destination.
            // Reference uses the next point's stored orientation (the
            // baked exit facing), not the bot's current facing.
            if (hasNext)
            {
                PathNodePoint const& dst = path[1];
                return bot->TeleportTo(dst.point.GetMapId(),
                                       dst.point.GetPositionX(),
                                       dst.point.GetPositionY(),
                                       dst.point.GetPositionZ(),
                                       dst.point.GetOrientation());
            }
            return false;
        }

        case PathNodeType::NODE_TRANSPORT:
        {
            // Disembark: head is a transport node and bot is on one.
            // RemovePassenger + TeleportTo the next-step world position.
            if (!hasNext)
                return false;

            Transport* transport = bot->GetTransport();
            if (!transport)
                return false;

            PathNodePoint const& dst = path[1];
            transport->RemovePassenger(bot);
            bot->StopMovingOnCurrentPos();
            bool const teleported = bot->TeleportTo(dst.point.GetMapId(),
                                                    dst.point.GetPositionX(),
                                                    dst.point.GetPositionY(),
                                                    dst.point.GetPositionZ(),
                                                    bot->GetOrientation());
            AI_VALUE(LastMovement&, "last movement").lastTransportEntry = 0;
            // Throttle re-evaluation after disembark — reference adds a
            // 1s WaitForReach here to prevent rapid re-tries while the
            // post-teleport state settles.
            WaitForReach(1000.0f);
            return teleported;
        }

        default:
            break;
    }

    // Head not special — check next-step for board/taxi handlers.
    if (!hasNext)
        return false;

    PathNodePoint const& next = path[1];
    switch (next.type)
    {
        case PathNodeType::NODE_TRANSPORT:
        {
            if (!next.entry)
                return false;
            Map* map = bot->GetMap();
            if (!map)
                return false;

            // Always consume the tick (return true) + throttle 1s,
            // matching reference. Prevents per-tick board retries
            // while we wait for the transport to actually receive us.
            Transport* transport = GetTransportForPosTolerant(
                map, bot, bot->GetPhaseMask(),
                next.point.GetPositionX(),
                next.point.GetPositionY(),
                next.point.GetPositionZ());
            if (transport && transport->GetEntry() == next.entry)
            {
                if (BoardTransport(transport))
                    AI_VALUE(LastMovement&, "last movement").lastTransportEntry = next.entry;
            }

            WaitForReach(1000.0f);
            return true;
        }

        case PathNodeType::NODE_FLIGHTPATH:
        {
            if (!next.entry)
                return false;

            TravelMgr::FlightMasterInfo const* fmInfo =
                sTravelMgr.GetNearestFlightMasterInfo(bot);
            if (!fmInfo)
                return false;

            ObjectGuid fmGuid = ObjectGuid::Create<HighGuid::Unit>(
                fmInfo->templateEntry, fmInfo->dbGuid);
            Creature* flightMaster = ObjectAccessor::GetCreature(*bot, fmGuid);
            if (!flightMaster || !flightMaster->IsAlive())
                return false;

            uint32 fromTaxi = sObjectMgr->GetNearestTaxiNode(
                cur.point.GetPositionX(), cur.point.GetPositionY(),
                cur.point.GetPositionZ(), cur.point.GetMapId(),
                bot->GetTeamId());
            uint32 toTaxi = sObjectMgr->GetNearestTaxiNode(
                next.point.GetPositionX(), next.point.GetPositionY(),
                next.point.GetPositionZ(), next.point.GetMapId(),
                bot->GetTeamId());
            if (!fromTaxi || !toTaxi || fromTaxi == toTaxi)
                return false;

            std::vector<uint32> route = sTravelNodeMap.FindTaxiPath(fromTaxi, toTaxi);
            if (route.empty())
                return false;

            botAI->RemoveShapeshift();
            if (bot->IsMounted())
                bot->Dismount();

            return bot->ActivateTaxiPathTo(route, flightMaster, 0);
        }

        default:
            return false;
    }
}


Transport* MovementAction::GetTransportForPosTolerant(Map* map, WorldObject* ref, uint32 phaseMask, float x, float y, float z)
{
    if (!map || !ref)
        return nullptr;

    std::array<float, 4> const probes = { z, z + 0.5f, z + 1.5f, z - 0.5f };
    for (float const pz : probes)
    {
        if (Transport* transport = map->GetTransportForPos(phaseMask, x, y, pz, ref))
            return transport;
    }
    return nullptr;
}

bool MovementAction::FindBoardingPointOnTransport(Map* map, Transport* expectedTransport, WorldObject* ref,
    float refX, float refY, float refZ, float botX, float botY, float botZ, float& outX, float& outY, float& outZ)
{
    if (!map || !expectedTransport || !ref)
        return false;

    uint32 const phaseMask = ref->GetPhaseMask();
    if (GetTransportForPosTolerant(map, ref, phaseMask, refX, refY, refZ)
        != expectedTransport)
        return false;

    float const probeZ = std::max(refZ, botZ);
    float const dx2 = botX - refX;
    float const dy2 = botY - refY;
    float const dist2d = std::sqrt(dx2 * dx2 + dy2 * dy2);
    int32 const steps = std::clamp(static_cast<int32>(dist2d / 0.75f), 10, 28);
    float const dx = (botX - refX) / static_cast<float>(steps);
    float const dy = (botY - refY) / static_cast<float>(steps);

    if (map->GetTransportForPos(phaseMask, refX, refY, probeZ, ref) != expectedTransport)
        return false;

    float lastX = refX;
    float lastY = refY;
    bool found = false;

    for (int32 i = 1; i <= steps; ++i)
    {
        float const px = refX + dx * i;
        float const py = refY + dy * i;
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
    outZ = refZ;
    return true;
}

bool MovementAction::BoardTransport(Transport* transport)
{
    if (!transport || transport->IsStaticTransport())
        return false;

    Map* map = bot->GetMap();
    if (!map)
        return false;

    // Already on this transport
    if (bot->GetTransport() == transport)
        return true;

    // Check if bot is already on the transport surface (walked into range).
    float probeZ = std::max(bot->GetPositionZ(), transport->GetPositionZ());
    Transport* surface = GetTransportForPosTolerant(map, bot, bot->GetPhaseMask(), bot->GetPositionX(),
        bot->GetPositionY(), probeZ);

    if (surface == transport)
    {
        transport->AddPassenger(bot, true);
        bot->StopMovingOnCurrentPos();
        EmitDebugMove("Transport:board", "on-surface", transport->GetPositionX(),
                      transport->GetPositionY(), transport->GetPositionZ());
        return true;
    }

    // Bot off transport: find a boarding edge and teleport-snap directly
    // onto it, then AddPassenger. We can't walk on the deck (no transport-
    // surface mmap), so the snap-board is the only universal approach.
    float edgeX, edgeY, edgeZ;
    if (!FindBoardingPointOnTransport(map, transport, transport,
            transport->GetPositionX(), transport->GetPositionY(),
            transport->GetPositionZ(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            edgeX, edgeY, edgeZ))
    {
        // No boarding edge found — wait a tick. Caller's WaitForReach
        // upstream throttles the retry.
        EmitDebugMove("Transport:board", "no-edge",
                      transport->GetPositionX(), transport->GetPositionY(),
                      transport->GetPositionZ());
        return false;
    }

    if (!bot->TeleportTo(map->GetId(), edgeX, edgeY, edgeZ, bot->GetOrientation()))
        return false;

    transport->AddPassenger(bot, true);
    bot->StopMovingOnCurrentPos();
    EmitDebugMove("Transport:board", "snap", edgeX, edgeY, edgeZ);
    return true;
}

bool MovementAction::MoveTo2(WorldPosition endPos,
                             bool idle, bool react,
                             [[maybe_unused]] bool noPath,
                             bool ignoreEnemyTargets,
                             MovementPriority priority,
                             bool lessDelay)
{
    if (!endPos.IsValid())
        return false;

    UpdateMovementState();
    if (!IsMovingAllowed())
        return false;

    // Resume a transport ride if we're still on the same boat as last tick.
    if (WaitForTransport())
        return true;

    WorldPosition botPos(bot);
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

    // Detailed-move throttle: if this bot is in low-activity mode
    // (random/background) and a teleport cooldown is still in effect
    // from a prior dispatch, postpone re-evaluation until the cooldown
    // expires instead of re-resolving the path every tick.
    bool const detailedMove = botAI->AllowActivity(DETAILED_MOVE_ACTIVITY, true);
    if (!detailedMove && lastMove.nextTeleport)
    {
        time_t const now = time(nullptr);
        if (lastMove.nextTeleport > now)
        {
            botAI->SetNextCheckDelay((uint32)((lastMove.nextTeleport - now) * 1000));
            return true;
        }
    }
    else
    {
        lastMove.nextTeleport = 0;
    }

    // Short-stop: at destination — stop and clear the cached path.
    float const totalDistance = botPos.distance(endPos);
    if (totalDistance < sPlayerbotAIConfig.targetPosRecalcDistance)
    {
        if (!lastMove.lastPath.empty() &&
            lastMove.lastPath.getBack().distance(endPos) <= totalDistance)
            lastMove.clear();
        bot->StopMoving();
        return false;
    }

    // Per-tick re-resolve: rebuild the TravelPath from the bot's current
    // position every tick. ResolveMovePath internally gates graph A* by
    // sightDistance — short moves skip the graph and use a raw probe, so
    // funnelling every MoveTo here is cost-bounded for in-zone moves.
    TravelPath path = ResolveMovePath(botPos, endPos, lastMove);
    lastMove.setPath(path);
    if (path.empty())
        return false;

    // Trim leading waypoints behind the bot. Skip on transports — bot's
    // world-space position diverges from path coords mid-ride.
    if (!bot->GetTransport())
        path.makeShortCut(botPos, sPlayerbotAIConfig.reactDistance, bot);
    if (path.empty())
    {
        lastMove.setPath(path);
        return true;
    }

    bool const onTransport = bot->GetTransport() != nullptr;
    if (path.UpcommingSpecialMovement(botPos,
                                      sPlayerbotAIConfig.reactDistance,
                                      onTransport))
    {
        if (HandleSpecialMovement(path))
            return true;
        // Special handler declined (e.g. AREA_TRIGGER with entry → caller
        // dispatches the walk into the trigger volume). Fall through.
    }

    // Transport guard: bot is on a transport but no special movement
    // applies this tick — don't dispatch a walk spline (would fight the
    // transport's own movement).
    if (onTransport)
        return false;

    if (!path.empty())
        lastMove.setPath(path);

    // ClipPath — truncate at first hostile creature in range / non-walkable
    // hop / drifted past reactDistance / > 125 sqDist jump. Combat callers
    // pass ignoreEnemyTargets=true so the chase doesn't stop at an
    // intermediate enemy.
    path.ClipPath(botAI, bot, ignoreEnemyTargets);
    if (path.empty())
        return false;

    // If destination is on land, snap any underwater waypoints to the
    // water surface so the bot swims along the top instead of diving.
    path.surfaceSnapWaypoints(endPos);

    // Telemetry: show the path's actual tail coords vs bot + dest so we
    // can see whether the resolved path is heading toward the right place.
    if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    {
        WorldPosition tail = path.getBack();
        float const tailToDest = tail.distance(endPos);
        float const botToTail = bot->GetExactDist(tail.GetPositionX(),
                                                  tail.GetPositionY(),
                                                  tail.GetPositionZ());
        std::ostringstream tlog;
        tlog << "[PATH] tail=(" << std::fixed << std::setprecision(1)
             << tail.GetPositionX() << "," << tail.GetPositionY() << ","
             << tail.GetPositionZ()
             << ") botToTail=" << botToTail << "y tailToDest=" << tailToDest << "y";
        botAI->TellMasterNoFacing(tlog);
    }

    if (path.empty())
        return false;

    if (!bot->IsMounted() && !bot->IsInCombat() &&
        bot->IsOutdoors() && bot->IsAlive())
        botAI->DoSpecificAction("check mount state", Event(), true);

    bool const dispatched =
        DispatchMovement(path, endPos, "walk", priority, lessDelay, react);

    if (dispatched && !idle)
        ClearIdleState();

    return dispatched;
}

bool MovementAction::DispatchMovement(TravelPath path,
                                      WorldPosition dest,
                                      char const* label,
                                      MovementPriority priority,
                                      bool lessDelay,
                                      bool react)
{
    // Build the PointsArray from the TravelPath. Done here (not at the
    // caller) so DispatchMovement can be invoked with a TravelPath
    // directly, matching the reference's signature.
    std::vector<WorldPosition> const& pts = path.getPointPath();
    Movement::PointsArray points;
    points.reserve(pts.size());
    for (auto const& wp : pts)
        points.emplace_back(wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ());
    if (points.empty())
        return false;

    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
    G3D::Vector3 const& last = points.back();

    float totalDist = 0.f;
    for (size_t i = 1; i < points.size(); ++i)
        totalDist += (points[i] - points[i - 1]).length();

    // Skip cosmetic walking for low-activity bots with no nearby
    // player — teleport to the path tail and schedule a cooldown
    // instead. Matches the reference's MoveTo2 gate
    // (`!detailedMove && !HasPlayerNearby`).
    if (!botAI->AllowActivity(DETAILED_MOVE_ACTIVITY, true))
    {
        WorldPosition tail(dest.GetMapId(), last.x, last.y, last.z);
        time_t now = time(nullptr);
        if (totalDist > sPlayerbotAIConfig.reactDistance &&
            lastMove.nextTeleport <= now &&
            !botAI->HasPlayerNearby(&tail))
        {
            float speed = std::max(bot->GetSpeed(MOVE_RUN), 0.1f);
            lastMove.nextTeleport = now + (time_t)(totalDist / speed);

            EmitDebugMove("MoveFar", "teleport",
                          tail.GetPositionX(), tail.GetPositionY(), tail.GetPositionZ());

            WorldPosition botPos(bot);
            return bot->TeleportTo(dest.GetMapId(),
                                   tail.GetPositionX(), tail.GetPositionY(),
                                   tail.GetPositionZ(),
                                   botPos.getAngleTo(tail));
        }
    }

    // Match master's walk pace when they're walking and within 5y.
    // AC's ForcedMovement enum has no FLIGHT variant — flying is handled
    // via the MovePoint speed/flight flags below, not the moveMode.
    ForcedMovement moveMode = FORCED_MOVEMENT_RUN;
    if (Player* master = botAI->GetMaster())
    {
        if (bot->IsFriendlyTo(master) && master->IsWalking() &&
            bot->GetExactDist2d(master) < 5.0f)
        {
            moveMode = FORCED_MOVEMENT_WALK;
        }
    }

    // Reference: also gates on !IsInWater && !IsUnderWater so a bot
    // wading through shallow water (no SWIMMING movement flag yet)
    // doesn't trigger engine pathfinding mid-dispatch.
    bool const generatePath = !bot->IsFlying() && !bot->isSwimming() &&
                              !bot->IsInWater() && !bot->IsUnderWater();

    // Pre-dispatch normalization: clear looping emote, stand, interrupt
    // non-melee cast. Reference does this at MoveTo2 level before
    // DispatchMovement; we do it here at the equivalent point in the flow.
    bot->ClearEmoteState();
    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);
    if (bot->IsNonMeleeSpellCast(true))
        bot->InterruptNonMeleeSpells(true);

    // Per-point terrain clamp with transport-passenger conversion
    // sandwich: when on a transport, path coords are in transport-local
    // space; UpdateAllowedPositionZ samples world terrain, so we convert
    // local→world, snap, world→local. Without the sandwich, snapping a
    // transport-relative point against world terrain produces garbage.
    Transport* transport = bot->GetTransport();
    for (auto& pt : points)
    {
        if (transport)
            transport->CalculatePassengerPosition(pt.x, pt.y, pt.z);
        bot->UpdateAllowedPositionZ(pt.x, pt.y, pt.z);
        if (transport)
            transport->CalculatePassengerOffset(pt.x, pt.y, pt.z);
    }

    // mm.Clear → MovePoint(last) → MoveSplinePath.
    MotionMaster* mm = bot->GetMotionMaster();
    mm->Clear();

    if (!generatePath || !bot->IsFreeFlying())
    {
        float const flySpeed = bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.0f;
        mm->MovePoint(0, last.x, last.y, last.z, moveMode,
                      flySpeed, 0.0f, generatePath, false);
    }

    if (points.size() >= 2)
        mm->MoveSplinePath(&points, moveMode);

    EmitDebugMove("MoveFar", label, last.x, last.y, last.z);

    // WaitForReach equivalent: cache the dispatched target + duration on
    // lastMove. Leave ~10y headroom on long paths so we re-evaluate
    // before arrival. (Reference also calls WaitForReach here, which
    // blocks the AI loop; we omit that — see header comment.)
    float waitDist = totalDist > sPlayerbotAIConfig.reactDistance
                         ? std::max(totalDist - 10.0f, 0.0f) : totalDist;
    UnitMoveType const speedType = (moveMode == FORCED_MOVEMENT_WALK) ? MOVE_WALK : MOVE_RUN;
    float speed = std::max(bot->GetSpeed(speedType), 0.1f);
    float duration = 1000.0f * (waitDist / speed) + sPlayerbotAIConfig.reactDelay;
    if (lessDelay)
        duration -= sPlayerbotAIConfig.reactDelay;
    duration = std::min(duration, (float)sPlayerbotAIConfig.maxWaitForMove);
    if (duration < 0.0f)
        duration = 0.0f;

    lastMove.Set(bot->GetMapId(), last.x, last.y, last.z,
                 bot->GetOrientation(), (uint32)duration, priority);

    // Reference: DispatchMovement ends with WaitForReach(size) to block
    // the AI loop while the spline plays. Combat callers (react=true)
    // opt out so they can keep re-evaluating mid-chase.
    if (!react)
        WaitForReach(points);

    return true;
}
