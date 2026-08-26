/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPTRIGGERS_H
#define PLAYERBOTS_SWPTRIGGERS_H

#include "Trigger.h"

// General

class SunwellPlateauNoEncounterInProgressTrigger : public Trigger
{
public:
    SunwellPlateauNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell plateau no encounter in progress") {}
    bool IsActive() override;
};

class SunwellPlateauBotHasAuraToRemoveTrigger : public Trigger
{
public:
    SunwellPlateauBotHasAuraToRemoveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell plateau bot has aura to remove") {}
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

class KalecgosShouldCommunicateBossHealthTrigger : public Trigger
{
public:
    KalecgosShouldCommunicateBossHealthTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos should communicate boss health") {}
    bool IsActive() override;
};

class KalecgosPullingBossTrigger : public Trigger
{
public:
    KalecgosPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kalecgos pulling boss") {}
    bool IsActive() override;
};

class KalecgosBossRequiresTankRotationTrigger : public Trigger
{
public:
    KalecgosBossRequiresTankRotationTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos boss requires tank rotation") {}
    bool IsActive() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public Trigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos spectral rift is open") {}
    bool IsActive() override;
};

class KalecgosBotsTakeSplashDamageTrigger : public Trigger
{
public:
    KalecgosBotsTakeSplashDamageTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos bots take splash damage") {}
    bool IsActive() override;
};

class KalecgosTooManyArcaneBuffetStacksTrigger : public Trigger
{
public:
    KalecgosTooManyArcaneBuffetStacksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos too many arcane buffet stacks") {}
    bool IsActive() override;
};

class KalecgosHumanoidKalecTanksSathrovarrTrigger : public Trigger
{
public:
    KalecgosHumanoidKalecTanksSathrovarrTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos humanoid kalec tanks sathrovarr") {}
    bool IsActive() override;
};

class KalecgosBotsDontObserveGravityTrigger : public Trigger
{
public:
    KalecgosBotsDontObserveGravityTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kalecgos bots don't observe gravity") {}
    bool IsActive() override;
};

// Brutallus

class BrutallusPullingBossTrigger : public Trigger
{
public:
    BrutallusPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "brutallus pulling boss") {}
    bool IsActive() override;
};

class BrutallusRequiresTwoTanksTrigger : public Trigger
{
public:
    BrutallusRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "brutallus requires two tanks") {}
    bool IsActive() override;
};

class BrutallusMeleeShouldStandInPlaceTrigger : public Trigger
{
public:
    BrutallusMeleeShouldStandInPlaceTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "brutallus melee should stand in place") {}
    bool IsActive() override;
};

class BrutallusRangedShouldSoakMeteorSlashTrigger : public Trigger
{
public:
    BrutallusRangedShouldSoakMeteorSlashTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "brutallus ranged should soak meteor slash") {}
    bool IsActive() override;
};

class BrutallusBotIsBurningTrigger : public Trigger
{
public:
    BrutallusBotIsBurningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "brutallus bot is burning") {}
    bool IsActive() override;
};

// Felmyst

class FelmystPullingBossTrigger : public Trigger
{
public:
    FelmystPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "felmyst pulling boss") {}
    bool IsActive() override;
};

class FelmystGroundPhaseShouldBeTankedTrigger : public Trigger
{
public:
    FelmystGroundPhaseShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst ground phase should be tanked") {}
    bool IsActive() override;
};

class FelmystRangedShouldPositionToDispelAndFleeTrigger : public Trigger
{
public:
    FelmystRangedShouldPositionToDispelAndFleeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst ranged should position to dispel and flee") {}
    bool IsActive() override;
};

class FelmystMeleeShouldStayTogetherTrigger : public Trigger
{
public:
    FelmystMeleeShouldStayTogetherTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst melee should stay together") {}
    bool IsActive() override;
};

class FelmystBotIsEncapsulatedTrigger : public Trigger
{
public:
    FelmystBotIsEncapsulatedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst bot is encapsulated") {}
    bool IsActive() override;
};

