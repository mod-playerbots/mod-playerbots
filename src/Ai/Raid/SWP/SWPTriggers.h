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

class SunwellPlateauEncounterTrigger : public Trigger
{
public:
    SunwellPlateauEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class SunwellPlateauNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    SunwellPlateauNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell plateau no encounter in progress", 1000) {}
    bool IsActive() override;
};

class SunwellPlateauBotHasAuraToRemoveTrigger : public Trigger
{
public:
    // Also throttled, though this can occur in combat (clear Ice Block and Divine Shield). A bit
    // of a delay here feels more realistic anyway.
    SunwellPlateauBotHasAuraToRemoveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell plateau bot has aura to remove", 1000) {}
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

class KalecgosShouldCommunicateBossHealthTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosShouldCommunicateBossHealthTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos should communicate boss health") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosPullingBossTrigger : public Trigger
{
public:
    KalecgosPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kalecgos pulling boss") {}
    bool IsActive() override;
};

class KalecgosRequiresTankRotationTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosRequiresTankRotationTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos requires tank rotation") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos spectral rift is open") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsTakeSplashDamageTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosBotsTakeSplashDamageTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos bots take splash damage") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosTooManyArcaneBuffetStacksTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosTooManyArcaneBuffetStacksTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos too many arcane buffet stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosHumanoidKalecTanksSathrovarrTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosHumanoidKalecTanksSathrovarrTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos humanoid kalec tanks sathrovarr") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsDontObserveGravityTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KalecgosBotsDontObserveGravityTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kalecgos bots don't observe gravity") {}

protected:
    bool IsActiveInEncounter() override;
};

// Brutallus

class BrutallusPullingBossTrigger : public Trigger
{
public:
    BrutallusPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "brutallus pulling boss") {}
    bool IsActive() override;
};

class BrutallusRequiresTwoTanksTrigger : public SunwellPlateauEncounterTrigger
{
public:
    BrutallusRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "brutallus requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusMeleeShouldStandInPlaceTrigger : public SunwellPlateauEncounterTrigger
{
public:
    BrutallusMeleeShouldStandInPlaceTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "brutallus melee should stand in place") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusRangedShouldSoakMeteorSlashTrigger : public SunwellPlateauEncounterTrigger
{
public:
    BrutallusRangedShouldSoakMeteorSlashTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "brutallus ranged should soak meteor slash") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusBotIsBurningTrigger : public SunwellPlateauEncounterTrigger
{
public:
    BrutallusBotIsBurningTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "brutallus bot is burning") {}

protected:
    bool IsActiveInEncounter() override;
};

// Felmyst

class FelmystPullingBossTrigger : public Trigger
{
public:
    FelmystPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "felmyst pulling boss") {}
    bool IsActive() override;
};

class FelmystGroundPhaseShouldBeTankedTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystGroundPhaseShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst ground phase should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystRangedShouldPositionToDispelAndFleeTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystRangedShouldPositionToDispelAndFleeTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst ranged should position to dispel and flee") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeShouldStayTogetherTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystMeleeShouldStayTogetherTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst melee should stay together") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotIsEncapsulatedTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystBotIsEncapsulatedTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst bot is encapsulated") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotNearEncapsulatedPlayerTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystBotNearEncapsulatedPlayerTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst bot near encapsulated player") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerHasGasNovaTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystPlayerHasGasNovaTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst player has gas nova") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldAvoidDemonicVaporTrailsTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystShouldAvoidDemonicVaporTrailsTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst should avoid demonic vapor trails") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystBotIsDemonicVaporTargetTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystBotIsDemonicVaporTargetTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst bot is demonic vapor target") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst fog of corruption is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeCannotReachFlyingBossTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystMeleeCannotReachFlyingBossTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst melee cannot reach flying boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerIsCharmedByFogTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystPlayerIsCharmedByFogTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst player is charmed by fog") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldHoldDpsWhileLandingTrigger : public SunwellPlateauEncounterTrigger
{
public:
    FelmystShouldHoldDpsWhileLandingTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "felmyst should hold dps while landing") {}

protected:
    bool IsActiveInEncounter() override;
};

// Eredar Twins

class EredarTwinsMeleeIsAtBalconyTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsMeleeIsAtBalconyTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins melee is at balcony") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldAnnounceAlythessTankTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsShouldAnnounceAlythessTankTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins should announce alythess tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsPullingBossesTrigger : public Trigger
{
public:
    EredarTwinsPullingBossesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins pulling bosses") {}
    bool IsActive() override;
};

class EredarTwinsSacrolashRequiresTwoTanksTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsSacrolashRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins sacrolash requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsAlythessCastsBlazeOnTankTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsAlythessCastsBlazeOnTankTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins alythess casts blaze on tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsRangedNeedsLosTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsRangedNeedsLosTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins ranged needs los") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsOnlyAlythessRemainsTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsOnlyAlythessRemainsTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins only alythess remains") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsTooManyFlameTouchedStacksTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsTooManyFlameTouchedStacksTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins too many flame touched stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldFocusDpsTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins should focus dps") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsActiveConflagrationTargetTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsActiveConflagrationTargetTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins active conflagration target") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashVictimHasConflagrationTrigger : public SunwellPlateauEncounterTrigger
{
public:
    EredarTwinsSacrolashVictimHasConflagrationTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "eredar twins sacrolash victim has conflagration") {}

protected:
    bool IsActiveInEncounter() override;
};

// M'uru

class MuruVoidSentinelOrEntropiusHasAppearedTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruVoidSentinelOrEntropiusHasAppearedTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru void sentinel or entropius has appeared") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBossTransformedIntoEntropiusTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruBossTransformedIntoEntropiusTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru boss transformed into entropius") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruRangedShouldStackOrSpreadTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruRangedShouldStackOrSpreadTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru ranged should stack or spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDeterminingDpsPriorityTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruDeterminingDpsPriorityTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru determining dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSentinelPulsesShadowTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruVoidSentinelPulsesShadowTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru void sentinel pulses shadow") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruAddsSpawnAtEntranceTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruAddsSpawnAtEntranceTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru adds spawn at entrance") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarkFiendsSpawnedTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruDarkFiendsSpawnedTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru dark fiends spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarknessIsComingTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruDarknessIsComingTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru darkness is coming") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBerserkerIsBuffedWithFlurryTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruBerserkerIsBuffedWithFlurryTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru berserker is buffed with flurry") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageCastingFelFireballTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru fury mage casting fel fireball") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageIsBuffedWithSpellFuryTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruFuryMageIsBuffedWithSpellFuryTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru fury mage is buffed with spell fury") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru void spawn available for enslave") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru warlock has enslaved void spawn") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru entropius darkness pools spawn dark fiends") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruTheSingularityIsNearTrigger : public SunwellPlateauEncounterTrigger
{
public:
    MuruTheSingularityIsNearTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "m'uru the singularity is near") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kil'jaeden <The Deceiver>

// Kil'jaeden is the one Sunwell encounter that does not report IN_PROGRESS on engage:
// boss_kiljaeden does not chain BossAI::JustEngagedWith, and the controller sets the state only
// once the first Hand of the Deceiver dies. The two triggers below are the ones that run before
// that, so they cannot take SunwellPlateauEncounterTrigger. Every trigger after them needs
// Kil'jaeden himself, who does not emerge until all three Hands are dead.

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

class KiljaedenTanksShouldHoldBossAndReflectionsTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenTanksShouldHoldBossAndReflectionsTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden tanks should hold boss and reflections") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBossEngagedByMeleeTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenBossEngagedByMeleeTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden boss engaged by melee") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBossEngagedByRangedTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenBossEngagedByRangedTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden boss engaged by ranged") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotHasFireBloomTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenBotHasFireBloomTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden bot has fire bloom") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenSaysChaosDestructionOblivionTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenSaysChaosDestructionOblivionTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden says: Chaos! Destruction! Oblivion!") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenDragonOrbIsActiveTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenDragonOrbIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden dragon orb is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotHasStaleRootAfterDragonTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenBotHasStaleRootAfterDragonTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden bot has stale root after dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotControlsDragonTrigger : public SunwellPlateauEncounterTrigger
{
public:
    KiljaedenBotControlsDragonTrigger(PlayerbotAI* botAI)
        : SunwellPlateauEncounterTrigger(botAI, "kil'jaeden bot controls dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
