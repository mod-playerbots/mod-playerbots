/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTACTIONS_H
#define PLAYERBOTS_BTACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "BTHelpers.h"
#include "MovementActions.h"
#include "Unit.h"
#include <string>
#include <unordered_map>
#include <vector>

// Shared

class BlackTempleResetEncounterStatesAction : public Action
{
public:
    BlackTempleResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "black temple reset encounter states") {}
    bool Execute(Event event) override;
};

class BlackTempleAssignClassRolesAction : public Action
{
public:
    BlackTempleAssignClassRolesAction(PlayerbotAI* botAI)
        : Action(botAI, "black temple assign class roles") {}
    bool Execute(Event event) override;
};

// Used for High Warlord Naj'entus, Supremus, Teron Gorefiend, Gurtogg Bloodboil and Mother
// Shahraz.
class BlackTempleMisdirectToMainTankAction : public Action
{
public:
    BlackTempleMisdirectToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, BtHelpers::BtBosses boss)
        : Action(botAI, name), _boss(boss) {}
    bool Execute(Event event) override;

private:
    BtHelpers::BtBosses const _boss;
};

// Used for High Warlord Naj'entus, Teron Gorefiend and Gurtogg Bloodboil, including his Fel Rage
// target. The boss's victim walks it to the position. With _allTanks, every tank does while the
// boss is on any tank. A tank walks only while in melee range; a non-tank leads from anywhere.
class BlackTemplePositionBossAction : public AttackAction
{
public:
    BlackTemplePositionBossAction(
        PlayerbotAI* botAI, std::string const& name, BtHelpers::BtBosses boss,
        Position const& position, bool allTanks = false)
        : AttackAction(botAI, name), _boss(boss), _position(position), _allTanks(allTanks) {}
    bool Execute(Event event) override;

private:
    BtHelpers::BtBosses const _boss;
    Position const _position;
    bool const _allTanks;
};

// Used for High Warlord Naj'entus, Supremus, and Illidari Council.
class BlackTempleSpreadRangedAction : public MovementAction
{
public:
    BlackTempleSpreadRangedAction(PlayerbotAI* botAI, std::string const& name, float distance)
        : MovementAction(botAI, name), _distance(distance) {}
    bool Execute(Event event) override;

private:
    float const _distance;
};

// Used for Reliquary of Souls and Illidari Council.
class BlackTempleCastSpellReflectionAction : public Action
{
public:
    BlackTempleCastSpellReflectionAction(PlayerbotAI* botAI, std::string const& name)
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Trash

// Damage on the Sister of Pleasure is split evenly with her Sister of Pain, and they have equal
// health, so killing Pleasure kills both without anyone hitting Pain through Shell of Pain. The
// mechanic tracker skulls Pleasure, and every other non-tank drops Pain.
class SisterOfPleasureFocusDpsAction : public Action
{
public:
    SisterOfPleasureFocusDpsAction(PlayerbotAI* botAI)
        : Action(botAI, "sister of pleasure focus dps") {}
    bool Execute(Event event) override;
};

class ShadowmoonReaverStopWandAction : public Action
{
public:
    ShadowmoonReaverStopWandAction(PlayerbotAI* botAI)
        : Action(botAI, "shadowmoon reaver stop wand") {}
    bool Execute(Event event) override;
};

// Imps, water elementals, succubi and felhunters are kept off Reavers while anything else can be
// attacked. With only Reavers left, the pet waits passive by its owner while one is unsafe, and
// is sent onto them otherwise.
class ShadowmoonReaverControlCasterPetAction : public Action
{
public:
    ShadowmoonReaverControlCasterPetAction(PlayerbotAI* botAI)
        : Action(botAI, "shadowmoon reaver control caster pet") {}
    bool Execute(Event event) override;

private:
    bool RestoreReactState(Guardian* pet);

    ReactStates _previousReactState = REACT_DEFENSIVE;
    // Each summon is a new pet with a new GUID, so a stance saved for an earlier pet is dropped.
    ObjectGuid _passivePet;
};

// High Warlord Naj'entus

class HighWarlordNajentusAssignSpineRemoverAction : public Action
{
public:
    HighWarlordNajentusAssignSpineRemoverAction(PlayerbotAI* botAI)
        : Action(botAI, "high warlord naj'entus assign spine remover") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (_spineGuid.IsEmpty() && !_usedSpine && !_reactionStartTime && !_reactionDelay)
            return false;

