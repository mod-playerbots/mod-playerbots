/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Triggers.h"
#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "Timer.h"

using namespace Aq40Helpers;
using namespace EncounterHelpers;

// Shared

bool Aq40EncounterTrigger::IsFreeToAct() { return !bot->HasUnitState(UNIT_STATE_CASTING); }

bool Aq40NoEncounterInProgressTrigger::IsActive()
{
    return bot->GetMapId() == AQ40_MAP_ID && !IsEncounterInProgress(bot, AQ40_MAP_ID);
}

bool Aq40TrackerTrigger::IsActiveInEncounter()
{
    return (_encounter == Aq40Encounter::None || IsInEncounter(bot, _encounter)) && IsActingTracker(bot);
}

bool Aq40StuckMidairTrigger::IsActiveInEncounter()
{
    // Server bots have no client to resume gravity after a knockback. C'Thun knocks back often and the stomach exit
    // drops players 10 yd up. Not in the stomach, where a fall sinks bots below the exit.
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    if (encounter != Aq40Encounter::Twins && (encounter != Aq40Encounter::Cthun || InStomach(bot)))
        return false;
    if (IsSelfBot(bot) || !botAI->CanMove() || bot->IsFlying() || bot->HasAuraType(SPELL_AURA_HOVER))
        return false;
    float floor = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    // C'Thun: only a short drop (a knockback, the 10-yd stomach drop onto C'Thun). On the entrance slope the height
    // lookup finds ground far below the walkway and a fall drops bots through the world.
    float drop = bot->GetPositionZ() - floor;
    return floor > INVALID_HEIGHT && drop > 3.0f && (encounter == Aq40Encounter::Twins || drop < 15.0f);
}

bool Aq40AssignedTargetTrigger::IsActiveInEncounter()
{
    if (!IsInEncounter(bot, _encounter) || PlayerbotAI::IsHeal(bot) || !IsFreeToAct())
        return false;
    Unit* target = GetAssignedTarget(botAI);
    if (!target)
        return false;
    Position spot;
    // A transfer owns the bot until it reaches its C'Thun spot: attack setup re-faces the target every tick and
    // breaks up the walk the position action just started.
    if (_encounter == Aq40Encounter::Cthun && GetCthunPositionGoal(botAI, spot))
        return false;
    // Weakened burn: melee hit C'Thun only from within 10 yd of his center; until then they walk in.
    if (CthunMeleeBurn(botAI) && bot->GetExactDist2d(target) > CTHUN_BURN_MELEE_REACH)
        return false;
    return AI_VALUE(Unit*, "current target") != target || bot->GetVictim() != target;
}

bool Aq40NoAssignedTargetTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot) || !IsFreeToAct() || GetAssignedTarget(botAI))
        return false;
    return bot->GetVictim() || (bot->GetPet() && bot->GetPet()->GetVictim());
}

bool Aq40TankShouldTauntTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot) || TauntSpell(bot).empty())
        return false;
    Unit* target = GetAssignedTarget(botAI);
    return target && GetTankFor(bot, target) == bot && target->GetVictim() != bot && !Whirling(target);
}

bool Aq40RangedShouldHoldBossRangeTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetBossRangeGoal(botAI, goal);
}

// The Prophet Skeram

bool TheProphetSkeramRaidMemberMindControlledTrigger::IsActiveInEncounter()
{
    return IsFreeToAct() && GetMindControlledMember(bot);
}

bool TheProphetSkeramShouldMoveToPlatformTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetSkeramPlatformGoal(botAI, goal);
}

// Bug Trio

bool BugTrioPoisonCloudNearbyTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetPoisonCloudEscapeGoal(botAI, goal);
}

bool BugTrioBossesTooCloseTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTrioTankGoal(botAI, goal);
}

// Battleguard Sartura

bool BattleguardSarturaWhirlwindNearbyTrigger::IsActiveInEncounter() { return GetWhirlwindInRange(bot) != nullptr; }

// Viscidus

bool ViscidusShouldBeFrozenTrigger::IsActiveInEncounter()
{
    return ViscidusShouldBeFrozen(bot) && IsFreeToAct();
}

bool ViscidusIsFrozenTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot) || !IsFreeToAct())
        return false;
    Unit* target = GetAssignedTarget(botAI);
    return target && target->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) && bot->IsWithinMeleeRange(target) &&
           (bot->GetVictim() != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING));
}

bool ViscidusToxinCloudNearbyTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetToxinEscapeGoal(botAI, goal);
}

bool ViscidusFrozenOutOfMeleeRangeTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetViscidusShatterGoal(botAI, goal);
}

bool ViscidusRangedShouldHoldRingTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetViscidusRingGoal(botAI, goal);
}

// Princess Huhuran

bool PrincessHuhuranFrenziedTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER || !IsFreeToAct())
        return false;
    Creature* boss = GetBoss(bot);
    return boss && boss->HasAura(Id(Aq40Spells::SPELL_FRENZY));
}