class FelmystBotNearEncapsulatedPlayerTrigger : public Trigger
{
public:
    FelmystBotNearEncapsulatedPlayerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst bot near encapsulated player") {}
    bool IsActive() override;
};

class FelmystPlayerHasGasNovaTrigger : public Trigger
{
public:
    FelmystPlayerHasGasNovaTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst player has gas nova") {}
    bool IsActive() override;
};

class FelmystShouldAvoidDemonicVaporTrailsTrigger : public Trigger
{
public:
    FelmystShouldAvoidDemonicVaporTrailsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst should avoid demonic vapor trails") {}
    bool IsActive() override;
};

class FelmystBotIsDemonicVaporTargetTrigger : public Trigger
{
public:
    FelmystBotIsDemonicVaporTargetTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst bot is demonic vapor target") {}
    bool IsActive() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public Trigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst fog of corruption is active") {}
    bool IsActive() override;
};

class FelmystMeleeCannotReachFlyingBossTrigger : public Trigger
{
public:
    FelmystMeleeCannotReachFlyingBossTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst melee cannot reach flying boss") {}
    bool IsActive() override;
};

class FelmystPlayerIsCharmedByFogTrigger : public Trigger
{
public:
    FelmystPlayerIsCharmedByFogTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst player is charmed by fog") {}
    bool IsActive() override;
};

class FelmystShouldHoldDpsWhileLandingTrigger : public Trigger
{
public:
    FelmystShouldHoldDpsWhileLandingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "felmyst should hold dps while landing") {}
    bool IsActive() override;
};

// Eredar Twins

class EredarTwinsMeleeIsAtBalconyTrigger : public Trigger
{
public:
    EredarTwinsMeleeIsAtBalconyTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins melee is at balcony") {}
    bool IsActive() override;
};

class EredarTwinsPullingBossesTrigger : public Trigger
{
public:
    EredarTwinsPullingBossesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins pulling bosses") {}
    bool IsActive() override;
};

class EredarTwinsSacrolashRequiresTwoTanksTrigger : public Trigger
{
public:
    EredarTwinsSacrolashRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins sacrolash requires two tanks") {}
    bool IsActive() override;
};

class EredarTwinsAlythessCastsBlazeOnTankTrigger : public Trigger
{
public:
    EredarTwinsAlythessCastsBlazeOnTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins alythess casts blaze on tank") {}
    bool IsActive() override;
};

class EredarTwinsRangedNeedsLosTrigger : public Trigger
{
public:
    EredarTwinsRangedNeedsLosTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins ranged needs los") {}
    bool IsActive() override;
};

class EredarTwinsOnlyAlythessRemainsTrigger : public Trigger
{
public:
    EredarTwinsOnlyAlythessRemainsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins only alythess remains") {}
    bool IsActive() override;
};

class EredarTwinsTooManyFlameTouchedStacksTrigger : public Trigger
{
public:
    EredarTwinsTooManyFlameTouchedStacksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins too many flame touched stacks") {}
    bool IsActive() override;
};

class EredarTwinsShouldFocusDpsTrigger : public Trigger
{
public:
    EredarTwinsShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins should focus dps") {}
    bool IsActive() override;
};

class EredarTwinsActiveConflagrationTargetTrigger : public Trigger
{
public:
    EredarTwinsActiveConflagrationTargetTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins active conflagration target") {}
    bool IsActive() override;
};

class EredarTwinsSacrolashVictimHasConflagrationTrigger : public Trigger
{
public:
    EredarTwinsSacrolashVictimHasConflagrationTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "eredar twins sacrolash victim has conflagration") {}
    bool IsActive() override;
};

// M'uru

class MuruVoidSentinelOrEntropiusHasAppearedTrigger : public Trigger
{
public:
    MuruVoidSentinelOrEntropiusHasAppearedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru void sentinel or entropius has appeared") {}
    bool IsActive() override;
};

class MuruBossTransformedIntoEntropiusTrigger : public Trigger
{
public:
    MuruBossTransformedIntoEntropiusTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru boss transformed into entropius") {}
    bool IsActive() override;
};

