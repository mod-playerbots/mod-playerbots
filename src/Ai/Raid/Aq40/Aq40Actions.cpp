/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Actions.h"
#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "MoveSpline.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "Timer.h"
#include <cmath>
#include <vector>

using namespace Aq40Helpers;
using namespace EncounterHelpers;

// Shared

bool Aq40ResetEncounterStatesAction::Execute(Event /*event*/)
{
    BotCache(botAI) = Aq40BotCache();
    if (IsMechanicTrackerBot(bot, AQ40_MAP_ID))
    {
        Aq40ResetInstance(bot->GetInstanceId());
        ClearStrategyTargetIcon(bot, SKULL_ICON, {Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS), Id(Aq40Npcs::NPC_FANKRISS)});
    }
    return false;
}

bool Aq40DropToGroundAction::Execute(Event /*event*/)
{
    bot->GetMotionMaster()->MoveFall();
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
    Position spot;
    if (!GetSkeramPlatformGoal(botAI, spot))
        return false;
    if (!PlayerbotAI::IsTank(bot))
        StopAttacking(bot);
    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(bot, spot, 0.0f, PATH_STEP_DISTANCE, stepX, stepY))
        return false;
    return MoveTo(AQ40_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Bug Trio

bool BugTrioMoveAwayFromPoisonCloudAction::Execute(Event /*event*/)
{
    Position spot;
    Position step;
    if (!GetPoisonCloudEscapeGoal(botAI, spot) || !GetEscapeStep(bot, spot, {}, step))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
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
    Position step;
    if (!whirler || !GetEscapeStep(bot, Around(bot, WHIRLWIND_ESCAPE_STEP, whirler->GetAngle(bot)), {}, step))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
}

// Fankriss the Unyielding

bool FankrissTheUnyieldingSwapTanksAction::Execute(Event /*event*/)
{
    SwapFankrissTanks(bot);
    return false;
}

bool FankrissTheUnyieldingMarkKillTargetAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    // A living skull off Fankriss stays until it dies: the raid finishes one worm before the next, and a player's
    // skull on any other unit wins.
    if (Creature* marked = bot->GetMap()->GetCreature(group->GetTargetIcon(SKULL_ICON));
        marked && marked->IsAlive() && marked->GetEntry() != Id(Aq40Npcs::NPC_FANKRISS))
        return false;
    return MarkTargetWithSkull(bot, FindFankrissKillTarget(bot));
}

bool FankrissTheUnyieldingMainTankAttackBossAction::Execute(Event /*event*/)
{
    Creature* boss = GetBoss(bot);
    if (!boss)
        return false;
    if (AI_VALUE(Unit*, "current target") != boss || bot->GetVictim() != boss)
        return Attack(boss);
    std::string const taunt = TauntSpell(bot);
    return !taunt.empty() && TryCast(botAI, taunt, boss);
}

bool FankrissTheUnyieldingTanksPickUpSpawnsAction::Execute(Event /*event*/)
{
    Creature* spawn = FindFankrissOwnedSpawn(bot);
    if (!spawn)
        return false;
    if (AI_VALUE(Unit*, "current target") != spawn || bot->GetVictim() != spawn)
        return Attack(spawn);
    std::string const taunt = TauntSpell(bot);
    return !taunt.empty() && TryCast(botAI, taunt, spawn);
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
    Position spot;
    Position step;
    if (!GetToxinEscapeGoal(botAI, spot) ||
        !GetEscapeStep(bot, spot, GetUnits(bot, Id(Aq40Npcs::NPC_TOXIC_SLIME)), step))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
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
    if (!GetTwinPrepullSpot(botAI, spot))
        return false;
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

bool TwinEmperorsAssignTanksAndSidesAction::Execute(Event /*event*/)
{
    AssignTwinRoles(bot);
    return false;
}

bool TwinEmperorsMoveAwayFromExplodingBugAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetExplodingBugEscapeGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_FORCED, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops lower actions from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool TwinEmperorsEscapeBlizzardAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetBlizzardEscapeGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_FORCED, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops lower actions from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool TwinEmperorsMoveAwayFromVeklorAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetVeklorEscapeGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_FORCED, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops lower actions from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool TwinEmperorsClearVeknilashLandingSpotAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetVeknilashLandingEscapeGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_FORCED, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops lower actions from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool TwinEmperorsTanksPositionEmperorsAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetTwinTankGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool EmperorVeklorWarlockTankKeepRangeAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetTwinWarlockGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_COMBAT, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops attack set-up from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool TwinEmperorsMeleeLeaveLandingSpotsAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetTwinMeleeSwapGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    bool const tank = PlayerbotAI::IsTank(bot);
    if (!tank)
        StopAttacking(bot);
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_COMBAT, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops attack set-up from
    // re-facing the bot and breaking up the run.
    return moved || (!tank && !bot->movespline->Finalized());
}

