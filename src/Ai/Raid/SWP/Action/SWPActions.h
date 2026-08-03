/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPACTIONS_H
#define PLAYERBOTS_SWPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include <limits>
#include <vector>

// General

class SunwellPlateauResetEncounterStatesAction : public Action
{
public:
    SunwellPlateauResetEncounterStatesAction(
        PlayerbotAI* botAI) : Action(botAI, "sunwell plateau reset encounter states") {}
    bool Execute(Event event) override;
};

class SunwellPlateauRemoveProtectiveAuraAction : public Action
{
public:
    SunwellPlateauRemoveProtectiveAuraAction(
        PlayerbotAI* botAI) : Action(botAI, "sunwell plateau remove protective aura") {}
    bool Execute(Event event) override;
};

// Trash

class VolatileFiendKeepEnemyAwayFromGroupAction : public AttackAction
{
public:
    VolatileFiendKeepEnemyAwayFromGroupAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "volatile fiend keep enemy away from group") {}
    bool Execute(Event event) override;
};

class ApocalypseGuardAttackWithHolyMagicAction : public Action
{
public:
    ApocalypseGuardAttackWithHolyMagicAction(
        PlayerbotAI* botAI) : Action(botAI, "apocalypse guard attack with holy magic") {}
    bool Execute(Event event) override;
};

// Kalecgos

class KalecgosAnnounceBossHealthAction : public Action
{
public:
    KalecgosAnnounceBossHealthAction(
        PlayerbotAI* botAI) : Action(botAI, "kalecgos announce boss health") {}
    bool Execute(Event event) override;
};

class KalecgosTankPositionBossAction : public AttackAction
{
public:
    KalecgosTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kalecgos tank position boss") {}
    bool Execute(Event event) override;
};

class KalecgosEnterSpectralRiftAction : public MovementAction
{
public:
    KalecgosEnterSpectralRiftAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos enter spectral rift") {}
    bool Execute(Event event) override;

private:
    bool ShouldTankEnter();
};

class KalecgosDisperseRangedAction : public MovementAction
{
public:
    KalecgosDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos disperse ranged") {}
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

class KalecgosRemoveArcaneBuffetAction : public Action
{
public:
    KalecgosRemoveArcaneBuffetAction(
        PlayerbotAI* botAI) : Action(botAI, "kalecgos remove arcane buffet") {}
    bool Execute(Event event) override;
};

class KalecgosSathrovarrTankStandWithKalecAction : public MovementAction
{
public:
    KalecgosSathrovarrTankStandWithKalecAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos sathrovarr tank stand with kalec") {}
    bool Execute(Event event) override;
};

class KalecgosReturnToSpectralRealmGroundAction : public MovementAction
{
public:
    KalecgosReturnToSpectralRealmGroundAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos return to spectral realm ground") {}
    bool Execute(Event event) override;
};

// Brutallus

class BrutallusMisdirectBossToMainTankAction : public AttackAction
{
public:
    BrutallusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "brutallus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class BrutallusTanksHandleBossAction : public AttackAction
{
public:
    BrutallusTanksHandleBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "brutallus tanks handle boss") {}
    bool Execute(Event event) override;
    bool ResetInitialPositionReached()
    {
        if (!_mainTankInitialPositionReached)
            return false;
        _mainTankInitialPositionReached = false;
        return true;
    }

private:
    bool _mainTankInitialPositionReached = false;
};

class BrutallusPositionMeleeAction : public MovementAction
{
public:
    BrutallusPositionMeleeAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "brutallus position melee") {}
    bool Execute(Event event) override;

private:
    bool TryGetBrutallusMeleePosition(
        Unit* brutallus, Player* mainTank, Player* assistTank,
        uint8 meleeIndex, Position& position);
};

class BrutallusPositionRangedAction : public MovementAction
{
public:
    BrutallusPositionRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "brutallus position ranged") {}
    bool Execute(Event event) override;
};

class BrutallusHandleBurnAction : public MovementAction
{
public:
    BrutallusHandleBurnAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "brutallus handle burn") {}
    bool Execute(Event event) override;

private:
    bool RemoveBurnWithCooldown();
};

// Felmyst

class FelmystMisdirectBossToMainTankAction : public AttackAction
{
public:
    FelmystMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "felmyst misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class FelmystMainTankPositionBossOnGroundAction : public AttackAction
{
public:
    FelmystMainTankPositionBossOnGroundAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "felmyst main tank position boss on ground") {}
    bool Execute(Event event) override;
};

