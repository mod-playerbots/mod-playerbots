/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPTRIGGERS_H
#define PLAYERBOTS_SWPTRIGGERS_H

#include "EncounterHelpers.h"
#include "SWPShared.h"
#include "Trigger.h"
#include <string>

// General

class SunwellEncounterTrigger : public Trigger
{
public:
    SunwellEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class SunwellNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    SunwellNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell no encounter in progress", 1000) {}
    bool IsActive() override;
};

class SunwellBotHasAuraToRemoveTrigger : public Trigger
{
public:
    // Also throttled, though this can occur in combat (clear Ice Block and Divine Shield). A bit
    // of a delay here feels more realistic anyway.
    SunwellBotHasAuraToRemoveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell bot has aura to remove", 1000) {}
    bool IsActive() override;
};

// Trash

class VolatileFiendSelfDestructsWhenNearTrigger : public Trigger
{
public:
    VolatileFiendSelfDestructsWhenNearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "volatile fiend self destructs when near") {}
    bool IsActive() override;
};

class ApocalypseGuardProtectedByInfernalDefenseTrigger : public Trigger
{
public:
    ApocalypseGuardProtectedByInfernalDefenseTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "apocalypse guard protected by infernal defense") {}
    bool IsActive() override;
};

// Kalecgos

class KalecgosShouldCommunicateBossHealthTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosShouldCommunicateBossHealthTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos should communicate boss health") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosPullingBossTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosPullingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos pulling boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosRequiresTankRotationTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosRequiresTankRotationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos requires tank rotation") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos spectral rift is open") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsTakeSplashDamageTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosBotsTakeSplashDamageTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos bots take splash damage") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosTooManyArcaneBuffetStacksTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosTooManyArcaneBuffetStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos too many arcane buffet stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosHumanoidKalecTanksSathrovarrTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosHumanoidKalecTanksSathrovarrTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos humanoid kalec tanks sathrovarr") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsDontObserveGravityTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosBotsDontObserveGravityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos bots don't observe gravity") {}

protected:
    bool IsActiveInEncounter() override;
};

// Brutallus

class BrutallusPullingBossTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusPullingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus pulling boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusRequiresTwoTanksTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusMeleeShouldStandInPlaceTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusMeleeShouldStandInPlaceTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus melee should stand in place") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusRangedShouldSoakMeteorSlashTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRangedShouldSoakMeteorSlashTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus ranged should soak meteor slash") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusBotIsBurningTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusBotIsBurningTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus bot is burning") {}

protected:
    bool IsActiveInEncounter() override;
};

// Felmyst

class FelmystPullingBossTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPullingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst pulling boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystGroundPhaseShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    FelmystGroundPhaseShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst ground phase should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystRangedShouldPositionToDispelAndFleeTrigger : public SunwellEncounterTrigger
{
public:
    FelmystRangedShouldPositionToDispelAndFleeTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst ranged should position to dispel and flee") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeShouldStayTogetherTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeShouldStayTogetherTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst melee should stay together") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotIsEncapsulatedTrigger : public SunwellEncounterTrigger
{
public:
    FelmystBotIsEncapsulatedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst bot is encapsulated") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotNearEncapsulatedPlayerTrigger : public SunwellEncounterTrigger
{
public:
    FelmystBotNearEncapsulatedPlayerTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst bot near encapsulated player") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerHasGasNovaTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerHasGasNovaTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst player has gas nova") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldAvoidDemonicVaporTrailsTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldAvoidDemonicVaporTrailsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst should avoid demonic vapor trails") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotIsDemonicVaporTargetTrigger : public SunwellEncounterTrigger
{
public:
    FelmystBotIsDemonicVaporTargetTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst bot is demonic vapor target") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst fog of corruption is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeCannotReachFlyingBossTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeCannotReachFlyingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst melee cannot reach flying boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerIsCharmedByFogTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerIsCharmedByFogTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst player is charmed by fog") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldHoldDpsWhileLandingTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldHoldDpsWhileLandingTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst should hold dps while landing") {}

protected:
    bool IsActiveInEncounter() override;
};

// Eredar Twins

class EredarTwinsMeleeIsAtBalconyTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsMeleeIsAtBalconyTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins melee is at balcony") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldAnnounceAlythessTankTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldAnnounceAlythessTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins should announce alythess tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsPullingBossesTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsPullingBossesTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins pulling bosses") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashRequiresTwoTanksTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins sacrolash requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsAlythessCastsBlazeOnTankTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsAlythessCastsBlazeOnTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins alythess casts blaze on tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsRangedNeedsLosTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsRangedNeedsLosTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins ranged needs los") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsOnlyAlythessRemainsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsOnlyAlythessRemainsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins only alythess remains") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsTooManyFlameTouchedStacksTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsTooManyFlameTouchedStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins too many flame touched stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldFocusDpsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins should focus dps") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsActiveConflagrationTargetTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsActiveConflagrationTargetTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins active conflagration target") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashVictimHasConflagrationTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashVictimHasConflagrationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins sacrolash victim has conflagration") {}

protected:
    bool IsActiveInEncounter() override;
};

// M'uru

class MuruVoidSentinelOrEntropiusHasAppearedTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSentinelOrEntropiusHasAppearedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru void sentinel or entropius has appeared") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBossTransformedIntoEntropiusTrigger : public SunwellEncounterTrigger
{
public:
    MuruBossTransformedIntoEntropiusTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru boss transformed into entropius") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruRangedShouldStackOrSpreadTrigger : public SunwellEncounterTrigger
{
public:
    MuruRangedShouldStackOrSpreadTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru ranged should stack or spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDeterminingDpsPriorityTrigger : public SunwellEncounterTrigger
{
public:
    MuruDeterminingDpsPriorityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru determining dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSentinelPulsesShadowTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSentinelPulsesShadowTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru void sentinel pulses shadow") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruAddsSpawnAtEntranceTrigger : public SunwellEncounterTrigger
{
public:
    MuruAddsSpawnAtEntranceTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru adds spawn at entrance") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarkFiendsSpawnedTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarkFiendsSpawnedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru dark fiends spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarknessIsComingTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarknessIsComingTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru darkness is coming") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBerserkerIsBuffedWithFlurryTrigger : public SunwellEncounterTrigger
{
public:
    MuruBerserkerIsBuffedWithFlurryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru berserker is buffed with flurry") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageCastingFelFireballTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru fury mage casting fel fireball") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageIsBuffedWithSpellFuryTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageIsBuffedWithSpellFuryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru fury mage is buffed with spell fury") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru void spawn available for enslave") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public SunwellEncounterTrigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru warlock has enslaved void spawn") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger : public SunwellEncounterTrigger
{
public:
    MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru entropius darkness pools spawn dark fiends") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruTheSingularityIsNearTrigger : public SunwellEncounterTrigger
{
public:
    MuruTheSingularityIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru the singularity is near") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kil'jaeden <The Deceiver>

// Kil'jaeden is the one Sunwell encounter that does not report IN_PROGRESS on engage:
// boss_kiljaeden does not chain BossAI::JustEngagedWith, and the controller sets the state only
// once the first Hand of the Deceiver dies. The two triggers below are the ones that run before
// that, so they cannot inherit from SunwellEncounterTrigger. Every trigger after them needs
// Kil'jaeden himself so they can be subclassed.

class KiljaedenShouldCoordinateOrbUseTrigger : public Trigger
{
public:
    KiljaedenShouldCoordinateOrbUseTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden should coordinate orb use") {}
    bool IsActive() override;
};

class KiljaedenHandsOfTheDeceiverAreActiveTrigger : public Trigger
{
public:
    KiljaedenHandsOfTheDeceiverAreActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden hands of the deceiver are active") {}
    bool IsActive() override;
};

class KiljaedenTanksShouldHoldBossAndReflectionsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenTanksShouldHoldBossAndReflectionsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden tanks should hold boss and reflections") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden melee should split into two groups") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenRangedShouldSpreadInTwoArcsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenRangedShouldSpreadInTwoArcsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden ranged should spread in two arcs") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotHasFireBloomTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenBotHasFireBloomTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden bot has fire bloom") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenSaysChaosDestructionOblivionTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenSaysChaosDestructionOblivionTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden says: Chaos! Destruction! Oblivion!") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenDragonOrbIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenDragonOrbIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden dragon orb is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotControlsDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenBotControlsDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden bot controls dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotHasStaleRootAfterDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenBotHasStaleRootAfterDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden bot has stale root after dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