        _spineGuid.Clear();
        _usedSpine = false;
        _reactionStartTime = 0;
        _reactionDelay = 0;
        return true;
    }

private:
    ObjectGuid _spineGuid;
    bool _usedSpine = false;
    uint32 _reactionStartTime = 0;
    uint32 _reactionDelay = 0;
};

class HighWarlordNajentusAssignSpineThrowerAction : public Action
{
public:
    HighWarlordNajentusAssignSpineThrowerAction(PlayerbotAI* botAI)
        : Action(botAI, "high warlord naj'entus assign spine thrower") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (!_throwDelay)
            return false;

        _throwDelay = 0;
        return true;
    }

private:
    uint32 _throwDelay = 0;
};

// Supremus

// Each step, the fixated bot takes the heading he would take longest to catch it on, stopped by
// walls, the open area's edge or fire. It runs through fire only when that is the one way to stay
// out of his reach.
class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (_lastAngle == 0.0f && !_lastStepTime)
            return false;

        _lastAngle = 0.0f;
        _lastStepTime = 0;
        return true;
    }

private:
    struct KitePlan
    {
        float angle;
        float runDistance;
        float fireExitDistance;
        float turn;
        float timeUntilCaught;
    };

    float GetCollisionFreeDistance(float angle, float maxDistance);
    float SimulateChase(
        Unit* supremus, float timeRemaining, float catchDistance, float angle, float runDistance);

    float _lastAngle = 0.0f;
    uint32 _lastStepTime = 0;
};

// Runs straight to the nearest point a yard clear of the fire. A bot not yet burning takes only a
// route that stays out of the fire, and otherwise waits at its edge; a burning bot takes the route
// with the least fire on it.
class SupremusMoveAwayFromFireAction : public MovementAction
{
public:
    SupremusMoveAwayFromFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "supremus move away from fire") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (!_lastFailedSearchTime)
            return false;

        _lastFailedSearchTime = 0;
        return true;
    }

private:
    bool FindSafestNearbyPosition(
        std::vector<BtHelpers::SupremusHazard> const& hazards, Position& destination);
    bool CanRunStraightTo(Position const& destination);

    uint32 _lastFailedSearchTime = 0;
};

// Steps toward the target, turning as little as it must to keep each step clear of fire, so the
// bot slides round a volcano or a trail in its way. With nothing clear it waits.
class SupremusReachAroundFireAction : public MovementAction
{
public:
    SupremusReachAroundFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "supremus reach around fire") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (_lastSide == 1 && !_lastStepTime)
            return false;

        _lastSide = 1;
        _lastStepTime = 0;
        return true;
    }

private:
    int8 _lastSide = 1;
    uint32 _lastStepTime = 0;
};

// Shade of Akama

// Channelers stand on a platform out of sight from below, and Attack refuses a target out of
// sight, so a bot walks the path toward its target until it can see it.
class ShadeOfAkamaAttackChannelersAndSorcerersAction : public AttackAction
{
public:
    ShadeOfAkamaAttackChannelersAndSorcerersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "shade of akama attack channelers and sorcerers") {}
    bool Execute(Event event) override;
};

// Teron Gorefiend

class TeronGorefiendPositionRangedOnBalconyAction : public MovementAction
{
public:
    TeronGorefiendPositionRangedOnBalconyAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend position ranged on balcony") {}
    bool Execute(Event event) override;
};

