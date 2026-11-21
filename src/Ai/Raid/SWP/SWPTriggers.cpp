/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPTriggers.h"
#include "EncounterHelpers.h"
#include "Pet.h"
#include "Playerbots.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "ThreatHelpers.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

// Shared

bool SunwellNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SWP_MAP_ID);
}

bool SunwellClassRolesShouldBeAssignedTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID);
}

bool SunwellAuraToRemoveTrigger::IsActive()
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (spellId && bot->getClass() != CLASS_ROGUE && !PlayerbotAI::IsHeal(bot) &&
        bot->HasAura(spellId))
    {
        return true;
    }

    if (IsEncounterInProgress(bot, SWP_MAP_ID))
        return false;

    return HasBrutallusBurn(bot);
}

// Trash

bool VolatileFiendSelfDestructsWhenNearTrigger::IsActive()
{
    Unit* fiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    return fiend && fiend->IsAlive();
}

bool ApocalypseGuardProtectedByInfernalDefenseTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_PRIEST)
        return false;

    return GetInfernalDefenseGuard(botAI);
}

// Shared Bosses

bool SunwellHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetEncounterBoss();
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Kalecgos

bool KalecgosShouldCommunicateBossHealthTrigger::IsActiveInEncounter()
{
    return GetDueKalecgosCall(bot) != KalecgosCall::None;
}

bool KalecgosRequiresTankRotationTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = GetEncounterBoss();
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    return !IsInSpectralRealm(bot);
}

bool KalecgosSpectralRiftIsOpenTrigger::IsActiveInEncounter()
{
    Unit* kalecgos = GetEncounterBoss();
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    if (!ShouldEnterKalecgosPortal(bot))
        return false;

    return botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift"));
}

bool KalecgosRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kalecgos = GetEncounterBoss();
    if (!kalecgos || kalecgos->IsFriendlyTo(bot) || kalecgos->GetVictim() == bot)
        return false;

    return !ShouldEnterKalecgosPortal(bot);
}

bool KalecgosTooManyArcaneBuffetStacksTrigger::IsActiveInEncounter()
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_ROGUE && botClass != CLASS_MAGE && botClass != CLASS_PALADIN)
        return false;

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = GetEncounterBoss();
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    if (IsInSpectralRealm(bot))
        return false;

    Aura const* arcaneBuffet = bot->GetAura(Id(SwpSpells::SPELL_ARCANE_BUFFET));
    return arcaneBuffet && arcaneBuffet->GetStackAmount() >= 10;
}

bool KalecgosHumanoidKalecTanksSathrovarrTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && IsInSpectralRealm(bot);
}

bool KalecgosBotsDontObserveGravityTrigger::IsActiveInEncounter()
{
    if (!IsInSpectralRealm(bot))
        return false;

    constexpr float verticalOffset = 5.0f;
    return bot->GetPositionZ() > SPECTRAL_REALM_Z + verticalOffset ||
        bot->GetPositionZ() < SPECTRAL_REALM_Z - verticalOffset;
}

bool KalecgosShouldManageRaidAssignmentsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID);
}

bool KalecgosDpsHoldIsActiveTrigger::IsActiveInEncounter()
{
    return GetKalecgosDpsHold(bot) != KalecgosDpsHold::None;
}

// Only once the health calls have started, so a pet passive by config doesn't run the action all
// fight.
bool KalecgosShouldManagePetAggressionTrigger::IsActiveInEncounter()
{
    if (!HasControllableMinion(bot))
        return false;

    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    if (!state || (state->healthCall == KalecgosHealthCall::None && !state->endHoldCalled))
        return false;

    return IsMinionStanceWrongForHold(
        bot, GetKalecgosDpsHoldWindow(bot) != KalecgosDpsHold::None);
}

// Brutallus

bool BrutallusRequiresTwoTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusMeleeShouldStandInPlaceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* brutallus = GetEncounterBoss();
    if (!brutallus || brutallus->GetVictim() == bot)
        return false;

    return !PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusRangedShouldSoakMeteorSlashTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (HasBrutallusBurn(bot))
        return false;

    Unit* brutallus = GetEncounterBoss();
    return brutallus && brutallus->GetVictim() != bot;
}

bool BrutallusBurnOnNonTankTrigger::IsActiveInEncounter()
{
    if (!HasBrutallusBurn(bot))
        return false;

    return !PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

// Felmyst

bool FelmystHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    if (felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return true;

    if (felmyst->IsFlying())
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    return mainTank && felmyst->GetVictim() != mainTank;
}

bool FelmystGroundPhaseShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        if (FelmystEncounterState* state = FindFelmystState(bot->GetInstanceId()))
            state->encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    return !HasFelmystEarlyMoveStarted(botAI);
}

bool FelmystRangedShouldPositionToDispelAndFleeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        if (FelmystEncounterState* state = FindFelmystState(bot->GetInstanceId()))
            state->encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    return !HasFelmystEarlyMoveStarted(botAI) && IsFelmystRangedStackActive(bot, felmyst);
}

bool FelmystMeleeShouldStayTogetherTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        if (FelmystEncounterState* state = FindFelmystState(bot->GetInstanceId()))
            state->encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    if (HasFelmystEarlyMoveStarted(botAI))
        return false;

    if (felmyst->GetVictim() == bot)
        return false;

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool FelmystEncapsulateOnMageOrPaladinTrigger::IsActiveInEncounter()
{
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    if (!isPaladin && bot->getClass() != CLASS_MAGE)
        return false;

    if (!bot->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
        return false;

    if (isPaladin)
        return !PlayerbotAI::IsMainTank(bot);

    return true;
}

bool FelmystNearEncapsulatedPlayerTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || felmyst->IsFlying())
        return false;

    return ShouldMoveAwayFromFelmystEncapsulateTarget(
        bot, felmyst, GetFelmystEncapsulateTarget(bot));
}

bool FelmystPlayerHasGasNovaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || felmyst->IsFlying())
        return false;

    return GetFelmystGasNovaDispelTarget(bot);
}

bool FelmystTakeoffIsNearTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    AdvanceFelmystEncounterState(felmyst);

    // Once started, the early move stays on until takeoff, or it would switch itself off as the
    // bot nears G0 and the ground action would take the bot back out.
    if (HasEarlyMoveStarted())
        return true;

    FelmystEncounterState const* state = FindFelmystState(bot->GetInstanceId());
    if (!state || !IsFelmystEarlyMoveDue(bot, felmyst))
        return false;

    _earlyMoveTouchdownMs = state->touchdownMs;
    return true;
}

// Takeoff clears the touchdown time, which ends the mark.
bool FelmystTakeoffIsNearTrigger::HasEarlyMoveStarted() const
{
    FelmystEncounterState const* state = FindFelmystState(bot->GetInstanceId());
    return _earlyMoveTouchdownMs && state && state->touchdownMs == _earlyMoveTouchdownMs;
}

bool FelmystTakeoffIsNearTrigger::ResetEarlyMove()
{
    if (!_earlyMoveTouchdownMs)
        return false;

    _earlyMoveTouchdownMs = 0;
    return true;
}

bool HasFelmystEarlyMoveStarted(PlayerbotAI* botAI)
{
    Trigger* trigger = botAI->GetAiObjectContext()->GetTrigger("felmyst takeoff is near");
    return trigger && static_cast<FelmystTakeoffIsNearTrigger*>(trigger)->HasEarlyMoveStarted();
}

bool FelmystAirPhaseHasStartedTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    AdvanceFelmystEncounterState(felmyst);
    return IsFelmystAirGroupMoveActive(bot, felmyst);
}

bool FelmystShouldAvoidDemonicVaporTrailsTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    AdvanceFelmystEncounterState(felmyst);
    if (!felmyst->IsFlying() || GetFelmystOwnDemonicVapor(bot))
        return false;

    FogOfCorruptionState fogState;
    return !TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);
}

bool FelmystTargetedByDemonicVaporTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    return felmyst && felmyst->IsFlying() && GetFelmystOwnDemonicVapor(bot);
}

bool FelmystFogOfCorruptionIsActiveTrigger::IsActiveInEncounter()
{
    Unit* felmyst = GetEncounterBoss();
    if (!felmyst)
        return false;

    AdvanceFelmystEncounterState(felmyst);
    if (!felmyst->IsFlying())
        return false;

    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(bot, felmyst, fogState))
        return true;

    FogLane thirdPassLane = FogLane::None;
    return TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);
}

bool FelmystMeleeCannotReachFlyingBossTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || AI_VALUE(Unit*, "current target") != felmyst)
        return false;

    return IsFelmystAirPhaseTargetSuppressed(felmyst);
}

bool FelmystPlayerIsCharmedByFogTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    Unit* felmyst = GetEncounterBoss();
    return felmyst && GetFelmystCharmedTarget(bot, felmyst);
}

bool FelmystShouldHoldDpsWhileLandingTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID);
}

bool FelmystLandingDpsHoldIsActiveTrigger::IsActiveInEncounter()
{
    return IsFelmystLandingDpsHoldActive(botAI);
}

// Eredar Twins

// Hunters misdirect Sacrolash all through phase 1 and nothing in phase 2.
bool EredarTwinsHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER && FindSacrolash(bot);
}

bool EredarTwinsShouldLeaveBalconyTrigger::IsActiveInEncounter()
{
    if (bot->GetPositionZ() <= EREDAR_TWINS_BALCONY_Z)
        return false;

    if (PlayerbotAI::IsMelee(bot))
        return true;

    Unit* sacrolash = FindSacrolash(bot);
    return !sacrolash || sacrolash->GetVictim() == bot;
}

bool EredarTwinsSacrolashShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!FindSacrolash(bot))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return IsAnySacrolashTank(bot);
}

bool EredarTwinsAlythessShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return IsAlythessTank(bot);
}

bool EredarTwinsRangedShouldStackOnBalconyTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash || sacrolash->GetVictim() == bot)
        return false;

    return !IsEredarTwinsConflagrationTarget(bot, FindAlythess(bot), sacrolash);
}

bool EredarTwinsOnlyAlythessRemainsTrigger::IsActiveInEncounter()
{
    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    if (FindSacrolash(bot))
        return false;

    if (IsEredarTwinsConflagrationTarget(bot, FindAlythess(bot), nullptr))
        return false;

    return !IsAlythessTank(bot);
}

bool EredarTwinsAlythessTargetsNonTankTrigger::IsActiveInEncounter()
{
    return IsHoldingAlythessAggro(bot, FindAlythess(bot));
}

bool EredarTwinsTooManyFlameTouchedStacksTrigger::IsActiveInEncounter()
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_ROGUE && botClass != CLASS_MAGE && botClass != CLASS_PALADIN)
        return false;

    if (PlayerbotAI::IsTank(bot))
        return false;

    Aura const* flameSear = bot->GetAura(Id(SwpSpells::SPELL_FLAME_SEAR));
    if (!flameSear || flameSear->GetDuration() > FLAME_SEAR_PROTECT_WINDOW_MS)
        return false;

    Aura const* flameTouched = bot->GetAura(Id(SwpSpells::SPELL_FLAME_TOUCHED));
    return flameTouched && flameTouched->GetStackAmount() >= FLAME_TOUCHED_PROTECT_STACKS;
}

bool EredarTwinsShouldFocusDpsTrigger::IsActiveInEncounter()
{
    // Healers are included to flip to their combat engines since dps assist is disabled. The
    // Sacrolash tanks join once she is dead.
    return !PlayerbotAI::IsTank(bot) || (!IsAlythessTank(bot) && !FindSacrolash(bot));
}