bool PrincessHuhuranWyvernStingOnTankTrigger::IsActiveInEncounter()
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_PALADIN && botClass != CLASS_DRUID && botClass != CLASS_SHAMAN)
        return false;
    return IsFreeToAct() && GetWyvernStungTank(bot);
}

bool PrincessHuhuranSoakerTooFarTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetHuhuranSoakGoal(botAI, goal);
}

bool PrincessHuhuranRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetHuhuranSpreadGoal(botAI, goal);
}

// Twin Emperors

bool TwinEmperorsNonTankHasEmperorAggroTrigger::IsActiveInEncounter()
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_PRIEST && botClass != CLASS_HUNTER && botClass != CLASS_ROGUE)
        return false;
    if (PlayerbotAI::IsTank(bot) || IsTwinLock(bot, bot) || !IsFreeToAct())
        return false;
    for (Creature* emperor : GetUnits(bot))
        if (IsTwinEmperor(emperor) && emperor->GetVictim() == bot)
            return true;
    return false;
}

bool EmperorVeknilashHittingTankTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot) || !IsFreeToAct())
        return false;
    // Unbalancing Strike plus a swing can take a tank from full to dead within 2 seconds, so waiting for 35% is too
    // late. One cooldown at a time, so the two cover separate windows.
    bool covered = botAI->HasAura("shield wall", bot) || botAI->HasAura("last stand", bot);
    return (IsHoldingVeknilash(bot) && !covered) || bot->GetHealthPct() < 35.0f;
}

bool TwinEmperorsHealerShouldFocusEmperorTargetsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsHeal(bot) && IsFreeToAct() && GetTwinHealFocus(bot);
}

bool TwinEmperorsWarlockPetActiveTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK || !IsFreeToAct())
        return false;
    Pet* pet = bot->GetPet();
    return pet && pet->GetReactState() != REACT_PASSIVE;
}

bool TwinEmperorsWarlockWaitingTrigger::IsActiveInEncounter()
{
    if (!IsTwinLock(bot, bot) || !IsFreeToAct() || GetAssignedTarget(botAI))
        return false;
    // Between his phases a side's warlock refills mana with Life Tap (never while tanking: a phase costs ~4000 mana)
    // and puts up Shadow Ward before the teleport window.
    bool lifeTap = bot->GetPowerPct(POWER_MANA) < 85.0f && bot->GetHealthPct() > 60.0f;
    bool shadowWard = TwinSwapWindow(bot) && !botAI->HasAura("shadow ward", bot);
    return lifeTap || shadowWard;
}

bool EmperorVeklorHeldByWrongPlayerTrigger::IsActiveInEncounter()
{
    if (!IsTwinWaiter(bot, bot) || !IsFreeToAct() || TauntSpell(bot).empty())
        return false;
    // The warrior waiting beside a warlock-held Vek'lor peels him off anyone else (the puller at the opening, a
    // healer later). His taunt copies only their small threat, so Searing Pain retakes him.
    for (Creature* veklor : GetUnits(bot, Id(Aq40Npcs::NPC_VEKLOR)))
        if (Player* lock = GetTankFor(bot, veklor); lock && lock != bot && IsTwinLock(bot, lock))
            if (Unit* victim = veklor->GetVictim(); victim && victim != lock && victim != bot)
                return true;
    return false;
}

bool EmperorVeklorShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    if (!IsTwinLock(bot, bot) || !IsFreeToAct())
        return false;
    Unit* target = GetAssignedTarget(botAI);
    return target && target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR);
}

bool EmperorVeklorWarlockTankLowHealthTrigger::IsActiveInEncounter()
{
    // Vek'lor's Shadow Bolt hits a warlock for ~3-4k. Below half health, a Voidwalker shields him with Sacrifice even
    // though it stays passive for the rest of the fight.
    if (!IsTwinLock(bot, bot) || bot->GetHealthPct() >= 50.0f)
        return false;
    Unit* target = GetAssignedTarget(botAI);
    Pet* pet = bot->GetPet();
    return target && target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && pet && pet->IsAlive() &&
           pet->GetEntry() == Id(Aq40Npcs::NPC_VOIDWALKER) && !pet->HasUnitState(UNIT_STATE_CASTING);
}

bool EmperorVeklorTankInShoutRangeTrigger::IsActiveInEncounter()
{
    if ((bot->getClass() != CLASS_WARRIOR && bot->getClass() != CLASS_DRUID) || !IsFreeToAct())
        return false;
    // Vek'lor's tank waits outside melee, so Taunt alone only ties the puller's threat and he drifts back once it
    // fades. The shout reaches him from the waiting spot and builds a margin.
    Unit* target = GetAssignedTarget(botAI);
    return target && target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && GetTankFor(bot, target) == bot &&
           bot->GetDistance(target) <= 10.0f;
}

