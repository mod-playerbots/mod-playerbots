/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPTriggers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include <cmath>

using namespace SwpHelpers;
using namespace EncounterHelpers;

// General

bool SunwellPlateauNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != SWP_MAP_ID)
        return false;

    // InstanceScript reports IN_PROGRESS for every SWP boss from JustEngagedWith until kill/evade,
    // except for Kil'jaeden, which does not commence until the first Hand dies.
    if (IsEncounterInProgress(bot, SWP_MAP_ID))
        return false;

    // Use a distance gate to avoid searching for Hands through the entire instance.
    if (bot->GetExactDist2d(SUNWELL_CENTER_POSITION) > SUNWELL_CENTER_RADIUS)
        return true;

    return AI_VALUE(GuidVector, "kiljaeden hands").empty();
}

bool SunwellPlateauBotHasAuraToRemoveTrigger::IsActive()
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
    if (!fiend || !fiend->IsAlive())
        return false;

    // Z-position comparison is so bots will go up the ramp to M'uru without getting stuck
    // due to proximity to the volatile fiends below, in case the player decides to skip them.
    constexpr float verticalOffset = 10.0f;
    return std::abs(bot->GetPositionZ() - fiend->GetPositionZ()) < verticalOffset;
}

bool ApocalypseGuardProtectedByInfernalDefenseTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "apocalypse guard");
}

// Kalecgos

bool KalecgosShouldCommunicateBossHealthTrigger::IsActiveInEncounter()
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->GetHealthPct() >= 20.0f)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* spectralBot = nullptr;
    Player* surfaceBot = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (!spectralBot && IsInSpectralRealm(member))
            spectralBot = member;

        if (!surfaceBot && !IsInSpectralRealm(member))
            surfaceBot = member;

        if (spectralBot && surfaceBot)
            break;
    }

    return bot == spectralBot || bot == surfaceBot;
}

bool KalecgosPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    return kalecgos && kalecgos->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool KalecgosRequiresTankRotationTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    return !IsInSpectralRealm(bot);
}

bool KalecgosSpectralRiftIsOpenTrigger::IsActiveInEncounter()
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    if (!ShouldEnterKalecgosPortal(bot))
        return false;

    return botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift"));
}

bool KalecgosBotsTakeSplashDamageTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot) || kalecgos->GetVictim() == bot)
        return false;

    return !ShouldEnterKalecgosPortal(bot);
}

bool KalecgosTooManyArcaneBuffetStacksTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    if (IsInSpectralRealm(bot))
        return false;

    Aura* arcaneBuffet = bot->GetAura(Id(SwpSpells::SPELL_ARCANE_BUFFET));
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

// Brutallus

bool BrutallusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool BrutallusRequiresTwoTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusMeleeShouldStandInPlaceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
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

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetVictim() != bot;
}

bool BrutallusBotIsBurningTrigger::IsActiveInEncounter()
{
    if (!HasBrutallusBurn(bot))
        return false;

    return !PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

// Felmyst

bool FelmystPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
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

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        auto const stateItr = felmystEncounterStates.find(bot->GetInstanceId());
        if (stateItr != felmystEncounterStates.end())
            stateItr->second.encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    return true;
}

bool FelmystRangedShouldPositionToDispelAndFleeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        auto const stateItr = felmystEncounterStates.find(bot->GetInstanceId());
        if (stateItr != felmystEncounterStates.end())
            stateItr->second.encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    if (felmyst->GetVictim() == bot)
        return false;

    // On initial landing, let the MT get aggro before assuming positions
    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && felmyst->GetVictim() != mainTank &&
        felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
    {
        return false;
    }

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool FelmystMeleeShouldStayTogetherTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (felmyst->IsFlying())
    {
        auto const stateItr = felmystEncounterStates.find(bot->GetInstanceId());
        if (stateItr != felmystEncounterStates.end())
            stateItr->second.encapsulateOccurredThisGroundPhase = false;

        return false;
    }

    if (felmyst->GetVictim() == bot)
        return false;

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool FelmystBotIsEncapsulatedTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    if (!bot->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool FelmystBotNearEncapsulatedPlayerTrigger::IsActiveInEncounter()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    FelmystGroundStack const botStack = GetClosestFelmystGroundStack(bot, felmyst, bot);
    FelmystGroundStack const targetStack = GetClosestFelmystGroundStack(
        bot, felmyst, encapsulateTarget);

    return botStack != FelmystGroundStack::None && botStack == targetStack;
}

bool FelmystPlayerHasGasNovaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return GetFelmystGasNovaDispelTarget(bot);
}

bool FelmystShouldAvoidDemonicVaporTrailsTrigger::IsActiveInEncounter()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    if (GetFelmystDemonicVaporSummonedByBot(bot))
        return false;

    FogOfCorruptionState fogState;
    return !TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);
}

bool FelmystBotIsDemonicVaporTargetTrigger::IsActiveInEncounter()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(bot, felmyst, fogState))
        return false;

    return IsFelmystDemonicVaporHeadNearBot(bot);
}

bool FelmystFogOfCorruptionIsActiveTrigger::IsActiveInEncounter()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
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

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return false;

    return IsFelmystAirPhaseTargetSuppressed(felmyst);
}

bool FelmystPlayerIsCharmedByFogTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    return GetFelmystCharmedTarget(bot, felmyst);
}

bool FelmystShouldHoldDpsWhileLandingTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID) && AI_VALUE2(Unit*, "find target", "felmyst");
}

// Eredar Twins

bool EredarTwinsMeleeIsAtBalconyTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    return bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z;
}

bool EredarTwinsShouldAnnounceAlythessTankTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return false;

    auto const itr = eredarTwinsTankAssignments.find(bot->GetInstanceId());
    if (itr != eredarTwinsTankAssignments.end() && itr->second.announcementMs)
        return false;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") ||
        AI_VALUE2(Unit*, "find target", "lady sacrolash");
}

bool EredarTwinsPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    return alythess && alythess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool EredarTwinsSacrolashRequiresTwoTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return IsAnySacrolashTank(bot);
}

bool EredarTwinsAlythessCastsBlazeOnTankTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return IsAlythessTank(bot);
}

bool EredarTwinsRangedNeedsLosTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    return GetEredarTwinsConflagrationTarget(bot) != bot;
}

bool EredarTwinsOnlyAlythessRemainsTrigger::IsActiveInEncounter()
{
    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess") ||
        AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        return false;
    }

    if (GetEredarTwinsConflagrationTarget(bot) == bot)
        return false;

    return !IsAlythessTank(bot);
}

bool EredarTwinsTooManyFlameTouchedStacksTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Aura* flameSear = bot->GetAura(Id(SwpSpells::SPELL_FLAME_SEAR));
    if (!flameSear || flameSear->GetDuration() > FLAME_SEAR_PROTECT_WINDOW_MS)
        return false;

    Aura* flameTouched = bot->GetAura(Id(SwpSpells::SPELL_FLAME_TOUCHED));
    return flameTouched && flameTouched->GetStackAmount() >= FLAME_TOUCHED_PROTECT_STACKS;
}

bool EredarTwinsShouldFocusDpsTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    if (PlayerbotAI::IsDps(bot) || PlayerbotAI::IsHeal(bot))
        return true;

    return !IsAnySacrolashTank(bot) && !IsAlythessTank(bot);
}

bool EredarTwinsActiveConflagrationTargetTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return false;

    if (bot->getClass() == CLASS_ROGUE && botAI->HasAura("vanish", bot))
        return false;

    return GetEredarTwinsConflagrationTarget(bot) == bot;
}

bool EredarTwinsSacrolashVictimHasConflagrationTrigger::IsActiveInEncounter()
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    Player* conflagTarget = GetEredarTwinsConflagrationTarget(bot);
    if (!conflagTarget || conflagTarget == bot)
        return false;

    return sacrolash->GetVictim() == conflagTarget;
}

// M'uru

bool MuruVoidSentinelOrEntropiusHasAppearedTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (voidSentinel && voidSentinel->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
        return true;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    return entropius && entropius->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT;
}

bool MuruBossTransformedIntoEntropiusTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "entropius");
}

bool MuruRangedShouldStackOrSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "m'uru");
}

bool MuruDeterminingDpsPriorityTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "m'uru");
}

bool MuruVoidSentinelPulsesShadowTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (AI_VALUE2(Unit*, "find target", "void sentinel"))
        return true;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    return IsMuruPhaseActive(muru);
}

bool MuruAddsSpawnAtEntranceTrigger::IsActiveInEncounter()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!IsMuruPhaseActive(muru))
        return false;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (voidSentinel && voidSentinel->GetVictim() == bot)
        return false;

    return !AI_VALUE2(Unit*, "find target", "shadowsword fury mage") &&
        !AI_VALUE2(Unit*, "find target", "shadowsword berserker");
}

bool MuruDarkFiendsSpawnedTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    return bot->FindNearestCreature(Id(SwpNpcs::NPC_DARK_FIEND), DARK_FIEND_DISPEL_SEARCH_RADIUS);
}

bool MuruDarknessIsComingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!IsMuruPhaseActive(muru))
        return false;

    return TryGetMuruDarknessActiveState(bot, muru);
}

bool MuruBerserkerIsBuffedWithFlurryTrigger::IsActiveInEncounter()
{
    // No stuns and can't be a Tauren. Too bad.
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_PRIEST ||
        bot->getClass() == CLASS_WARLOCK)
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    return FindMuruBerserkerToStun(botAI);
}

bool MuruFuryMageCastingFelFireballTrigger::IsActiveInEncounter()
{
    // Do Druids have no interrupts...?
    if (bot->getClass() == CLASS_DRUID)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    return FindMuruFuryMageToInterrupt(botAI);
}

bool MuruFuryMageIsBuffedWithSpellFuryTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    return FindMuruFuryMageToSpellsteal(botAI);
}

bool MuruVoidSpawnAvailableForEnslaveTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    if (bot->GetCharm())
        return false;

    return FindAvailableVoidSpawnForEnslave(botAI);
}

bool MuruWarlockHasEnslavedVoidSpawnTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return false;

    Unit* charm = bot->GetCharm();
    return charm && charm->IsAlive() && charm->GetEntry() == Id(SwpNpcs::NPC_VOID_SPAWN);
}

bool MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "entropius"))
        return false;

    if (FindMuruVoidZoneToAvoid(botAI))
        return true;

    return bot->FindNearestCreature(Id(SwpNpcs::NPC_DARK_FIEND), DARK_FIEND_AVOID_SEARCH_RADIUS);
}

bool MuruTheSingularityIsNearTrigger::IsActiveInEncounter()
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = botAI->GetCreature(AI_VALUE(ObjectGuid, "muru singularity"));
    return singularity && singularity->IsAlive();
}

// Kil'jaeden <The Deceiver>

bool KiljaedenShouldCoordinateOrbUseTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return false;

    auto const stateItr = kiljaedenEncounterStates.find(bot->GetInstanceId());
    if (stateItr != kiljaedenEncounterStates.end() && stateItr->second.dragonOrbAnnouncementMs)
        return false;

    return AI_VALUE2(Unit*, "find target", "hand of the deceiver");
}

bool KiljaedenHandsOfTheDeceiverAreActiveTrigger::IsActive()
{
    if (bot->GetExactDist2d(SUNWELL_CENTER_POSITION) > SUNWELL_CENTER_RADIUS)
        return false;

    return !AI_VALUE(GuidVector, "kiljaeden hands").empty();
}

bool KiljaedenTanksShouldHoldBossAndReflectionsTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return !IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenBossEngagedByMeleeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return !IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenBossEngagedByRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    // Allow Demo Locks to AoE the Reflections
    if (bot->getClass() == CLASS_WARLOCK && bot->HasAura(Id(SwpSpells::SPELL_METAMORPHOSIS)))
        return !AI_VALUE2(Unit*, "find target", "sinister reflection");

    return true;
}

bool KiljaedenBotHasFireBloomTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!bot->HasAura(Id(SwpSpells::SPELL_FIRE_BLOOM)))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    return kiljaeden && kiljaeden->GetHealthPct() < KILJAEDEN_PHASE4_HP_THRESHOLD;
}

bool KiljaedenSaysChaosDestructionOblivionTrigger::IsActiveInEncounter()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenDragonOrbIsActiveTrigger::IsActiveInEncounter()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD)
        return false;

    if (GetKiljaedenDragonOrbUser(bot) != bot)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    bool orbInUse = false;
    bool result = false;

    for (ObjectGuid const& orbGuid : AI_VALUE(GuidVector, "kiljaeden dragon orbs"))
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

bool KiljaedenBotHasStaleRootAfterDragonTrigger::IsActiveInEncounter()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD)
        return false;

    if (GetKiljaedenDragonOrbUser(bot) != bot)
        return false;

    if (!bot->IsRooted() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    if (HasKiljaedenDragonAura(bot) || HasRecentKiljaedenDragonOrbUse(bot, DRAGON_ORB_USE_GRACE_MS))
        return false;

    return bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == NULL_MOTION_TYPE;
}

bool KiljaedenBotControlsDragonTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return false;

    if (!HasKiljaedenDragonAura(bot))
        return false;

    return GetKiljaedenControlledDragon(bot);
}