bool EredarTwinsShouldManagePetAggressionTrigger::IsActiveInEncounter()
{
    if (!HasControllableMinion(bot))
        return false;

    return IsMinionStanceWrongForHold(bot, IsEredarTwinsStartHoldActive(botAI));
}

bool EredarTwinsActiveConflagrationTargetTrigger::IsActiveInEncounter()
{
    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash)
        return false;

    return IsEredarTwinsConflagrationTarget(bot, FindAlythess(bot), sacrolash);
}

bool EredarTwinsSacrolashVictimHasConflagrationTrigger::IsActiveInEncounter()
{
    Unit* sacrolash = FindSacrolash(bot);
    if (!sacrolash)
        return false;

    ObjectGuid const conflagTargetGuid =
        GetEredarTwinsConflagrationTargetGuid(FindAlythess(bot), sacrolash);
    if (conflagTargetGuid.IsEmpty() || conflagTargetGuid == bot->GetGUID())
        return false;

    Unit* victim = sacrolash->GetVictim();
    return victim && victim->GetGUID() == conflagTargetGuid;
}

// M'uru

bool MuruHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* voidSentinel = FindThreatTarget(bot, "void sentinel");
    if (voidSentinel && voidSentinel->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
        return true;

    Unit* entropius = FindThreatTarget(bot, "entropius");
    return entropius && entropius->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT;
}

bool MuruEntropiusShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) && FindThreatTarget(bot, "entropius");
}

bool MuruRangedShouldStackOrSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

bool MuruShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot);
}

bool MuruVoidSentinelShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (FindThreatTarget(bot, "void sentinel"))
        return true;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* muru = GetEncounterBoss();
    return IsMuruPhaseActive(muru);
}

bool MuruEntranceShouldBeGuardedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* muru = GetEncounterBoss();
    if (!IsMuruPhaseActive(muru))
        return false;

    Unit* voidSentinel = FindThreatTarget(bot, "void sentinel");
    if (voidSentinel && voidSentinel->GetVictim() == bot)
        return false;

    return !FindThreatTarget(bot, "shadowsword fury mage") &&
        !FindThreatTarget(bot, "shadowsword berserker");
}

bool MuruDarkFiendsSpawnedTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    return GetNearestMuruDarkFiend(botAI, RANGED_ABILITY_REACH);
}

bool MuruDarknessIsActiveTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* muru = GetEncounterBoss();
    return IsMuruPhaseActive(muru) && IsMuruDarknessActive(muru);
}

bool MuruBerserkerHasFlurryTrigger::IsActiveInEncounter()
{
    // These classes have no stuns and can't be Tauren.
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_PRIEST)
        return false;

    return FindMuruBerserkerToStun(botAI);
}

bool MuruFuryMageCastingFelFireballTrigger::IsActiveInEncounter()
{
    // Only tank Druids have interrupts, which we don't want to waste here. Paladins have no
    // interrupt other than Hammer of Justice, which should be saved as a stun for Berserkers.
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN)
        return false;

    return FindMuruFuryMageToInterrupt(botAI);
}

bool MuruFuryMageHasSpellFuryTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    return FindMuruFuryMageToSpellsteal(botAI);
}

bool MuruVoidSpawnAvailableForEnslaveTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (bot->GetCharm())
        return false;

    return FindAvailableVoidSpawnForEnslave(botAI);
}

bool MuruWarlockHasEnslavedVoidSpawnTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* charm = bot->GetCharm();
    return charm && charm->IsAlive() && charm->GetEntry() == Id(SwpNpcs::NPC_VOID_SPAWN);
}

// Phase 2 only: void zones are Entropius's, and phase 1 fiends are left to the dispellers.
bool MuruVoidZoneIsNearTrigger::IsActiveInEncounter()
{
    return !IsMuruPhaseActive(GetEncounterBoss()) && FindMuruVoidZoneToAvoid(botAI);
}

