/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPTriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"

using namespace SwpHelpers;

// General

bool SunwellPlateauBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == SWP_MAP_ID && !AI_VALUE2(bool, "combat", "self target");
}

bool SunwellPlateauBotHasProtectiveAuraTrigger::IsActive()
{
    if (bot->getClass() == CLASS_MAGE)
    {
        if (bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_ICE_BLOCK)))
            return true;
    }
    else if (bot->getClass() == CLASS_PALADIN && !PlayerbotAI::IsHeal(bot))
    {
        if (bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_DIVINE_SHIELD)))
            return true;
    }

    return false;
}

// Trash

bool VolatileFiendSelfDestructsWhenNearTrigger::IsActive()
{
    constexpr float searchRadius = 25.0f;
    Unit* volatileFiend = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_VOLATILE_FIEND), searchRadius, true);

    if (!volatileFiend)
        return false;

    // Z-position check is so bots will go up the ramp to M'uru without clearing
    // the volatile fiends below
    return std::abs(bot->GetPositionZ() - volatileFiend->GetPositionZ()) < 10.0f;
}

bool ApocalypseGuardProtectedByInfernalDefenseTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "apocalypse guard");
}

// Kalecgos

bool KalecgosShouldCommunicateBossHealthTrigger::IsActive()
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

bool KalecgosBossEngagedByTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    return !IsInSpectralRealm(bot);
}

bool KalecgosSpectralRiftIsOpenTrigger::IsActive()
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot))
        return false;

    if (!ShouldEnterKalecgosPortal(bot))
        return false;

    constexpr float searchRadius = 75.0f;
    return bot->FindNearestGameObject(
        static_cast<uint32>(SwpObjects::GO_SPECTRAL_RIFT), searchRadius, true);
}

bool KalecgosBotsTakeSplashDamageTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot) || kalecgos->GetVictim() == bot)
        return false;

    return !ShouldEnterKalecgosPortal(bot);
}

bool KalecgosBotHasTooManyArcaneBuffetStacksTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->IsFriendlyTo(bot) || IsInSpectralRealm(bot))
        return false;

    Aura* arcaneBuffet = bot->GetAura(
        static_cast<uint32>(SwpSpells::SPELL_ARCANE_BUFFET));
    return arcaneBuffet && arcaneBuffet->GetStackAmount() >= 10;
}

bool KalecgosHumanoidKalecTanksSathrovarrTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && IsInSpectralRealm(bot);
}

bool KalecgosBotsDontObserveGravityTrigger::IsActive()
{
    if (!IsInSpectralRealm(bot))
        return false;

    constexpr float verticalOffset = 5.0f;
    return bot->GetPositionZ() > KALECGOS_SPECTRAL_REALM_Z + verticalOffset ||
        bot->GetPositionZ() < KALECGOS_SPECTRAL_REALM_Z - verticalOffset;
}

// Brutallus

bool BrutallusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetHealthPct() > 95.0f;
}

bool BrutallusBossEngagedByTanksTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusBossEngagedByMeleeTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        return false;
    }

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetVictim() != bot;
}

bool BrutallusBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_BURN)))
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetVictim() != bot;
}

bool BrutallusBotIsBurningTrigger::IsActive()
{
    if (!bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_BURN)))
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

    if (felmyst->GetHealthPct() > 90.0f)
        return true;

    if (felmyst->IsFlying())
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && felmyst->GetVictim() != mainTank)
        return true;

    return false;
}

bool FelmystBossEngagedByMainTankOnGroundTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    return felmyst && !felmyst->IsFlying();
}

bool FelmystBossEngagedByRangedOnGroundTrigger::IsActive()
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

    // On initial landing, let MT get aggro before trying to line up
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && felmyst->GetVictim() != mainTank && felmyst->GetHealthPct() > 90.0f)
        return false;

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool FelmystBossEngagedByMeleeOnGroundTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
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

    if (felmyst->GetVictim() == bot || PlayerbotAI::IsMainTank(bot))
        return false;

    return !GetFelmystEncapsulateTarget(bot) && !DidEncapsulateOccurThisGroundPhase(bot);
}

bool FelmystBotIsEncapsulatedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    if (!bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_ENCAPSULATE)))
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool FelmystBotNearEncapsulatedPlayerTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    FelmystGroundStack const botStack =
        GetClosestFelmystGroundStack(bot, felmyst, bot);
    FelmystGroundStack const targetStack =
        GetClosestFelmystGroundStack(bot, felmyst, encapsulateTarget);

    return botStack != FelmystGroundStack::None && botStack == targetStack;
}

bool FelmystPlayerHasGasNovaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return GetFelmystGasNovaDispelTarget(bot);
}

bool FelmystShouldAvoidDemonicVaporTrailsTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    if (GetFelmystDemonicVaporSummonedByBot(bot))
        return false;

    FogOfCorruptionState fogState;
    return !TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);
}

bool FelmystBotIsDemonicVaporTargetTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(bot, felmyst, fogState))
        return false;

    return IsFelmystDemonicVaporHeadNearBot(bot);
}

bool FelmystFogOfCorruptionIsActiveTrigger::IsActive()
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

bool FelmystMeleeCannotReachBossTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || AI_VALUE(Unit*, "current target") != felmyst)
        return false;

    return IsFelmystAirPhaseTargetSuppressed(felmyst);
}

bool FelmystPlayerIsCharmedByFogTrigger::IsActive()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    return felmyst && GetFelmystCharmedTarget(bot, felmyst);
}

bool FelmystShouldHoldDpsWhileLandingTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID) && AI_VALUE2(Unit*, "find target", "felmyst");
}

// Eredar Twins

bool EredarTwinsMeleeIsAtBalconyTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (bot->GetPositionZ() <= EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess");
}

bool EredarTwinsPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    return alythess && alythess->GetHealthPct() > 90.0f;
}

bool EredarTwinsSacrolashEngagedByTwoTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
        IsAnySacrolashTank(bot);
}

bool EredarTwinsAlythessEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") &&
        IsAlythessTank(bot);
}

bool EredarTwinsBossesEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    return GetEredarTwinsConflagrationTarget(bot) != bot;
}

bool EredarTwinsOnlyOneBossRemainsTrigger::IsActive()
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

bool EredarTwinsBotHasTooManyFlameTouchedStacksTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Aura* flameSear = bot->GetAura(
        static_cast<uint32>(SwpSpells::SPELL_FLAME_SEAR));
    if (!flameSear || flameSear->GetDuration() > 2000)
        return false;

    Aura* flameTouched = bot->GetAura(
        static_cast<uint32>(SwpSpells::SPELL_FLAME_TOUCHED));
    return flameTouched && flameTouched->GetStackAmount() >= 5;
}

bool EredarTwinsDeterminingDpsPriorityTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    return !IsAnySacrolashTank(bot) && !IsAlythessTank(bot);
}

bool EredarTwinsBotHasConflagrationTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
        GetEredarTwinsConflagrationTarget(bot) == bot;
}

bool EredarTwinsSacrolashVictimHasConflagrationTrigger::IsActive()
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

bool MuruVoidSentinelOrEntropiusHasAppearedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() > 80.0f)
        return true;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    return voidSentinel && voidSentinel->GetHealthPct() > 80.0f;
}

bool MuruBossTransformedIntoEntropiusTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "entropius");
}

bool MuruBossesEngagedByRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "m'uru");
}

bool MuruDeterminingDpsPriorityTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "m'uru");
}

bool MuruVoidSentinelPulsesShadowTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (AI_VALUE2(Unit*, "find target", "void sentinel"))
        return true;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    return muru && muru->GetHealth() > 1;
}

bool MuruAddsSpawnAtEntranceTrigger::IsActive()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() == 1)
        return false;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (voidSentinel && voidSentinel->GetVictim() == bot)
        return false;

    return !AI_VALUE2(Unit*, "find target", "shadowsword fury mage") &&
        !AI_VALUE2(Unit*, "find target", "shadowsword berserker");
}

bool MuruDarkFiendsSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    return AI_VALUE2(Unit*, "find target", "dark fiend");
}

bool MuruEntropiusSpawnsDarknessPoolsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "entropius"))
        return false;

    if (AI_VALUE2(Unit*, "find target", "dark fiend"))
        return true;

    constexpr float searchRadius = 20.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_DARKNESS), searchRadius, true);
}

bool MuruDarknessIsComingTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() == 1)
        return false;

    return TryGetMuruDarknessActiveState(bot, muru);
}

bool MuruTheSingularityIsNearTrigger::IsActive()
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    constexpr float searchRadius = 30.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_SINGULARITY), searchRadius, true);
}

bool MuruBerserkerIsBuffedWithFlurryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARLOCK &&
        bot->getClass() != CLASS_WARRIOR)
    {
        return false;
    }

    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    return berserker && berserker->HasAura(
        static_cast<uint32>(SwpSpells::SPELL_FLURRY));
}

bool MuruFuryMageCastingFelFireballTrigger::IsActive()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN ||
        bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_WARLOCK)
    {
        return false;
    }

    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage && furyMage->HasUnitState(UNIT_STATE_CASTING) &&
        furyMage->FindCurrentSpellBySpellId(static_cast<uint32>(SwpSpells::SPELL_FEL_FIREBALL));
}

bool MuruFuryMageIsBuffedWithSpellFuryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage && furyMage->HasAura(
        static_cast<uint32>(SwpSpells::SPELL_SPELL_FURY));
}

bool MuruVoidSpawnAvailableForEnslaveTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK || bot->GetCharm())
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    return FindAvailableVoidSpawnForEnslave(bot);
}

bool MuruWarlockHasEnslavedVoidSpawnTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* charm = bot->GetCharm();
    if (!charm || !charm->IsAlive() ||
        charm->GetEntry() != static_cast<uint32>(SwpNpcs::NPC_VOID_SPAWN))
    {
        return false;
    }

    return AI_VALUE2(Unit*, "find target", "m'uru");
}

// Kil'jaeden <The Deceiver>

bool KiljaedenEncounterHasBegunTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SWP_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "hand of the deceiver");
}

bool KiljaedenHandsOfTheDeceiverAreActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "hand of the deceiver");
}

bool KiljaedenBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || HasKiljaedenDragonAura(bot) ||
        IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
    {
        return false;
    }

    if (kiljaeden->GetHealthPct() > 85.0f)
        return true;

    if (PlayerbotAI::IsMainTank(bot))
        return true;

    constexpr float searchRadius = 100.0f;
    if (AI_VALUE2(Unit*, "find target", "sinister reflection") ||
        bot->FindNearestCreature(
            static_cast<uint32>(SwpNpcs::NPC_SINISTER_REFLECTION), searchRadius, true))
    {
        return false;
    }

    return true;
}

bool KiljaedenBossEngagedByMeleeTrigger::IsActive()
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || HasKiljaedenDragonAura(bot) ||
        IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
    {
        return false;
    }

    if (kiljaeden->GetHealthPct() <= 85.0f)
    {
        constexpr float searchRadius = 50.0f;
        if (AI_VALUE2(Unit*, "find target", "sinister reflection") ||
            bot->FindNearestCreature(
                static_cast<uint32>(SwpNpcs::NPC_SINISTER_REFLECTION), searchRadius, true))
        {
            return false;
        }
    }

    return true;
}

bool KiljaedenBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || HasKiljaedenDragonAura(bot) ||
        IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
    {
        return false;
    }

    // Let the demo lock go AoE down the reflections
    if (bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_METAMORPHOSIS)))
        return false;

    return true;
}

bool KiljaedenBotHasFireBloomTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!bot->HasAura(static_cast<uint32>(SwpSpells::SPELL_FIRE_BLOOM)))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    return kiljaeden && kiljaeden->GetHealthPct() < 55.0f;
}

bool KiljaedenSaysChaosDestructionOblivionTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenDragonOrbIsActiveTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > 85.0f)
        return false;

    if (HasKiljaedenDragonAura(bot))
        return false;

    if (GetKiljaedenDragonOrbUser(bot) != bot)
        return false;

    bool orbInUse = false;
    bool result = false;

    for (uint32 const orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        GameObject* orb = bot->FindNearestGameObject(orbEntry, 200.0f, true);
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

bool KiljaedenBotHasStaleRootAfterDragonTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > 85.0f)
        return false;

    if (GetKiljaedenDragonOrbUser(bot) != bot)
        return false;

    constexpr uint32 orbUseGraceMs = 2000;
    if (HasKiljaedenDragonAura(bot) || !bot->IsRooted() ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
        HasRecentKiljaedenDragonOrbUse(bot, orbUseGraceMs))
    {
        return false;
    }

    return bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == NULL_MOTION_TYPE;
}

bool KiljaedenBotControlsDragonTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return false;

    if (!HasKiljaedenDragonAura(bot))
        return false;

    return GetKiljaedenControlledDragon(bot);
}
