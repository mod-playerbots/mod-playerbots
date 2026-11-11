/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Actions.h"
#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using namespace Aq40Helpers;
using namespace EncounterHelpers;

// Shared

bool Aq40MovementAction::MoveToGoal(Position goal, bool hazard)
{
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    bool const twins = encounter == Aq40Encounter::Twins;
    bool const tank = PlayerbotAI::IsTank(bot);
    // C'Thun: the entrance ramps reach the floor only by one slope south-southeast of the Eye, so a spot is often
    // 100+ yd of path away and short steps stall on the ramps.
    bool const cthunRoute = encounter == Aq40Encounter::Cthun && !InStomach(bot);
    if ((!hazard && encounter == Aq40Encounter::Skeram) || twins || cthunRoute)
    {
        if (twins || cthunRoute)
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
        else if (!tank)
            StopAttacking(bot);
        PathGenerator route(bot);
        if (!route.CalculatePath(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()) ||
            route.GetPath().empty())
            return false;
        // A long C'Thun route may come back incomplete; walk its reachable part and path again from there.
        bool partial = cthunRoute && route.GetPathType() == PATHFIND_INCOMPLETE;
        if (route.GetPathType() != PATHFIND_NORMAL && !partial)
            return false;
        auto const& end = route.GetPath().back();
        if (!partial && goal.GetExactDist(end.x, end.y, end.z) > 3.0f)
            return false;
        // Twins transfers run continuously across the room: stopping every few yards adds an AI reaction delay per
        // segment and can miss the next teleport. A hazard escape can still interrupt this route.
        float remaining = twins || cthunRoute ? 250.0f : 4.0f;
        Position next = bot->GetPosition();
        for (auto const& point : route.GetPath())
        {
            float segment = next.GetExactDist(point.x, point.y, point.z);
            if (segment > remaining)
            {
                float fraction = remaining / segment;
                next.Relocate(next.GetPositionX() + (point.x - next.GetPositionX()) * fraction,
                              next.GetPositionY() + (point.y - next.GetPositionY()) * fraction,
                              next.GetPositionZ() + (point.z - next.GetPositionZ()) * fraction);
                break;
            }
            next.Relocate(point.x, point.y, point.z);
            remaining -= segment;
        }
        if (twins && (!tank || hazard))
            StopAttacking(bot);
        if (hazard)
            bot->CastStop();
        if ((twins || cthunRoute) && bot->movespline->Finalized())
            AI_VALUE(LastMovement&, "last movement").clear();  // A knockback may have cancelled the old route.
        bool const moved =
            MoveTo(bot->GetMapId(), next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true,
                   true, hazard ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT, true);
        // An escape still running this route is refused as a duplicate. It keeps the bot's turn, or a lower action
        // re-faces the bot and stops the run.
        return moved || (hazard && !bot->movespline->Finalized());
    }
    bool const stomachExit = hazard && encounter == Aq40Encounter::Cthun && InStomach(bot);
    float distance = bot->GetExactDist2d(goal);
    if (distance < 1.5f)
    {
        if (stomachExit)
        {
            StopMoving(botAI);  // Stay on the exit while the server's delayed area-trigger checks run.
            Aq40BotCache& cache = BotCache(botAI);
            uint32 const now = getMSTime();
            if ((!cache.exitAttemptTime || getMSTimeDiff(cache.exitAttemptTime, now) > 4 * IN_MILLISECONDS) &&
                std::abs(bot->GetPositionZ() - goal.GetPositionZ()) < 4.0f)
            {
                cache.exitAttemptTime = now;
                WorldPacket packet(CMSG_AREATRIGGER);
                packet << uint32(CTHUN_STOMACH_EXIT_TRIGGER);
                bot->GetSession()->HandleAreaTriggerOpcode(packet);
            }
        }
        return false;
    }
    float angle = bot->GetAngle(&goal);
    float length = std::min(distance, hazard ? 7.0f : 4.0f);
    // A hazard escape can be walled in (ramp edge, cloud cluster). Turn before giving up, and only interrupt the
    // bot's cast once a step is actually possible, so a stuck healer keeps healing.
    std::vector<float> turns =
        hazard ? std::vector<float>{0.0f, 0.9f, -0.9f, 1.8f, -1.8f, AQ40_PI} : std::vector<float>{0.0f};
    std::vector<Creature*> clouds = hazard && encounter == Aq40Encounter::Viscidus
                                        ? GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME))
                                        : std::vector<Creature*>{};
    bool found = false;
    float bestX = 0.0f;
    float bestY = 0.0f;
    float bestZ = 0.0f;
    float bestClearance = -1.0f;
    for (float turn : turns)
    {
        float x = bot->GetPositionX() + length * std::cos(angle + turn);
        float y = bot->GetPositionY() + length * std::sin(angle + turn);
        float z = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;
        PathGenerator path(bot);
        if (!path.CalculatePath(x, y, z) || path.GetPathType() != PATHFIND_NORMAL)
            continue;
        float pathLength = 0.0f;
        Position previous = bot->GetPosition();
        for (auto const& point : path.GetPath())
        {
            Position step(point.x, point.y, point.z);
            pathLength += previous.GetExactDist(step);
            previous = step;
        }
        if (pathLength > (hazard ? 12.0f : 8.0f))
            continue;
        float clearance = std::numeric_limits<float>::max();
        for (Creature* cloud : clouds)
            clearance = std::min(clearance, cloud->GetExactDist2d(x, y));
        if (found && clearance <= bestClearance)
            continue;
        found = true;
        bestX = x;
        bestY = y;
        bestZ = z;
        bestClearance = clearance;
        if (clouds.empty())
            break;
    }
    if (!found && stomachExit)
    {
        // Players drop from the stomach entrance into the pool, but a bot hangs where it was teleported and no short
        // step validates from mid-air. Go straight to the exit.
        bot->CastStop();
        bot->GetMotionMaster()->MovePoint(0, goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ(),
                                          FORCED_MOVEMENT_NONE, 0.0f, 0.0f, false, false);
        return true;
    }
    if (!found)
        return false;
    if (hazard)
    {
        StopAttacking(bot);
        bot->CastStop();
    }
    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true,
                  hazard ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40ResetEncounterStatesAction::Execute(Event /*event*/)
{
    BotCache(botAI) = Aq40BotCache();
    if (IsMechanicTrackerBot(bot, AQ40_MAP_ID))
        Aq40ResetInstance(bot->GetInstanceId());
    return false;
}