class FelmystPositionRangedOnGroundAction : public MovementAction
{
public:
    FelmystPositionRangedOnGroundAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst position ranged on ground") {}
    bool Execute(Event event) override;
};

class FelmystPositionMeleeOnGroundAction : public MovementAction
{
public:
    FelmystPositionMeleeOnGroundAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst position melee on ground") {}
    bool Execute(Event event) override;
};

class FelmystRemoveEncapsulateAction : public Action
{
public:
    FelmystRemoveEncapsulateAction(
        PlayerbotAI* botAI) : Action(botAI, "felmyst remove encapsulate") {}
    bool Execute(Event event) override;
};

class FelmystRunAwayFromEncapsulatedPlayerAction : public MovementAction
{
public:
    FelmystRunAwayFromEncapsulatedPlayerAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst run away from encapsulated player") {}
    bool Execute(Event event) override;
};

class FelmystMassDispelGasNovaAction : public Action
{
public:
    FelmystMassDispelGasNovaAction(
        PlayerbotAI* botAI) : Action(botAI, "felmyst mass dispel gas nova") {}
    bool Execute(Event event) override;
};

class FelmystAvoidDemonicVaporAction : public MovementAction
{
public:
    FelmystAvoidDemonicVaporAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst avoid demonic vapor") {}
    bool Execute(Event event) override;

private:
    void AnnounceFlightLeader(Player* leader);
    bool MoveAwayFromVapor(bool unrestricted = false);
    bool MoveToFlightLeader(Player* leader);

    ObjectGuid _announcedFlightLeaderGuid;
};

class FelmystKiteDemonicVaporAction : public MovementAction
{
public:
    FelmystKiteDemonicVaporAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst kite demonic vapor") {}
    bool Execute(Event event) override;
};

class FelmystMoveToSafeFogLaneAction : public MovementAction
{
public:
    FelmystMoveToSafeFogLaneAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst move to safe fog lane") {}
    bool Execute(Event event) override;

private:
    Position _fogCrateStuckDestination;
    float _fogCrateStuckNearestDist = std::numeric_limits<float>::max();
    uint32 _fogCrateStuckSampleMs = 0;
    bool TryTeleportStuckBotOntoCrate(Position const& destination);
};

class FelmystMeleeClearTargetAction : public Action
{
public:
    FelmystMeleeClearTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "felmyst melee clear target") {}
    bool Execute(Event event) override;
};

class FelmystKillCharmedPlayerAction : public AttackAction
{
public:
    FelmystKillCharmedPlayerAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "felmyst kill charmed player") {}
    bool Execute(Event event) override;
};

class FelmystManageLandingDpsTimerAction : public Action
{
public:
    FelmystManageLandingDpsTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "felmyst manage landing dps timer") {}
    bool Execute(Event event) override;
};

// Eredar Twins

class EredarTwinsMeleeJumpDownFromBalconyAction : public MovementAction
{
public:
    EredarTwinsMeleeJumpDownFromBalconyAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins melee jump down from balcony") {}
    bool Execute(Event event) override;
};