class TeronGorefiendAvoidShadowOfDeathAction : public Action
{
public:
    TeronGorefiendAvoidShadowOfDeathAction(PlayerbotAI* botAI)
        : Action(botAI, "teron gorefiend avoid shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMoveToCornerToDieAction : public MovementAction
{
public:
    TeronGorefiendMoveToCornerToDieAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend move to corner to die") {}
    bool Execute(Event event) override;
};

class TeronGorefiendDestroyShadowyConstructsAction : public MovementAction
{
public:
    TeronGorefiendDestroyShadowyConstructsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend destroy shadowy constructs") {}
    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetConstructs(Unit* gorefiend) const;
    Unit* GetLeadConstruct(Unit* gorefiend, std::vector<Unit*> const& constructs) const;
    Unit* GetLanceTarget(Unit* gorefiend, std::vector<Unit*> const& constructs) const;
};

// Gurtogg Bloodboil

class GurtoggBloodboilRotateRangedGroupsAction : public MovementAction
{
public:
    GurtoggBloodboilRotateRangedGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil rotate ranged groups") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilMeleeMoveBehindAction : public MovementAction
{
public:
    GurtoggBloodboilMeleeMoveBehindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil melee move behind") {}
    bool Execute(Event event) override;
};

// Steps sideways off his line to the Fel Rage target, the axis of Arcing Smash, so a bot ahead
// of a walking target isn't pushed along in front of it.
class GurtoggBloodboilRangedMoveOutOfFrontalConeAction : public MovementAction
{
public:
    GurtoggBloodboilRangedMoveOutOfFrontalConeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil ranged move out of frontal cone") {}
    bool Execute(Event event) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsMisdirectToMainTankAction : public Action
{
public:
    ReliquaryOfSoulsMisdirectToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "reliquary of souls misdirect to main tank") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsAdjustDistanceFromSufferingAction : public MovementAction
{
public:
    ReliquaryOfSoulsAdjustDistanceFromSufferingAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "reliquary of souls adjust distance from suffering") {}
    bool Execute(Event event) override;

private:
    bool MoveToDistanceFromSuffering(Unit* suffering, float distance);
    bool RangedMoveAwayFromBoss(Unit* suffering);
};

class ReliquaryOfSoulsHealersDpsSufferingAction : public Action
{
public:
    ReliquaryOfSoulsHealersDpsSufferingAction(PlayerbotAI* botAI)
        : Action(botAI, "reliquary of souls healers dps suffering") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
{
public:
    ReliquaryOfSoulsSpellstealRuneShieldAction(PlayerbotAI* botAI)
        : Action(botAI, "reliquary of souls spellsteal rune shield") {}
    bool Execute(Event event) override;
};

// Mother Shahraz

class MotherShahrazTanksPositionBossUnderPillarAction : public AttackAction
{
public:
    MotherShahrazTanksPositionBossUnderPillarAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "mother shahraz tanks position boss under pillar") {}
    bool Execute(Event event) override;
};

class MotherShahrazMeleeDpsWaitAtSafePositionAction : public MovementAction
{
public:
    MotherShahrazMeleeDpsWaitAtSafePositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz melee dps wait at safe position") {}
    bool Execute(Event event) override;
};

class MotherShahrazPositionRangedUnderStatueAction : public MovementAction
{
public:
    MotherShahrazPositionRangedUnderStatueAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz position ranged under statue") {}
    bool Execute(Event event) override;
};

// Holders run away from the ranged position, in GUID order: with two, one to each side; with
// three, the first straight away and the other two to each side.
class MotherShahrazBreakFatalAttractionAction : public MovementAction
{
public:
    MotherShahrazBreakFatalAttractionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz break fatal attraction") {}
    bool Execute(Event event) override;

private:
    std::vector<Player*> GetAttractedPlayers();
};

// Illidari Council

// Hunter bots in group order: the first, and any after the fourth, misdirect Zerevor to the mage
// tank; the second Malande to the first assist tank; the third Gathios to the main tank; the
// fourth Veras to the second assist tank.
class IllidariCouncilMisdirectToTanksAction : public Action
{
public:
    IllidariCouncilMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council misdirect to tanks") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMainTankPositionGathiosAction : public AttackAction
{
public:
    IllidariCouncilMainTankPositionGathiosAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council main tank position gathios") {}
    bool Execute(Event event) override;
    bool ResetTankStep()
    {
        if (_tankStep == 0)
            return false;

        _tankStep = 0;
        return true;
    }

private:
    uint8 _tankStep = 0;
};

class IllidariCouncilInterruptCircleOfHealingAction : public Action
{
public:
    IllidariCouncilInterruptCircleOfHealingAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council interrupt circle of healing") {}
    bool Execute(Event event) override;

private:
    bool CastWarriorInterrupt(Unit* malande);
};

// Leaves a council patch only for a position from which it can still hit Malande; otherwise it
// stays.
class IllidariCouncilFirstAssistTankFocusMalandeAction : public AttackAction
{
public:
    IllidariCouncilFirstAssistTankFocusMalandeAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council first assist tank focus malande") {}
    bool Execute(Event event) override;
};

class IllidariCouncilSecondAssistTankPositionVerasAction : public AttackAction
{
public:
    IllidariCouncilSecondAssistTankPositionVerasAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council second assist tank position veras") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMageTankPositionZerevorAction : public AttackAction
{
public:
    IllidariCouncilMageTankPositionZerevorAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council mage tank position zerevor") {}
    bool Execute(Event event) override;
    bool ResetTankStep()
    {
        if (_tankStep == 0)
            return false;

        _tankStep = 0;
        return true;
    }

private:
    uint8 _tankStep = 0;
};

class IllidariCouncilPositionMageTankHealerAction : public MovementAction
{
public:
    IllidariCouncilPositionMageTankHealerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidari council position mage tank healer") {}
    bool Execute(Event event) override;
};

class IllidariCouncilCommandPetTargetAction : public AttackAction
{
public:
    IllidariCouncilCommandPetTargetAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council command pet target") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAssignDpsTargetsAction : public AttackAction
{
public:
    IllidariCouncilAssignDpsTargetsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council assign dps targets") {}
    bool Execute(Event event) override;

private:
    bool ShouldAttackMalande(Unit* malande, Unit* gathios, Unit* zerevor);
};

class IllidariCouncilManageDpsTimerAction : public Action
{
public:
    IllidariCouncilManageDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council manage dps timer") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAnnounceClassRolesAction : public Action
{
public:
    IllidariCouncilAnnounceClassRolesAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council announce class roles") {}
    bool Execute(Event event) override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormragePositionOnGrateDuringSpeechAction : public MovementAction
{
public:
    IllidanStormragePositionOnGrateDuringSpeechAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage position on grate during speech") {}
    bool Execute(Event event) override;
};

class IllidanStormrageMisdirectToTanksAction : public Action
{
public:
    IllidanStormrageMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage misdirect to tanks") {}
    bool Execute(Event event) override;

private:
    bool TryMisdirectToFlameTanks(Group* group);
    bool TryMisdirectToWarlockTank(Unit* illidan);
};

class IllidanStormrageMainTankRepositionBossAction : public AttackAction
{
public:
    IllidanStormrageMainTankRepositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage main tank reposition boss") {}
    bool Execute(Event event) override;
    bool ResetEncounterState()
    {
        if (!_phaseStartTime && _lastFlameCrash.IsEmpty() && _leadPosition == Position() &&
            _lead == BtHelpers::IllidanTankLead::None && !_isOpeningLeadDone)
        {
            return false;
        }

        _phaseStartTime = 0;
        _lastFlameCrash.Clear();
        _leadPosition = Position();
        _lead = BtHelpers::IllidanTankLead::None;
        _isOpeningLeadDone = false;
        return true;
    }

private:
    bool FindDemonFormPosition(Unit const& illidan, Position& position);
    bool FindRingPosition(Unit const& illidan, BtHelpers::IllidanPhase phase, Position& position);
    void StartRingLead(Unit const& illidan, BtHelpers::IllidanPhase phase);
    void RouteRoundIllidan(Unit const& illidan, Position& position);

    uint32 _phaseStartTime = 0;
    ObjectGuid _lastFlameCrash;
    Position _leadPosition;
    BtHelpers::IllidanTankLead _lead = BtHelpers::IllidanTankLead::None;
    bool _isOpeningLeadDone = false;
};

class IllidanStormrageIsolateBotWithInfectionAction : public MovementAction
{
public:
    IllidanStormrageIsolateBotWithInfectionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage isolate bot with infection") {}
    bool Execute(Event event) override;

private:
    bool HunterSetFrostTrap();
};

class IllidanStormrageTrackInfectionsAction : public Action
{
public:
    IllidanStormrageTrackInfectionsAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage track infections") {}
    bool Execute(Event event) override;

private:
    bool FindTrapHunter(
        Player& infected, Position const& infectionPosition,
        std::unordered_map<ObjectGuid, BtHelpers::IllidanInfection> const& infections,
        ObjectGuid& hunter);
};

class IllidanStormrageAssistTanksHandleFlamesAction : public AttackAction
{
public:
    IllidanStormrageAssistTanksHandleFlamesAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage assist tanks handle flames") {}
    bool Execute(Event event) override;
    bool ResetTankStep()
    {
        if (_tankStep == 0)
            return false;

        _tankStep = 0;
        return true;
    }

private:
    bool RepositionToAvoidEyeBlast(BtHelpers::EyeBlastDangerArea const& dangerArea);
    // While the beam's head is near, dangerArea is its path ahead, which the steps keep out of.
    bool RepositionToAvoidBlaze(Unit* flame, BtHelpers::EyeBlastDangerArea const* dangerArea);

    uint8 _tankStep = 0;
};

class IllidanStormrageControlPetAggressionAction : public Action
{
public:
    IllidanStormrageControlPetAggressionAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage control pet aggression") {}
    bool Execute(Event event) override;

private:
    ReactStates _previousReactState = REACT_DEFENSIVE;
    bool _setPassive = false;
};

class IllidanStormrageCommandPetAttackAddAction : public Action
{
public:
    IllidanStormrageCommandPetAttackAddAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage command pet attack add") {}
    bool Execute(Event event) override;
};

class IllidanStormragePositionAboveGrateAction : public MovementAction
{
public:
    IllidanStormragePositionAboveGrateAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage position above grate") {}
    bool Execute(Event event) override;
};

class IllidanStormrageClearDebuffWithImmunityAction : public Action
{
public:
    IllidanStormrageClearDebuffWithImmunityAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage clear debuff with immunity") {}
    bool Execute(Event event) override;
};

class IllidanStormrageCancelImmunityAction : public Action
{
public:
    IllidanStormrageCancelImmunityAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage cancel immunity") {}
    bool Execute(Event event) override;
};

class IllidanStormrageRecallTotemsAction : public Action
{
public:
    IllidanStormrageRecallTotemsAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage recall totems") {}
    bool Execute(Event event) override;
};

// Everyone but the main tank waits at the landing stack, out of his way to the ring.
class IllidanStormragePositionForLandingAction : public MovementAction
{
public:
    IllidanStormragePositionForLandingAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage position for landing") {}
    bool Execute(Event event) override;
};

class IllidanStormrageMeleeStopAttackingAction : public Action
{
public:
    IllidanStormrageMeleeStopAttackingAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage melee stop attacking") {}
    bool Execute(Event event) override;
};

// The target is kept: the hold keeps the shot or channel from starting again.
class IllidanStormrageStopShootingAction : public Action
{
public:
    IllidanStormrageStopShootingAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage stop shooting") {}
    bool Execute(Event event) override;
};

// Phase 1: stacked on the grate center. Phases 3 and 5: in the grate slots by roster, healers
// first, the warlock tank in the one nearest its Demon Form position, and ranged DPS no farther
// than their spell reach of him. Phase 4: healers and ranged DPS each in their own slots, by
// roster.
class IllidanStormrageSpreadRangedAction : public MovementAction
{
public:
    IllidanStormrageSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage spread ranged") {}
    bool Execute(Event event) override;

private:
    bool FindDemonFormSlot(Unit const& illidan, Position& position);
};

// Short steps toward the Demon, so the bot stops once in reach, never into Aura of Dread or near
// Shadow Blast round his victim. Spread ranged brings it back once the Demon is dead.
class IllidanStormrageReachShadowDemonAction : public MovementAction
{
public:
    IllidanStormrageReachShadowDemonAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage reach shadow demon") {}
    bool Execute(Event event) override;
};

// Melee and tanks but the main tank, which has its own positions, wait in their Demon Form slots,
// facing him, and leave them only for a Shadow Demon.
class IllidanStormrageMeleeStayAwayFromDemonFormAction : public MovementAction
{
public:
    IllidanStormrageMeleeStayAwayFromDemonFormAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage melee stay away from demon form") {}
    bool Execute(Event event) override;
};

// The warlock tank holds him from its position with Shadow Ward and Searing Pain, Life Tapping
// between Searing Pains.
class IllidanStormrageWarlockTankHandleDemonFormAction : public MovementAction
{
public:
    IllidanStormrageWarlockTankHandleDemonFormAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage warlock tank handle demon form") {}
    bool Execute(Event event) override;
};

// A channel would hold the first Seed back; Life Tap comes between Seeds.
class IllidanStormrageSeedShadowDemonsAction : public Action
{
public:
    IllidanStormrageSeedShadowDemonsAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage seed shadow demons") {}
    bool Execute(Event event) override;

private:
    Unit* FindSeedTarget();
};

class IllidanStormrageAssignTargetPriorityAction : public AttackAction
{
public:
    IllidanStormrageAssignTargetPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage assign target priority") {}
    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetTargetsInPriorityOrder(Unit* illidan, BtHelpers::IllidanPhase phase);
    Unit* FindShadowfiendForMelee();
    Unit* FindShadowfiendForRanged(BtHelpers::IllidanPhase phase);
};

class IllidanStormrageManageDpsTimersAction : public Action
{
public:
    IllidanStormrageManageDpsTimersAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage manage dps timers") {}
    bool Execute(Event event) override;
};

class IllidanStormrageClearHazardsAction : public Action
{
public:
    IllidanStormrageClearHazardsAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage clear hazards") {}
    bool Execute(Event event) override;
};

class IllidanStormrageAnnounceClassRolesAction : public Action
{
public:
    IllidanStormrageAnnounceClassRolesAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage announce class roles") {}
    bool Execute(Event event) override;
};

#endif