bool Aq40DropToGroundAction::Execute(Event /*event*/)
{
    bot->GetMotionMaster()->MoveFall();
    return true;
}

bool Aq40MovementAction::MoveToPrepullSpot(Position const& spot)
{
    if (bot->GetExactDist2d(spot) > 2.0f)
    {
        if (!bot->isMoving() || bot->movespline->Finalized())
            bot->GetMotionMaster()->MovePoint(0, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
        return true;
    }
    // Hold the spot: returning true keeps "follow" from walking the bot back to the master.
    if (bot->isMoving())
        StopMoving(botAI);
    return true;
}

bool Aq40AttackAssignedTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetAssignedTarget(botAI);
    return target && Attack(target);
}

bool Aq40StopAttackingAction::Execute(Event /*event*/)
{
    bool attacking = bot->GetVictim() || (bot->GetPet() && bot->GetPet()->GetVictim());
    StopAttacking(bot);
    return attacking;
}

bool Aq40TankTauntAssignedTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetAssignedTarget(botAI);
    std::string const taunt = TauntSpell(bot);
    return target && !taunt.empty() && TryCast(botAI, taunt, target);
}

bool Aq40TrackEncounterAction::Execute(Event /*event*/)
{
    TrackEncounter(bot);
    return false;
}

bool Aq40AssignAddTanksAction::Execute(Event /*event*/)
{
    AssignAddTanks(bot);
    return false;
}

bool Aq40RangedMoveIntoBossRangeAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetBossRangeGoal(botAI, spot))
        return false;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, spot, 0.0f, nullptr, moveX, moveY, backwards))
        return false;
    return MoveTo(AQ40_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// The Prophet Skeram

bool TheProphetSkeramCrowdControlMindControlledAction::Execute(Event /*event*/)
{
    Player* member = GetMindControlledMember(bot);
    if (!member)
        return false;
    for (char const* name : {"polymorph", "hammer of justice", "cyclone"})
        if (TryCast(botAI, name, member))
            return true;
    return false;
}

bool TheProphetSkeramAssignPlatformsAction::Execute(Event /*event*/)
{
    AssignSkeramPlatforms(bot);
    return false;
}

bool TheProphetSkeramMoveToAssignedPlatformAction::Execute(Event /*event*/)
{
    Position goal;
    return GetSkeramPlatformGoal(botAI, goal) && MoveToGoal(goal, false);
}

// Bug Trio

bool BugTrioMoveAwayFromPoisonCloudAction::Execute(Event /*event*/)
{
    Position goal;
    return GetPoisonCloudEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool BugTrioTanksSeparateBossesAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetTrioTankGoal(botAI, spot))
        return false;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, spot, 0.0f, GetAssignedTarget(botAI), moveX, moveY, backwards))
        return false;
    return MoveTo(AQ40_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Battleguard Sartura

bool BattleguardSarturaMoveAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Creature* whirler = GetWhirlwindInRange(bot);
    if (!whirler)
        return false;
    return MoveToGoal(Around(bot, WHIRLWIND_ESCAPE_STEP, whirler->GetAngle(bot)), true);
}

// Fankriss the Unyielding

bool FankrissTheUnyieldingSwapTanksAction::Execute(Event /*event*/)
{
    SwapFankrissTanks(bot);
    return false;
}

// Viscidus

bool ViscidusCastFrostSpellsAction::Execute(Event /*event*/)
{
    Creature* boss = GetBoss(bot);
    if (!boss)
        return false;
    // Rank-one Frostbolt maximizes hit frequency; use the normal spell engine and costs.
    uint32 const frostbolt = Id(Aq40Spells::SPELL_FROSTBOLT_RANK_1);
    if (bot->HasSpell(frostbolt) && bot->GetDistance(boss) < 28.0f && bot->IsWithinLOSInMap(boss))
    {
        if (bot->isMoving())
            StopMoving(botAI);
        if (botAI->CanCastSpell(frostbolt, boss) && botAI->CastSpell(frostbolt, boss))
            return true;
    }
    for (char const* name : {"frost shock", "icy touch", "ice lance"})
        if (TryCast(botAI, name, boss))
            return true;
    return false;
}

bool ViscidusMeleeShatterAction::Execute(Event /*event*/)
{
    Unit* target = GetAssignedTarget(botAI);
    if (!target)
        return false;
    bool starting = bot->GetVictim() != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);
    Attack(target);
    return starting && bot->Attack(target, true);
}

