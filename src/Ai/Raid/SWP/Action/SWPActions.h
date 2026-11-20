/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPACTIONS_H
#define PLAYERBOTS_SWPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "EncounterHelpers.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

class Creature;

// General

class SunwellResetEncounterStatesAction : public Action
{
public:
    SunwellResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "sunwell reset encounter states") {}
    bool Execute(Event event) override;
};

class SunwellAssignClassRolesAction : public Action
{
public:
    SunwellAssignClassRolesAction(PlayerbotAI* botAI)
        : Action(botAI, "sunwell assign class roles") {}
    bool Execute(Event event) override;
};

class SunwellRemoveAuraAction : public Action
{
public:
    SunwellRemoveAuraAction(PlayerbotAI* botAI) : Action(botAI, "sunwell remove aura") {}
    bool Execute(Event event) override;
};

// Trash

class VolatileFiendInterceptAction : public AttackAction
{
public:
    VolatileFiendInterceptAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "volatile fiend intercept") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class VolatileFiendMoveAwayAction : public MovementAction
{
public:
    VolatileFiendMoveAwayAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "volatile fiend move away") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ApocalypseGuardAttackWithHolyMagicAction : public Action
{
public:
    ApocalypseGuardAttackWithHolyMagicAction(PlayerbotAI* botAI)
        : Action(botAI, "apocalypse guard attack with holy magic") {}
    bool Execute(Event event) override;
};

// Shared Bosses

class SunwellRemoveDebuffWithImmunityAction : public Action
{
public:
    SunwellRemoveDebuffWithImmunityAction(PlayerbotAI* botAI, std::string const name)
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class SunwellMisdirectToMainTankAction : public Action
{
public:
    SunwellMisdirectToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, SwpHelpers::SwpBosses boss)
        : Action(botAI, name), _boss(boss) {}
    bool Execute(Event event) override;

private:
    SwpHelpers::SwpBosses const _boss;
};

// Kalecgos

class KalecgosAnnounceBossHealthAction : public Action
{
public:
    KalecgosAnnounceBossHealthAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos announce boss health") {}
    bool Execute(Event event) override;
};

class KalecgosUpdateRaidAssignmentsAction : public Action
{
public:
    KalecgosUpdateRaidAssignmentsAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos update raid assignments") {}
    bool Execute(Event event) override;
};

class KalecgosStopAttackingForHoldAction : public Action
{
public:
    KalecgosStopAttackingForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos stop attacking for hold") {}
    bool Execute(Event event) override;
};

// Takes the target dps assist would take, without attacking it, for a bot in its non-combat engine.
class KalecgosSelectTargetForHoldAction : public Action
{
public:
    KalecgosSelectTargetForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos select target for hold") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class KalecgosSetPetStanceAction : public Action
{
public:
    KalecgosSetPetStanceAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos set pet stance") {}
    bool Execute(Event event) override;
    bool ResetPetStance();

private:
    std::vector<EncounterHelpers::SavedMinionStance> _previousReactStates;
};

class KalecgosSurfaceTankPositionDragonAction : public AttackAction
{
public:
    KalecgosSurfaceTankPositionDragonAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kalecgos surface tank position dragon") {}
    bool Execute(Event event) override;
};

class KalecgosEnterSpectralRiftAction : public MovementAction
{
public:
    KalecgosEnterSpectralRiftAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos enter spectral rift") {}
    bool Execute(Event event) override;

private:
    bool ShouldTankEnter();
};

class KalecgosSpreadRangedAction : public MovementAction
{
public:
    KalecgosSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos spread ranged") {}
    bool Execute(Event event) override;
    bool ResetInitialRangedPositionReached()
    {
        if (!_initialRangedPositionReached)
            return false;
        _initialRangedPositionReached = false;
        return true;
    }

private:
    bool _initialRangedPositionReached = false;
};

class KalecgosSathrovarrTankStandWithKalecAction : public MovementAction
{
public:
    KalecgosSathrovarrTankStandWithKalecAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos sathrovarr tank stand with kalec") {}
    bool Execute(Event event) override;
};

class KalecgosReturnToSpectralRealmGroundAction : public MovementAction
{
public:
    KalecgosReturnToSpectralRealmGroundAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos return to spectral realm ground") {}
    bool Execute(Event event) override;
};

// Brutallus

class BrutallusTanksPositionAndSwapAction : public AttackAction
{
public:
    BrutallusTanksPositionAndSwapAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "brutallus tanks position and swap") {}
    bool Execute(Event event) override;
    bool ResetMainTankPlacement()
    {
        if (!_mainTankInPlace && !_placementStartMs)
            return false;
        _mainTankInPlace = false;
        _placementStartMs = 0;
        return true;
    }

private:
    bool _mainTankInPlace = false;
    uint32 _placementStartMs = 0;
};