bool TwinEmperorsEmperorCannotReachTargetTrigger::IsActiveInEncounter()
{
    // An emperor that can't path to his target takes no damage and regenerates a third of his health every tick
    // (Creature::RegenerateHealth).
    return botAI->CanMove() && GetEmperorThatCannotReach(bot);
}

bool TwinEmperorsWarlockShouldTakeStartSpotTrigger::IsActive()
{
    Position spot;
    return GetTwinPrepullSpot(botAI, spot);
}

bool TwinEmperorsExplodingBugNearbyTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetExplodingBugEscapeGoal(botAI, goal);
}

bool TwinEmperorsInBlizzardTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetBlizzardEscapeGoal(botAI, goal);
}

bool EmperorVeklorArcaneBurstRangeTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetVeklorEscapeGoal(botAI, goal);
}

bool EmperorVeknilashChoosingTargetAfterTeleportTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetVeknilashLandingEscapeGoal(botAI, goal);
}

bool TwinEmperorsTanksShouldHoldPlatformsTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTwinTankGoal(botAI, goal);
}

bool EmperorVeklorWarlockTankOutOfRangeTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTwinWarlockGoal(botAI, goal);
}

bool TwinEmperorsTeleportImminentTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTwinMeleeSwapGoal(botAI, goal);
}

bool TwinEmperorsDpsShouldPositionTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTwinDpsGoal(botAI, goal);
}

bool TwinEmperorsHealersShouldPositionTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetTwinHealerGoal(botAI, goal);
}

// Ouro

bool OuroFrontalConeTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetOuroConeEscapeGoal(botAI, goal);
}

bool OuroDirtMoundNearbyTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetDirtMoundEscapeGoal(botAI, goal);
}

// C'Thun

bool CthunMeleeOutOfBurnRangeTrigger::IsActiveInEncounter()
{
    if (!CthunMeleeBurn(botAI) || !bot->GetVictim())
        return false;
    Unit* target = GetAssignedTarget(botAI);
    return target && bot->GetExactDist2d(target) > CTHUN_BURN_MELEE_REACH;
}

bool CthunHealersShouldDamageCthunTrigger::IsActiveInEncounter()
{
    // While C'Thun is Weakened there is no healing to be done, so healers damage him too.
    if (!PlayerbotAI::IsHeal(bot) || InStomach(bot) || !IsFreeToAct())
        return false;
    Creature* body = GetCthunBody(bot);
    return body && IsCthunWeakened(bot) && IsAllowedDamage(bot, body);
}

bool CthunHealersShouldDamageFleshTentaclesTrigger::IsActiveInEncounter()
{
    // A swallowed healer damages the Flesh Tentacles: its heals can't reach the raid ~170 yd above. Heals on anyone
    // inside still rank above this.
    if (!PlayerbotAI::IsHeal(bot) || !InStomach(bot) || StomachExitNeeded(bot) || !IsFreeToAct())
        return false;
    for (Creature* flesh : GetUnits(bot, Id(Aq40Npcs::NPC_FLESH_TENTACLE)))
        if (IsAllowedDamage(bot, flesh))
            return true;
    return false;
}

bool CthunOnSpitOutCircleTrigger::IsActiveInEncounter()
{
    // A player on C'Thun's center is spat out by area trigger 4036, which only a client fires.
    if (IsSelfBot(bot) || InStomach(bot) || !botAI->CanMove() ||
        bot->GetExactDist2d(&CTHUN_CENTER) >= CTHUN_SPIT_OUT_RADIUS)
        return false;
    Aq40BotCache const& cache = BotCache(botAI);
    return !cache.spitAttemptTime || getMSTimeDiff(cache.spitAttemptTime, getMSTime()) > 3 * IN_MILLISECONDS;
}

bool CthunPullerShouldWalkInTrigger::IsActive()
{
    Position spot;
    return GetCthunPullerPrepullSpot(botAI, spot);
}

bool CthunRaidShouldTrailPullerTrigger::IsActive()
{
    Position spot;
    return GetCthunTrailSpot(botAI, spot);
}

bool CthunShouldLeaveStomachTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetStomachExitGoal(botAI, goal);
}

bool EyeOfCthunDarkGlareIncomingTrigger::IsActiveInEncounter()
{
    Position goal;
    return GetDarkGlareEscapeGoal(botAI, goal);
}

bool EyeOfCthunPullerHoldingTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetCthunPullerHoldGoal(botAI, goal);
}

bool EyeOfCthunOutOfAssignedSpotTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetCthunSpotGoal(botAI, goal);
}

bool CthunOutOfStackTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetCthunStackGoal(botAI, goal);
}

bool CthunMeleeShouldCloseInForBurnTrigger::IsActiveInEncounter()
{
    Position goal;
    return IsFreeToAct() && GetCthunMeleeBurnGoal(botAI, goal);
}