class EredarTwinsMisdirectBossesToTanksAction : public AttackAction
{
public:
    EredarTwinsMisdirectBossesToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins misdirect bosses to tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction : public AttackAction
{
public:
    EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins main and second assist tanks position sacrolash") {}
    bool Execute(Event event) override;
};

class EredarTwinsFirstAssistTankMoveOutOfBlazeAction : public AttackAction
{
public:
    EredarTwinsFirstAssistTankMoveOutOfBlazeAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins first assist tank move out of blaze") {}
    bool Execute(Event event) override;
    bool ResetAlythessTankStep()
    {
        if (!_alythessTankStep)
            return false;
        _alythessTankStep = 0;
        return true;
    }

private:
    uint8 _alythessTankStep = 0;
};

class EredarTwinsPositionRangedAction : public MovementAction
{
public:
    EredarTwinsPositionRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins position ranged") {}
    bool Execute(Event event) override;
};

class EredarTwinsStackInRoomCenterAction : public MovementAction
{
public:
    EredarTwinsStackInRoomCenterAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins stack in room center") {}
    bool Execute(Event event) override;
};

class EredarTwinsRemoveFlameSearAction : public Action
{
public:
    EredarTwinsRemoveFlameSearAction(
        PlayerbotAI* botAI) : Action(botAI, "eredar twins remove flame sear") {}
    bool Execute(Event event) override;
};

class EredarTwinsDpsPrioritizeLadySacrolashAction : public AttackAction
{
public:
    EredarTwinsDpsPrioritizeLadySacrolashAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins dps prioritize lady sacrolash") {}
    bool Execute(Event event) override;
};

class EredarTwinsConflagratedBotMoveFromGroupAction : public MovementAction
{
public:
    EredarTwinsConflagratedBotMoveFromGroupAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins conflagrated bot move from group") {}
    bool Execute(Event event) override;
};

class EredarTwinsMoveFromConflagSacrolashVictimAction : public MovementAction
{
public:
    EredarTwinsMoveFromConflagSacrolashVictimAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins move from conflag sacrolash victim") {}
    bool Execute(Event event) override;
};

// M'uru

class MuruMisdirectEnemiesToTanksAction : public AttackAction
{
public:
    MuruMisdirectEnemiesToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "m'uru misdirect enemies to tanks") {}
    bool Execute(Event event) override;
};

class MuruMainTankPickUpEntropiusAction : public AttackAction
{
public:
    MuruMainTankPickUpEntropiusAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "m'uru main tank pick up entropius") {}
    bool Execute(Event event) override;
};

class MuruPositionRangedAction : public MovementAction
{
public:
    MuruPositionRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru position ranged") {}
    bool Execute(Event event) override;

private:
    bool _entropiusRangedPositionReached = false;
    bool TryGetEntropiusInitialRangedPosition(Position& position) const;
};

class MuruSetDpsPriorityAction : public AttackAction
{
public:
    MuruSetDpsPriorityAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "m'uru set dps priority") {}
    bool Execute(Event event) override;

private:
    Unit* ResolveMuruDpsTarget(Unit*& currentTarget);
    Unit* SelectMuruEncounterTarget(
        Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates) const;
};

class MuruKillDarkFiendsWithDispelAction : public Action
{
public:
    MuruKillDarkFiendsWithDispelAction(
        PlayerbotAI* botAI) : Action(botAI, "m'uru kill dark fiends with dispel") {}
    bool Execute(Event event) override;
};

class MuruDontTouchTheDarkFiendAction : public MovementAction
{
public:
    MuruDontTouchTheDarkFiendAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru don't touch the dark fiend") {}
    bool Execute(Event event) override;
};

class MuruTanksMoveSentinelToSafePositionAction : public AttackAction
{
public:
    MuruTanksMoveSentinelToSafePositionAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "m'uru tanks move sentinel to safe position") {}
    bool Execute(Event event) override;

private:
    Position const& GetAssignedVoidSentinelTankPosition(Unit* voidSentinel);
};

class MuruSecondAssistTankGuardRangedAction : public MovementAction
{
public:
    MuruSecondAssistTankGuardRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru second assist tank guard ranged") {}
    bool Execute(Event event) override;
};

class MuruFleeTheDarknessAction : public MovementAction
{
public:
    MuruFleeTheDarknessAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru flee the darkness") {}
    bool Execute(Event event) override;
};

class MuruFleeFromSingularityAction : public MovementAction
{
public:
    MuruFleeFromSingularityAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru flee from singularity") {}
    bool Execute(Event event) override;
};

class MuruCastStunOnShadowswordBerserkerAction : public Action
{
public:
    MuruCastStunOnShadowswordBerserkerAction(
        PlayerbotAI* botAI) : Action(botAI, "m'uru cast stun on shadowsword berserker") {}
    bool Execute(Event event) override;
};

class MuruInterruptFelFireballAction : public Action
{
public:
    MuruInterruptFelFireballAction(
        PlayerbotAI* botAI) : Action(botAI, "m'uru interrupt fel fireball") {}
    bool Execute(Event event) override;
};

class MuruCastSpellStealOnSpellFuryAction : public Action
{
public:
    MuruCastSpellStealOnSpellFuryAction(
        PlayerbotAI* botAI) : Action(botAI, "m'uru cast spellsteal on spell fury") {}
    bool Execute(Event event) override;
};