class MuruRangedShouldStackOrSpreadTrigger : public Trigger
{
public:
    MuruRangedShouldStackOrSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru ranged should stack or spread") {}
    bool IsActive() override;
};

class MuruDeterminingDpsPriorityTrigger : public Trigger
{
public:
    MuruDeterminingDpsPriorityTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru determining dps priority") {}
    bool IsActive() override;
};

class MuruVoidSentinelPulsesShadowTrigger : public Trigger
{
public:
    MuruVoidSentinelPulsesShadowTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru void sentinel pulses shadow") {}
    bool IsActive() override;
};

class MuruAddsSpawnAtEntranceTrigger : public Trigger
{
public:
    MuruAddsSpawnAtEntranceTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru adds spawn at entrance") {}
    bool IsActive() override;
};

class MuruDarkFiendsSpawnedTrigger : public Trigger
{
public:
    MuruDarkFiendsSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru dark fiends spawned") {}
    bool IsActive() override;
};

class MuruDarknessIsComingTrigger : public Trigger
{
public:
    MuruDarknessIsComingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "m'uru darkness is coming") {}
    bool IsActive() override;
};

class MuruBerserkerIsBuffedWithFlurryTrigger : public Trigger
{
public:
    MuruBerserkerIsBuffedWithFlurryTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru berserker is buffed with flurry") {}
    bool IsActive() override;
};

class MuruFuryMageCastingFelFireballTrigger : public Trigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru fury mage casting fel fireball") {}
    bool IsActive() override;
};

class MuruFuryMageIsBuffedWithSpellFuryTrigger : public Trigger
{
public:
    MuruFuryMageIsBuffedWithSpellFuryTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru fury mage is buffed with spell fury") {}
    bool IsActive() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public Trigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru void spawn available for enslave") {}
    bool IsActive() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public Trigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru warlock has enslaved void spawn") {}
    bool IsActive() override;
};

class MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger : public Trigger
{
public:
    MuruEntropiusDarknessPoolsSpawnDarkFiendsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru entropius darkness pools spawn dark fiends") {}
    bool IsActive() override;
};

class MuruTheSingularityIsNearTrigger : public Trigger
{
public:
    MuruTheSingularityIsNearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "m'uru the singularity is near") {}
    bool IsActive() override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenEncounterHasBegunTrigger : public Trigger
{
public:
    KiljaedenEncounterHasBegunTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden encounter has begun") {}
    bool IsActive() override;
};

class KiljaedenHandsOfTheDeceiverAreActiveTrigger : public Trigger
{
public:
    KiljaedenHandsOfTheDeceiverAreActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden hands of the deceiver are active") {}
    bool IsActive() override;
};

class KiljaedenTanksShouldHoldBossAndReflectionsTrigger : public Trigger
{
public:
    KiljaedenTanksShouldHoldBossAndReflectionsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden tanks should hold boss and reflections") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByMeleeTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByMeleeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden boss engaged by melee") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByRangedTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByRangedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden boss engaged by ranged") {}
    bool IsActive() override;
};

class KiljaedenBotHasFireBloomTrigger : public Trigger
{
public:
    KiljaedenBotHasFireBloomTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden bot has fire bloom") {}
    bool IsActive() override;
};

class KiljaedenSaysChaosDestructionOblivionTrigger : public Trigger
{
public:
    KiljaedenSaysChaosDestructionOblivionTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden says: Chaos! Destruction! Oblivion!") {}
    bool IsActive() override;
};

class KiljaedenDragonOrbIsActiveTrigger : public Trigger
{
public:
    KiljaedenDragonOrbIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden dragon orb is active") {}
    bool IsActive() override;
};

class KiljaedenBotHasStaleRootAfterDragonTrigger : public Trigger
{
public:
    KiljaedenBotHasStaleRootAfterDragonTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden bot has stale root after dragon") {}
    bool IsActive() override;
};

class KiljaedenBotControlsDragonTrigger : public Trigger
{
public:
    KiljaedenBotControlsDragonTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kil'jaeden bot controls dragon") {}
    bool IsActive() override;
};

#endif