bool ViscidusEscapeToxinCloudsAction::Execute(Event /*event*/)
{
    Position goal;
    return GetToxinEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool ViscidusMeleeMoveInToShatterAction::Execute(Event /*event*/)
{
    Creature* boss = GetBoss(bot);
    Position spot;
    if (!boss || !GetViscidusShatterGoal(botAI, spot))
        return false;
    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, boss, boss->GetCombatReach(), PATH_STEP_DISTANCE, stepX, stepY))
        return false;
    return MoveTo(AQ40_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool ViscidusPositionRangedOnRingAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetViscidusRingGoal(botAI, spot))
        return false;
    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(bot, spot, 0.0f, PATH_STEP_DISTANCE, stepX, stepY))
        return false;
    return MoveTo(AQ40_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Princess Huhuran

bool PrincessHuhuranTranquilizingShotAction::Execute(Event /*event*/)
{
    Creature* boss = GetBoss(bot);
    return boss && TryCast(botAI, "tranquilizing shot", boss);
}

bool PrincessHuhuranCureWyvernStingAction::Execute(Event /*event*/)
{
    // Dispel damage is exactly 3,000 in this server's aura script. Protect low-health sleepers.
    Player* tank = GetWyvernStungTank(bot);
    if (!tank)
        return false;
    for (char const* name : {"cleanse", "abolish poison", "cure poison", "cleanse spirit"})
        if (TryCast(botAI, name, tank))
            return true;
    return false;
}

bool PrincessHuhuranAssignPoisonBoltSoakersAction::Execute(Event /*event*/)
{
    AssignHuhuranSoakers(bot);
    return false;
}

bool PrincessHuhuranSwapTanksAction::Execute(Event /*event*/)
{
    SwapHuhuranTanks(bot);
    return false;
}

bool PrincessHuhuranSoakersStandNearBossAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetHuhuranSoakGoal(botAI, spot))
        return false;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, spot, 0.0f, nullptr, moveX, moveY, backwards))
        return false;
    return MoveTo(AQ40_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool PrincessHuhuranSpreadRangedAndHealersAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetHuhuranSpreadGoal(botAI, spot))
        return false;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, spot, 0.0f, nullptr, moveX, moveY, backwards))
        return false;
    return MoveTo(AQ40_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Twin Emperors

bool TwinEmperorsDropEmperorThreatAction::Execute(Event /*event*/)
{
    // Healing threat reaches both emperors: at the pull a healer can take Vek'lor while the waiter's Taunt is on
    // cooldown and walk him into the casters. Anyone not meant to hold an emperor sheds it with a threat drop.
    char const* drop = bot->getClass() == CLASS_PRIEST   ? "fade"
                       : bot->getClass() == CLASS_HUNTER ? "feign death"
                       : bot->getClass() == CLASS_ROGUE  ? "vanish"
                                                         : nullptr;
    return drop && TryCast(botAI, drop, bot);
}

bool EmperorVeknilashTankUseDefensiveCooldownAction::Execute(Event /*event*/)
{
    // Whoever Vek'nilash is hitting spends Shield Wall, then Last Stand, as soon as each is ready.
    for (char const* cooldown : {"shield wall", "last stand"})
        if (!botAI->HasAura(cooldown, bot) && TryCast(botAI, cooldown, bot))
            return true;
    return false;
}

bool TwinEmperorsHealEmperorTargetsAction::Execute(Event /*event*/)
{
    Player* focus = GetTwinHealFocus(bot);
    if (!focus)
        return false;
    std::vector<char const*> heals;
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            if (!botAI->HasAura("weakened soul", focus))
                heals.push_back("power word: shield");
            heals.insert(heals.end(), {"flash heal", "greater heal", "heal"});
            break;
        case CLASS_DRUID:
            if (!botAI->HasAura("rejuvenation", focus, false, true))
                heals.push_back("rejuvenation");
            heals.insert(heals.end(), {"regrowth", "healing touch"});
            break;
        case CLASS_SHAMAN:
            heals.insert(heals.end(), {"lesser healing wave", "healing wave"});
            break;
        case CLASS_PALADIN:
            heals.insert(heals.end(), {focus->GetHealthPct() < 60.0f ? "holy light" : "flash of light", "holy light"});
            break;
        default:
            break;
    }
    for (char const* heal : heals)
        if (TryCast(botAI, heal, focus))
            return true;
    return false;
}

bool TwinEmperorsWarlockSetPetPassiveAction::Execute(Event /*event*/)
{
    // A demon near Vek'lor becomes Vek'nilash's nearest target after a swap, and even the imp's Firebolt threat can
    // take either emperor. Demons stay out of this fight.
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;
    pet->AttackStop();
    pet->SetReactState(REACT_PASSIVE);
    pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    return false;
}

bool TwinEmperorsWarlockPrepareForVeklorAction::Execute(Event /*event*/)
{
    if (bot->GetPowerPct(POWER_MANA) < 85.0f && bot->GetHealthPct() > 60.0f && TryCast(botAI, "life tap", bot))
        return true;
    return TwinSwapWindow(bot) && !botAI->HasAura("shadow ward", bot) && TryCast(botAI, "shadow ward", bot);
}

bool EmperorVeklorWaitingTankTauntBossAction::Execute(Event /*event*/)
{
    std::string const taunt = TauntSpell(bot);
    if (taunt.empty())
        return false;
    for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
        if (Player* lock = GetTankFor(bot, veklor); lock && lock != bot && IsTwinLock(bot, lock))
            if (Unit* victim = veklor->GetVictim(); victim && victim != lock && victim != bot)
                if (TryCast(botAI, taunt, veklor))
                    return true;
    return false;
}