class MuruWarlockEnslaveVoidSpawnAction : public MovementAction
{
public:
    MuruWarlockEnslaveVoidSpawnAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "m'uru warlock enslave void spawn") {}
    bool Execute(Event event) override;
};

class MuruEnslavedVoidSpawnAttackAction : public Action
{
public:
    MuruEnslavedVoidSpawnAttackAction(
        PlayerbotAI* botAI, std::string const name = "m'uru enslaved void spawn attack")
        : Action(botAI, name) {}

protected:
    Unit* GetControlledVoidSpawn() const;
    bool CommandControlledCreatureToAttack(Unit* controlled, Unit* target) const;
    Unit* GetVoidSpawnVolleyPriorityTarget() const;
};

class MuruEnslavedVoidSpawnCastShadowBoltVolleyAction : public MuruEnslavedVoidSpawnAttackAction
{
public:
    MuruEnslavedVoidSpawnCastShadowBoltVolleyAction(
        PlayerbotAI* botAI) :
        MuruEnslavedVoidSpawnAttackAction(botAI, "m'uru enslaved void spawn cast shadow bolt volley") {}
    bool Execute(Event event) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenAnnounceDragonOrbUserAction : public Action
{
public:
    KiljaedenAnnounceDragonOrbUserAction(
        PlayerbotAI* botAI) : Action(botAI, "kil'jaeden announce dragon orb user") {}
    bool Execute(Event event) override;
};

class KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction : public AttackAction
{
public:
    KiljaedenMarkAndPrioritizeHandsOfTheDeceiverAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kil'jaeden mark and prioritize hands of the deceiver") {}
    bool Execute(Event event) override;

private:
    bool ExecuteTankHandAssignment(
        std::vector<Unit*> const& hands,
        Player* mainTank, Player* firstAssistTank, Player* secondAssistTank);
};

class KiljaedenStunHandsOfTheDeceiverAction : public Action
{
public:
    KiljaedenStunHandsOfTheDeceiverAction(
        PlayerbotAI* botAI) : Action(botAI, "kil'jaeden stun hands of the deceiver") {}
    bool Execute(Event event) override;

private:
    bool CastStunOnHand(Unit* hand);
    bool CastSilenceOnHand(Unit* hand);
};

class KiljaedenPositionTanksAction : public AttackAction
{
public:
    KiljaedenPositionTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kil'jaeden position tanks") {}
    bool Execute(Event event) override;
};

class KiljaedenPositionMeleeAction : public MovementAction
{
public:
    KiljaedenPositionMeleeAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kil'jaeden position melee") {}
    bool Execute(Event event) override;

private:
    bool TryGetPosition(Position& position) const;
    bool TryAdjustForArmageddon(Position& position);
};

class KiljaedenPositionRangedAction : public MovementAction
{
public:
    KiljaedenPositionRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kil'jaeden position ranged") {}
    bool Execute(Event event) override;

private:
    bool TryGetPosition(Position& position) const;
    bool TryAdjustForArmageddon(Position& position);
};

class KiljaedenRemoveFireBloomAction : public Action
{
public:
    KiljaedenRemoveFireBloomAction(
        PlayerbotAI* botAI) : Action(botAI, "kil'jaeden remove fire bloom") {}
    bool Execute(Event event) override;
};

class KiljaedenStackForShieldOfTheBlueAction : public MovementAction
{
public:
    KiljaedenStackForShieldOfTheBlueAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kil'jaeden stack for shield of the blue") {}
    bool Execute(Event event) override;
};

class KiljaedenUseDragonOrbAction : public MovementAction
{
public:
    KiljaedenUseDragonOrbAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kil'jaeden use dragon orb") {}
    bool Execute(Event event) override;
};

class KiljaedenReleaseStaleRootAction : public Action
{
public:
    KiljaedenReleaseStaleRootAction(
        PlayerbotAI* botAI) : Action(botAI, "kil'jaeden release stale root") {}
    bool Execute(Event event) override;
};

class KiljaedenControlDragonAction : public Action
{
public:
    KiljaedenControlDragonAction(
        PlayerbotAI* botAI) : Action(botAI, "kil'jaeden control dragon") {}
    bool Execute(Event event) override;

private:
    bool ExecuteDuringDarknessOfAThousandSouls(Unit* kiljaeden, Unit* dragon);
    bool ExecuteOutsideDarknessOfAThousandSouls(Unit* dragon);
};

#endif
