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

// Shared

class SunwellEncounterTrigger : public Trigger
{
public:
    SunwellEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, SwpHelpers::SwpBosses boss,
        int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, SwpHelpers::SWP_MAP_ID, SwpHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    Unit* GetEncounterBoss() { return SwpHelpers::GetSwpBoss(bot, _boss); }

private:
    SwpHelpers::SwpBosses const _boss;
};

class SunwellNoEncounterInProgressTrigger : public Trigger
{
public:
    SunwellNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell no encounter in progress", 1000) {}
    bool IsActive() override;
};

class SunwellClassRolesShouldBeAssignedTrigger : public Trigger
{
public:
    SunwellClassRolesShouldBeAssignedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell class roles should be assigned", 1000) {}
    bool IsActive() override;
};

class SunwellAuraToRemoveTrigger : public Trigger
{
public:
    SunwellAuraToRemoveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell aura to remove", 1000) {}
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

// Shared Bosses

// For Misdirection before BOSS_ENGAGED_HEALTH_PCT (i.e., shortly after pull). Used for Kalecgos
// and Brutallus.
class SunwellHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    SunwellHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, SwpHelpers::SwpBosses boss)
        : SunwellEncounterTrigger(botAI, name, boss) {}

protected:
    bool IsActiveInEncounter() override;
};

// Kalecgos

class KalecgosShouldCommunicateBossHealthTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosShouldCommunicateBossHealthTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos should communicate boss health",
            SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosRequiresTankRotationTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosRequiresTankRotationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos requires tank rotation", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos spectral rift is open", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosRangedShouldSpreadTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos ranged should spread", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosTooManyArcaneBuffetStacksTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosTooManyArcaneBuffetStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos too many arcane buffet stacks",
            SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosHumanoidKalecTanksSathrovarrTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosHumanoidKalecTanksSathrovarrTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos humanoid kalec tanks sathrovarr",
            SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsDontObserveGravityTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosBotsDontObserveGravityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos bots don't observe gravity", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosShouldManageRaidAssignmentsTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosShouldManageRaidAssignmentsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos should manage raid assignments",
            SwpHelpers::SwpBosses::DATA_KALECGOS, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosDpsHoldIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos dps hold is active", SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosShouldManagePetAggressionTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kalecgos should manage pet aggression",
            SwpHelpers::SwpBosses::DATA_KALECGOS) {}

protected:
    bool IsActiveInEncounter() override;
};

// Brutallus

class BrutallusRequiresTwoTanksTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "brutallus requires two tanks", SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusMeleeShouldStandInPlaceTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusMeleeShouldStandInPlaceTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "brutallus melee should stand in place",
            SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusRangedShouldSoakMeteorSlashTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRangedShouldSoakMeteorSlashTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "brutallus ranged should soak meteor slash",
            SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusBurnOnNonTankTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusBurnOnNonTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "brutallus burn on non-tank", SwpHelpers::SwpBosses::DATA_BRUTALLUS) {}

protected:
    bool IsActiveInEncounter() override;
};

// Felmyst

class FelmystHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    FelmystHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst hunter should misdirect", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystGroundPhaseShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    FelmystGroundPhaseShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst ground phase should be tanked", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystRangedShouldPositionToDispelAndFleeTrigger : public SunwellEncounterTrigger
{
public:
    FelmystRangedShouldPositionToDispelAndFleeTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst ranged should position to dispel and flee",
            SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeShouldStayTogetherTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeShouldStayTogetherTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst melee should stay together", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystEncapsulateOnMageOrPaladinTrigger : public SunwellEncounterTrigger
{
public:
    FelmystEncapsulateOnMageOrPaladinTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst encapsulate on mage or paladin", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystNearEncapsulatedPlayerTrigger : public SunwellEncounterTrigger
{
public:
    FelmystNearEncapsulatedPlayerTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst near encapsulated player", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerHasGasNovaTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerHasGasNovaTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst player has gas nova", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystTakeoffIsNearTrigger : public SunwellEncounterTrigger
{
public:
    FelmystTakeoffIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst takeoff is near", SwpHelpers::SwpBosses::DATA_FELMYST) {}
    bool HasEarlyMoveStarted() const;
    bool ResetEarlyMove();

protected:
    bool IsActiveInEncounter() override;

private:
    // The touchdown the bot's early move started under
    uint32 _earlyMoveTouchdownMs = 0;
};

// Read by the ground-position triggers and Felmyst's update hook.
bool HasFelmystEarlyMoveStarted(PlayerbotAI* botAI);

class FelmystAirPhaseHasStartedTrigger : public SunwellEncounterTrigger
{
public:
    FelmystAirPhaseHasStartedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst air phase has started", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldAvoidDemonicVaporTrailsTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldAvoidDemonicVaporTrailsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst should avoid demonic vapor trails",
            SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystTargetedByDemonicVaporTrigger : public SunwellEncounterTrigger
{
public:
    FelmystTargetedByDemonicVaporTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst targeted by demonic vapor", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst fog of corruption is active", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeCannotReachFlyingBossTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeCannotReachFlyingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst melee cannot reach flying boss", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerIsCharmedByFogTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerIsCharmedByFogTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst player is charmed by fog", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldHoldDpsWhileLandingTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldHoldDpsWhileLandingTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst should hold dps while landing", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystLandingDpsHoldIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    FelmystLandingDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst landing dps hold is active", SwpHelpers::SwpBosses::DATA_FELMYST) {}

protected:
    bool IsActiveInEncounter() override;
};

// Eredar Twins

class EredarTwinsHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins hunter should misdirect",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldLeaveBalconyTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldLeaveBalconyTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins should leave balcony",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins sacrolash should be tanked",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsAlythessShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsAlythessShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins alythess should be tanked",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsRangedShouldStackOnBalconyTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsRangedShouldStackOnBalconyTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins ranged should stack on balcony",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsOnlyAlythessRemainsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsOnlyAlythessRemainsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins only alythess remains",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsAlythessTargetsNonTankTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsAlythessTargetsNonTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins alythess targets non-tank",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsTooManyFlameTouchedStacksTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsTooManyFlameTouchedStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins too many flame touched stacks",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldFocusDpsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins should focus dps", SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldManagePetAggressionTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins should manage pet aggression",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsActiveConflagrationTargetTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsActiveConflagrationTargetTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins active conflagration target",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashVictimHasConflagrationTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashVictimHasConflagrationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins sacrolash victim has conflagration",
            SwpHelpers::SwpBosses::DATA_EREDAR_TWINS) {}

protected:
    bool IsActiveInEncounter() override;
};

// M'uru

class MuruHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    MuruHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru hunter should misdirect", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruEntropiusShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    MuruEntropiusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru entropius should be tanked", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruRangedShouldStackOrSpreadTrigger : public SunwellEncounterTrigger
{
public:
    MuruRangedShouldStackOrSpreadTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru ranged should stack or spread", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruShouldAssignDpsPriorityTrigger : public SunwellEncounterTrigger
{
public:
    MuruShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru should assign dps priority", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSentinelShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSentinelShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru void sentinel should be tanked", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruEntranceShouldBeGuardedTrigger : public SunwellEncounterTrigger
{
public:
    MuruEntranceShouldBeGuardedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru entrance should be guarded", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarkFiendsSpawnedTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarkFiendsSpawnedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru dark fiends spawned", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarknessIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarknessIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru darkness is active", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBerserkerHasFlurryTrigger : public SunwellEncounterTrigger
{
public:
    MuruBerserkerHasFlurryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru berserker has flurry", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageCastingFelFireballTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru fury mage casting fel fireball", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageHasSpellFuryTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageHasSpellFuryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru fury mage has spell fury", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru void spawn available for enslave", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public SunwellEncounterTrigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru warlock has enslaved void spawn", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidZoneIsNearTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidZoneIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru void zone is near", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarkFiendIsNearTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarkFiendIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru dark fiend is near", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruTheSingularityIsNearTrigger : public SunwellEncounterTrigger
{
public:
    MuruTheSingularityIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru the singularity is near", SwpHelpers::SwpBosses::DATA_MURU) {}

protected:
    bool IsActiveInEncounter() override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenHandsOfTheDeceiverAreActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenHandsOfTheDeceiverAreActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden hands of the deceiver are active",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenShouldHoldDpsAtStartTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenShouldHoldDpsAtStartTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden should hold dps at start", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenDpsHoldIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden dps hold is active", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden should be tanked", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden melee should split into two groups",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenRangedShouldKeepFormationTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenRangedShouldKeepFormationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden ranged should keep formation",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenFireBloomOnImmunityClassTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenFireBloomOnImmunityClassTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden fire bloom on immunity class",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenCastsDarknessOfAThousandSoulsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenCastsDarknessOfAThousandSoulsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden casts darkness of a thousand souls",
            SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenDragonOrbIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenDragonOrbIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden dragon orb is active", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotControlsDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenBotControlsDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden bot controls dragon", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenStaleRootAfterDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenStaleRootAfterDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden stale root after dragon", SwpHelpers::SwpBosses::DATA_KILJAEDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