class BrutallusPositionMeleeAtRearCenterAction : public MovementAction
{
public:
    BrutallusPositionMeleeAtRearCenterAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus position melee at rear center") {}
    bool Execute(Event event) override;

private:
    bool TryGetBrutallusMeleePosition(
        Unit* brutallus, Player* mainTank, Player* assistTank,
        uint8 meleeIndex, Position& position);
};

class BrutallusPositionRangedInTwoGroupsAction : public MovementAction
{
public:
    BrutallusPositionRangedInTwoGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus position ranged in two groups") {}
    bool Execute(Event event) override;

private:
    bool StepToPosition(Position const& position, float arrivalDist);
};

class BrutallusIsolateBurnAction : public MovementAction
{
public:
    BrutallusIsolateBurnAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus isolate burn") {}
    bool Execute(Event event) override;

private:
    bool RemoveBurnWithCooldown();
    bool StepToPosition(Position const& position, float arrivalDist);
};

// Felmyst

class FelmystMainTankPositionBossOnGroundAction : public AttackAction
{
public:
    FelmystMainTankPositionBossOnGroundAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "felmyst main tank position boss on ground") {}
    bool Execute(Event event) override;
};

class FelmystRangedStackInThreeGroupsAction : public MovementAction
{
public:
    FelmystRangedStackInThreeGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst ranged stack in three groups") {}
    bool Execute(Event event) override;
};

class FelmystMeleeStackBehindBossAction : public MovementAction
{
public:
    FelmystMeleeStackBehindBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst melee stack behind boss") {}
    bool Execute(Event event) override;
};

class FelmystRemoveEncapsulateAction : public Action
{
public:
    FelmystRemoveEncapsulateAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst remove encapsulate") {}
    bool Execute(Event event) override;
};

class FelmystRunAwayFromEncapsulatedPlayerAction : public MovementAction
{
public:
    FelmystRunAwayFromEncapsulatedPlayerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst run away from encapsulated player") {}
    bool Execute(Event event) override;
};

class FelmystMassDispelGasNovaAction : public Action
{
public:
    FelmystMassDispelGasNovaAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst mass dispel gas nova") {}
    bool Execute(Event event) override;
};

class FelmystMoveIntoMassDispelReachAction : public MovementAction
{
public:
    FelmystMoveIntoMassDispelReachAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst move into mass dispel reach") {}
    bool Execute(Event event) override;
};

class FelmystMoveToAirGroupPositionAction : public MovementAction
{
public:
    FelmystMoveToAirGroupPositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst move to air group position") {}
    bool Execute(Event event) override;
};

class FelmystAvoidDemonicVaporAction : public MovementAction
{
public:
    FelmystAvoidDemonicVaporAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst avoid demonic vapor") {}
    bool Execute(Event event) override;
};

class FelmystKiteDemonicVaporAction : public MovementAction
{
public:
    FelmystKiteDemonicVaporAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst kite demonic vapor") {}
    bool Execute(Event event) override;
    bool ResetRouteProgress();

private:
    ObjectGuid _demonicVaporGuid;
    std::span<Position const> _route;
    size_t _routePointIndex = 0;
    // G1, which the first vapor's target passes through if the second vapor picks it again
    Position const* _throughPosition = nullptr;
    void StartRoute(Unit* felmyst, ObjectGuid const& demonicVaporGuid);
};

class FelmystMoveToSafeFogLaneAction : public MovementAction
{
public:
    FelmystMoveToSafeFogLaneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst move to safe fog lane") {}
    bool Execute(Event event) override;
};

class FelmystMeleeClearTargetAction : public Action
{
public:
    FelmystMeleeClearTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst melee clear target") {}
    bool Execute(Event event) override;
};

class FelmystKillCharmedPlayerAction : public AttackAction
{
public:
    FelmystKillCharmedPlayerAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "felmyst kill charmed player") {}
    bool Execute(Event event) override;
};

class FelmystManageLandingDpsTimerAction : public Action
{
public:
    FelmystManageLandingDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst manage landing dps timer") {}
    bool Execute(Event event) override;
};

class FelmystStopAttackingForLandingAction : public Action
{
public:
    FelmystStopAttackingForLandingAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst stop attacking for landing") {}
    bool Execute(Event event) override;
};