bool EmperorVeklorWarlockTankCastSearingPainAction::Execute(Event /*event*/)
{
    Unit* target = GetAssignedTarget(botAI);
    if (!target)
        return false;
    if (bot->GetPowerPct(POWER_MANA) < 10.0f && bot->GetHealthPct() > 60.0f && TryCast(botAI, "life tap", bot))
        return true;
    return TryCast(botAI, "searing pain", target);
}

bool EmperorVeklorWarlockVoidwalkerSacrificeAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;
    for (uint32 sacrifice : {47986u, 47985u, 27273u, 19443u, 19442u, 19441u, 19440u, 19438u, 7812u})
        if (pet->HasSpell(sacrifice))
        {
            if (!pet->HasSpellCooldown(sacrifice))
                pet->CastSpell(pet, sacrifice, false);
            break;
        }
    return false;
}

bool EmperorVeklorTankDemoralizingShoutAction::Execute(Event /*event*/)
{
    std::string const shout = bot->getClass() == CLASS_WARRIOR ? "demoralizing shout" : "demoralizing roar";
    return botAI->CanCastSpell(shout, bot) && botAI->CastSpell(shout, bot);
}

bool TwinEmperorsLeadEmperorToPlatformAction::Execute(Event /*event*/)
{
    Creature* emperor = GetEmperorThatCannotReach(bot);
    if (!emperor)
        return false;
    // His target finishes any fall, then leads him back onto the nearest platform top: standing next to him is not
    // enough when his own path is the problem.
    float floor = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (floor > INVALID_HEIGHT && bot->GetPositionZ() - floor > 0.5f && !bot->IsFlying())
    {
        bot->GetMotionMaster()->MoveFall();
        return true;
    }
    Position spot = TwinUnstuckSpot(bot, emperor);
    bot->UpdateAllowedPositionZ(spot.GetPositionX(), spot.GetPositionY(), spot.m_positionZ);
    bot->GetMotionMaster()->MovePoint(0, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
    return true;
}

bool TwinEmperorsWarlockMoveToStartSpotAction::Execute(Event /*event*/)
{
    Position spot;
    return GetTwinPrepullSpot(botAI, spot) && MoveToPrepullSpot(spot);
}

bool TwinEmperorsAssignTanksAndSidesAction::Execute(Event /*event*/)
{
    AssignTwinRoles(bot);
    return false;
}

bool TwinEmperorsMoveAwayFromExplodingBugAction::Execute(Event /*event*/)
{
    Position goal;
    return GetExplodingBugEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool TwinEmperorsEscapeBlizzardAction::Execute(Event /*event*/)
{
    Position goal;
    return GetBlizzardEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool TwinEmperorsMoveAwayFromVeklorAction::Execute(Event /*event*/)
{
    Position goal;
    return GetVeklorEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool TwinEmperorsClearVeknilashLandingSpotAction::Execute(Event /*event*/)
{
    Position goal;
    return GetVeknilashLandingEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool TwinEmperorsTanksPositionEmperorsAction::Execute(Event /*event*/)
{
    Position goal;
    return GetTwinTankGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool EmperorVeklorWarlockTankKeepRangeAction::Execute(Event /*event*/)
{
    Position goal;
    return GetTwinWarlockGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool TwinEmperorsMeleeLeaveLandingSpotsAction::Execute(Event /*event*/)
{
    Position goal;
    return GetTwinMeleeSwapGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool TwinEmperorsDpsMoveToAssignedEmperorAction::Execute(Event /*event*/)
{
    Position goal;
    return GetTwinDpsGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool TwinEmperorsHealersPositionBetweenTankAndWarlockAction::Execute(Event /*event*/)
{
    Position goal;
    return GetTwinHealerGoal(botAI, goal) && MoveToGoal(goal, false);
}

// Ouro

bool OuroMoveOutOfFrontalConeAction::Execute(Event /*event*/)
{
    Position goal;
    return GetOuroConeEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool OuroAvoidDirtMoundAction::Execute(Event /*event*/)
{
    Position goal;
    return GetDirtMoundEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool OuroAcceptNewMainTankAction::Execute(Event /*event*/)
{
    AcceptOuroTank(bot);
    return false;
}

// C'Thun

bool CthunHealersDamageCthunAction::Execute(Event /*event*/)
{
    Creature* body = GetCthunBody(bot);
    if (!body)
        return false;
    for (char const* nuke : {"mind blast", "smite", "starfire", "wrath", "lightning bolt", "holy shock"})
        if (TryCast(botAI, nuke, body))
            return true;
    return false;
}

bool CthunHealersDamageFleshTentaclesAction::Execute(Event /*event*/)
{
    Creature* nearest = nullptr;
    for (Creature* flesh : GetUnits(bot, Id(Aq40Npcs::NPC_FLESH_TENTACLE)))
        if (IsAllowedDamage(bot, flesh) && (!nearest || bot->GetDistance(flesh) < bot->GetDistance(nearest)))
            nearest = flesh;
    if (!nearest)
        return false;
    // Tree of Life blocks Wrath, Moonfire and Starfire (NOT_SHAPESHIFT).
    if (bot->HasAura(Id(Aq40Spells::SPELL_TREE_OF_LIFE)))
        bot->RemoveAurasDueToSpell(Id(Aq40Spells::SPELL_TREE_OF_LIFE));
    for (char const* nuke :
         {"mind blast", "holy fire", "smite", "shadow word: pain", "wrath", "moonfire", "starfire", "lightning bolt",
          "flame shock", "holy shock", "exorcism", "judgement of light", "judgement of wisdom"})
        if (TryCast(botAI, nuke, nearest))
            return true;
    return false;
}

bool CthunSendSpitOutTriggerAction::Execute(Event /*event*/)
{
    // Send area trigger 4036 for the bot, as the stomach exit already does.
    BotCache(botAI).spitAttemptTime = getMSTime();
    WorldPacket packet(CMSG_AREATRIGGER);
    packet << uint32(CTHUN_SPIT_OUT_TRIGGER);
    bot->GetSession()->HandleAreaTriggerOpcode(packet);
    return true;
}

bool CthunPullerMoveToPullSpotAction::Execute(Event /*event*/)
{
    Position spot;
    return GetCthunPullerPrepullSpot(botAI, spot) && MoveToPrepullSpot(spot);
}

bool CthunTrailPullerAction::Execute(Event /*event*/)
{
    Position spot;
    return GetCthunTrailSpot(botAI, spot) && MoveToPrepullSpot(spot);
}

bool EyeOfCthunTrackDarkGlareAction::Execute(Event /*event*/)
{
    TrackDarkGlare(bot);
    return false;
}

bool CthunAssignSpotsAction::Execute(Event /*event*/)
{
    AssignCthunSpots(bot);
    return false;
}

bool CthunAssignTentacleKillersAction::Execute(Event /*event*/)
{
    AssignCthunTentacleKillers(bot);
    return false;
}

bool CthunAssignGiantClawTanksAction::Execute(Event /*event*/)
{
    AssignCthunClawTanks(bot);
    return false;
}

bool CthunExitStomachAction::Execute(Event /*event*/)
{
    Position goal;
    return GetStomachExitGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool EyeOfCthunDodgeDarkGlareAction::Execute(Event /*event*/)
{
    Position goal;
    return GetDarkGlareEscapeGoal(botAI, goal) && MoveToGoal(goal, true);
}

bool EyeOfCthunPullerHoldPullSpotAction::Execute(Event /*event*/)
{
    Position goal;
    return GetCthunPullerHoldGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool EyeOfCthunMoveToAssignedSpotAction::Execute(Event /*event*/)
{
    Position goal;
    return GetCthunSpotGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool CthunMoveToStackAction::Execute(Event /*event*/)
{
    Position goal;
    return GetCthunStackGoal(botAI, goal) && MoveToGoal(goal, false);
}

bool CthunMeleeMoveIntoBurnRangeAction::Execute(Event /*event*/)
{
    Position goal;
    return GetCthunMeleeBurnGoal(botAI, goal) && MoveToGoal(goal, false);
}