bool TwinEmperorsDpsMoveToAssignedEmperorAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetTwinDpsGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    bool const tank = PlayerbotAI::IsTank(bot);
    if (!tank)
        StopAttacking(bot);
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_COMBAT, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops attack set-up from
    // re-facing the bot and breaking up the run.
    return moved || (!tank && !bot->movespline->Finalized());
}

bool TwinEmperorsHealersPositionBetweenTankAndWarlockAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetTwinHealerGoal(botAI, spot) || !GetRouteStep(bot, spot, false, next))
        return false;
    StopAttacking(bot);
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

// Ouro

bool OuroMoveOutOfFrontalConeAction::Execute(Event /*event*/)
{
    Position spot;
    Position step;
    if (!GetOuroConeEscapeGoal(botAI, spot) || !GetEscapeStep(bot, spot, {}, step))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
}

bool OuroAvoidDirtMoundAction::Execute(Event /*event*/)
{
    Position spot;
    Position step;
    if (!GetDirtMoundEscapeGoal(botAI, spot) || !GetEscapeStep(bot, spot, {}, step))
        return false;
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
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
    if (!GetCthunPullerPrepullSpot(botAI, spot))
        return false;
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

bool CthunTrailPullerAction::Execute(Event /*event*/)
{
    Position spot;
    if (!GetCthunTrailSpot(botAI, spot))
        return false;
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
    Position exit;
    if (!GetStomachExitGoal(botAI, exit))
        return false;
    if (bot->GetExactDist2d(exit) < 1.5f)
    {
        StopMoving(botAI);  // Stay on the exit while the server's delayed area-trigger checks run.
        Aq40BotCache& cache = BotCache(botAI);
        uint32 const now = getMSTime();
        if ((!cache.exitAttemptTime || getMSTimeDiff(cache.exitAttemptTime, now) > 4 * IN_MILLISECONDS) &&
            std::abs(bot->GetPositionZ() - exit.GetPositionZ()) < 4.0f)
        {
            cache.exitAttemptTime = now;
            WorldPacket packet(CMSG_AREATRIGGER);
            packet << uint32(CTHUN_STOMACH_EXIT_TRIGGER);
            bot->GetSession()->HandleAreaTriggerOpcode(packet);
        }
        return false;
    }
    Position step;
    if (!GetEscapeStep(bot, exit, {}, step))
    {
        // Players drop from the stomach entrance into the pool, but a bot hangs where it was teleported and no short
        // step validates from mid-air. Go straight to the exit.
        bot->CastStop();
        bot->GetMotionMaster()->MovePoint(0, exit.GetPositionX(), exit.GetPositionY(), exit.GetPositionZ(),
                                          FORCED_MOVEMENT_NONE, 0.0f, 0.0f, false, false);
        return true;
    }
    StopAttacking(bot);
    bot->CastStop();
    return MoveTo(AQ40_MAP_ID, step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
}

bool EyeOfCthunDodgeDarkGlareAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetDarkGlareEscapeGoal(botAI, spot) || !GetRouteStep(bot, spot, true, next))
        return false;
    bot->CastStop();
    ClearStaleLastMove(botAI);
    bool const moved = MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                              true, true, MovementPriority::MOVEMENT_FORCED, true);
    // A repeat of a route still running is refused as a duplicate. Keeping the turn stops lower actions from
    // re-facing the bot and breaking up the run.
    return moved || !bot->movespline->Finalized();
}

bool EyeOfCthunPullerHoldPullSpotAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetCthunPullerHoldGoal(botAI, spot) || !GetRouteStep(bot, spot, true, next))
        return false;
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool EyeOfCthunMoveToAssignedSpotAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetCthunSpotGoal(botAI, spot) || !GetRouteStep(bot, spot, true, next))
        return false;
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool CthunMoveToStackAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetCthunStackGoal(botAI, spot) || !GetRouteStep(bot, spot, true, next))
        return false;
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool CthunMeleeMoveIntoBurnRangeAction::Execute(Event /*event*/)
{
    Position spot;
    Position next;
    if (!GetCthunMeleeBurnGoal(botAI, spot) || !GetRouteStep(bot, spot, true, next))
        return false;
    ClearStaleLastMove(botAI);
    return MoveTo(AQ40_MAP_ID, next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, true, true,
                  MovementPriority::MOVEMENT_COMBAT, true);
}