// Takes the target dps assist would take, without attacking it, for a bot in its non-combat engine.
class FelmystSelectTargetForHoldAction : public Action
{
public:
    FelmystSelectTargetForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst select target for hold") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

// Eredar Twins

class EredarTwinsJumpFromBalconyAction : public MovementAction
{
public:
    EredarTwinsJumpFromBalconyAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins jump from balcony") {}
    bool Execute(Event event) override;
};

class EredarTwinsAnnounceClassRolesAction : public Action
{
public:
    EredarTwinsAnnounceClassRolesAction(PlayerbotAI* botAI)
        : Action(botAI, "eredar twins announce class roles") {}
    bool Execute(Event event) override;
};

class EredarTwinsMisdirectToTanksAction : public Action
{
public:
    EredarTwinsMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "eredar twins misdirect to tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsPositionSacrolashTanksAction : public AttackAction
{
public:
    EredarTwinsPositionSacrolashTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins position sacrolash tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsAlythessTankMoveOutOfBlazeAction : public AttackAction
{
public:
    EredarTwinsAlythessTankMoveOutOfBlazeAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins alythess tank move out of blaze") {}
    bool Execute(Event event) override;
    bool ResetAlythessTankState() { return EncounterHelpers::ResetIfSet(_alythessTankStep); }

private:
    bool StepToPosition(Unit* alythess, Position const& position, float arrivalDist);

    // Unset until the tank first moves, when it starts from the position it landed on.
    std::optional<uint8> _alythessTankStep;
};

class EredarTwinsRangedStackAtBalconyEdgeAction : public MovementAction
{
public:
    EredarTwinsRangedStackAtBalconyEdgeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins ranged stack at balcony edge") {}
    bool Execute(Event event) override;
};

class EredarTwinsStackBehindAlythessAction : public MovementAction
{
public:
    EredarTwinsStackBehindAlythessAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins stack behind alythess") {}
    bool Execute(Event event) override;
};

class EredarTwinsStepAsideFromStackAction : public MovementAction
{
public:
    EredarTwinsStepAsideFromStackAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins step aside from stack") {}
    bool Execute(Event event) override;
};

class EredarTwinsDpsPrioritizeSacrolashAction : public AttackAction
{
public:
    EredarTwinsDpsPrioritizeSacrolashAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins dps prioritize sacrolash") {}
    bool Execute(Event event) override;
};

class EredarTwinsSetPetStanceAction : public Action
{
public:
    EredarTwinsSetPetStanceAction(PlayerbotAI* botAI)
        : Action(botAI, "eredar twins set pet stance") {}
    bool Execute(Event event) override;
    bool ResetPetStance();

private:
    std::vector<EncounterHelpers::SavedMinionStance> _previousReactStates;
};

class EredarTwinsConflagrationTargetMoveFromGroupAction : public MovementAction
{
public:
    EredarTwinsConflagrationTargetMoveFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins conflagration target move from group") {}
    bool Execute(Event event) override;
};

class EredarTwinsMoveAwayFromSacrolashVictimAction : public MovementAction
{
public:
    EredarTwinsMoveAwayFromSacrolashVictimAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins move away from sacrolash victim") {}
    bool Execute(Event event) override;
};

// M'uru

class MuruMisdirectEnemyToTankAction : public Action
{
public:
    MuruMisdirectEnemyToTankAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru misdirect enemy to tank") {}
    bool Execute(Event event) override;
};

class MuruMainTankPickUpEntropiusAction : public AttackAction
{
public:
    MuruMainTankPickUpEntropiusAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru main tank pick up entropius") {}
    bool Execute(Event event) override;
};

class MuruPositionRangedByPhaseAction : public MovementAction
{
public:
    MuruPositionRangedByPhaseAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru position ranged by phase") {}
    bool Execute(Event event) override;
    bool ResetEntropiusRangedPositionReached()
    {
        if (!_entropiusRangedPositionReached)
            return false;
        _entropiusRangedPositionReached = false;
        return true;
    }

private:
    bool _entropiusRangedPositionReached = false;
    bool TryGetEntropiusInitialRangedPosition(Position& position) const;
};

class MuruAssignDpsPriorityAction : public AttackAction
{
public:
    MuruAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru assign dps priority") {}
    bool Execute(Event event) override;

private:
    Unit* ResolveMuruDpsTarget(Unit* currentTarget);
};

class MuruKillDarkFiendsWithDispelAction : public Action
{
public:
    MuruKillDarkFiendsWithDispelAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru kill dark fiends with dispel") {}
    bool Execute(Event event) override;
};

class MuruTanksMoveSentinelToSafePositionAction : public AttackAction
{
public:
    MuruTanksMoveSentinelToSafePositionAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru tanks move sentinel to safe position") {}
    bool Execute(Event event) override;
};

class MuruSecondAssistTankGuardRangedAction : public MovementAction
{
public:
    MuruSecondAssistTankGuardRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru second assist tank guard ranged") {}
    bool Execute(Event event) override;
};

class MuruMeleeFleeTheDarknessAction : public MovementAction
{
public:
    MuruMeleeFleeTheDarknessAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru melee flee the darkness") {}
    bool Execute(Event event) override;

private:
    bool StepToPosition(Position const& position, float arrivalDistance);
};

class MuruCastStunOnBerserkerAction : public Action
{
public:
    MuruCastStunOnBerserkerAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru cast stun on berserker") {}
    bool Execute(Event event) override;
};

class MuruInterruptFelFireballAction : public Action
{
public:
    MuruInterruptFelFireballAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru interrupt fel fireball") {}
    bool Execute(Event event) override;

private:
    bool CastWarriorInterrupt(Unit* furyMage);
};

class MuruCastSpellStealOnSpellFuryAction : public Action
{
public:
    MuruCastSpellStealOnSpellFuryAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru cast spellsteal on spell fury") {}
    bool Execute(Event event) override;
};

class MuruWarlockEnslaveVoidSpawnAction : public Action
{
public:
    MuruWarlockEnslaveVoidSpawnAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru warlock enslave void spawn") {}
    bool Execute(Event event) override;
};

class MuruVoidSpawnCastShadowBoltVolleyAction : public Action
{
public:
    MuruVoidSpawnCastShadowBoltVolleyAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru void spawn cast shadow bolt volley") {}
    bool Execute(Event event) override;

private:
    Unit* GetControlledVoidSpawn() const;
    Unit* GetVoidSpawnVolleyPriorityTarget(Unit* voidSpawn) const;
};

class MuruMoveClearOfVoidZonesAndFiendsAction : public MovementAction
{
public:
    MuruMoveClearOfVoidZonesAndFiendsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru move clear of void zones and fiends") {}
    bool Execute(Event event) override;
};

class MuruEscapeTheSingularityAction : public MovementAction
{
public:
    MuruEscapeTheSingularityAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru escape the singularity") {}
    bool Execute(Event event) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenAnnounceDragonOrbUserAction : public Action
{
public:
    KiljaedenAnnounceDragonOrbUserAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden announce dragon orb user") {}
    bool Execute(Event event) override;
};

class KiljaedenControlHandsOfTheDeceiverAction : public Action
{
public:
    KiljaedenControlHandsOfTheDeceiverAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden control hands of the deceiver") {}
    bool Execute(Event event) override;

private:
    bool CastStunOnHand(Unit* hand);
    bool CastSilenceOnHand(Unit* hand);
};

class KiljaedenMarkHandOfTheDeceiverAction : public Action
{
public:
    KiljaedenMarkHandOfTheDeceiverAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden mark hand of the deceiver") {}
    bool Execute(Event event) override;
};

class KiljaedenMoveHolyPaladinIntoStunRangeAction : public MovementAction
{
public:
    KiljaedenMoveHolyPaladinIntoStunRangeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden move holy paladin into stun range") {}
    bool Execute(Event event) override;
};

class KiljaedenPositionAndMoveTanksAction : public AttackAction
{
public:
    KiljaedenPositionAndMoveTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kil'jaeden position and move tanks") {}
    bool Execute(Event event) override;

private:
    bool PickUpSinisterReflections(Creature* reflection);
};

class KiljaedenPositionMeleeAndAvoidArmageddonsAction : public MovementAction
{
public:
    KiljaedenPositionMeleeAndAvoidArmageddonsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden position melee and avoid armageddons") {}
    bool Execute(Event event) override;

private:
    bool TryGetMeleePosition(Position& position) const;
    bool TryAdjustMeleeForArmageddon(Position& position);
};

class KiljaedenPositionRangedAndAvoidArmageddonsAction : public MovementAction
{
public:
    KiljaedenPositionRangedAndAvoidArmageddonsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden position ranged and avoid armageddons") {}
    bool Execute(Event event) override;

private:
    bool TryGetRangedPosition(Position& position) const;
    bool TryAdjustRangedForArmageddon(Position& position);
};

class KiljaedenStackForShieldOfTheBlueAction : public MovementAction
{
public:
    KiljaedenStackForShieldOfTheBlueAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden stack for shield of the blue") {}
    bool Execute(Event event) override;
};

class KiljaedenUseDragonOrbAction : public MovementAction
{
public:
    KiljaedenUseDragonOrbAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden use dragon orb") {}
    bool Execute(Event event) override;
};

class KiljaedenDragonBuffAndProtectRaidAction : public Action
{
public:
    KiljaedenDragonBuffAndProtectRaidAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden dragon buff and protect raid") {}
    bool Execute(Event event) override;

private:
    bool ExecuteDuringDarknessOfAThousandSouls(Unit* kiljaeden, Unit* dragon);
    bool ExecuteOutsideDarknessOfAThousandSouls(Unit* dragon);
};

class KiljaedenReleaseStaleRootAction : public Action
{
public:
    KiljaedenReleaseStaleRootAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden release stale root") {}
    bool Execute(Event event) override;
};

#endif