bool MuruDarkFiendIsNearTrigger::IsActiveInEncounter()
{
    return !IsMuruPhaseActive(GetEncounterBoss()) && FindMuruDarkFiendToAvoid(botAI);
}

bool MuruTheSingularityIsNearTrigger::IsActiveInEncounter()
{
    Unit* entropius = FindThreatTarget(bot, "entropius");
    if (!entropius)
        return false;

    Creature* singularity = botAI->GetCreature(AI_VALUE(ObjectGuid, "muru singularity"));
    return singularity && singularity->IsAlive() &&
        bot->GetExactDist(singularity) < GetSingularityClearance(bot, entropius);
}

// Kil'jaeden <The Deceiver>

bool KiljaedenHandsOfTheDeceiverAreActiveTrigger::IsActiveInEncounter()
{
    if (bot->GetExactDist2d(SUNWELL_CENTER_POSITION) > HAND_SEARCH_RADIUS)
        return false;

    return FindThreatTarget(bot, "hand of the deceiver");
}

// The tracker starts the hold's clock once Kil'jaeden is attackable (he engages the raid then).
bool KiljaedenShouldHoldDpsAtStartTrigger::IsActiveInEncounter()
{
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (state && state->dpsHoldStartMs)
        return false;

    return GetEncounterBoss() && IsMechanicTrackerBot(bot, SWP_MAP_ID);
}

bool KiljaedenDpsHoldIsActiveTrigger::IsActiveInEncounter()
{
    return IsKiljaedenStartDpsHoldActive(botAI);
}

bool KiljaedenShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return !IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return !IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenRangedShouldKeepFormationTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    // Allow Demo Locks to AoE the Reflections they can reach without entering the Knockback
    if (bot->getClass() != CLASS_WARLOCK || !bot->HasAura(Id(SwpSpells::SPELL_METAMORPHOSIS)))
        return true;

    for (ObjectGuid const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == Id(SwpNpcs::NPC_SINISTER_REFLECTION) &&
            !IsMeleeBlockedByKiljaedenKnockback(*bot, *unit))
        {
            return false;
        }
    }

    return true;
}

bool KiljaedenFireBloomOnImmunityClassTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(SwpSpells::SPELL_FIRE_BLOOM)) &&
        CanClearKiljaedenFireBloomWithImmunity(bot, GetEncounterBoss());
}

bool KiljaedenCastsDarknessOfAThousandSoulsTrigger::IsActiveInEncounter()
{
    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenDragonOrbIsActiveTrigger::IsActiveInEncounter()
{
    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden || kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD)
        return false;

    if (GetKiljaedenDragonOrbUser(bot) != bot || HasKiljaedenDragonAura(bot))
        return false;

    bool orbInUse = false;
    bool result = false;

    for (auto const& orbGuid : context->GetValue<GuidVector>("kiljaeden dragon orbs")->RefGet())
    {
        GameObject* orb = botAI->GetGameObject(orbGuid);
        if (!orb)
            continue;

        bool const inUse = orb->HasGameObjectFlag(GO_FLAG_IN_USE);

        if (inUse)
            orbInUse = true;

        if (!orb->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
            result = true;
    }

    if (orbInUse)
        result = true;

    return result;
}

bool KiljaedenBotControlsDragonTrigger::IsActiveInEncounter()
{
    if (!GetEncounterBoss())
        return false;

    if (!HasKiljaedenDragonAura(bot))
        return false;

    return GetKiljaedenControlledDragon(bot);
}

bool KiljaedenStaleRootAfterDragonTrigger::IsActiveInEncounter()
{
    // Shield of the Blue stuns the drake it is cast from. The drake dies on the last tick of its
    // second Shield, before the stun expires, and the possession ends before the stun is removed,
    // so the unroot is applied to the drake and the orb user bot is left with the root flag set.
    if (!HasUsedKiljaedenDragonOrb(bot) || HasKiljaedenDragonAura(bot))
        return false;

    return HasStaleRootFlag(bot);
}
