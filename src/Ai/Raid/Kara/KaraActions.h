/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAACTIONS_H
#define PLAYERBOTS_KARAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "MovementActions.h"
#include <optional>
#include <vector>

// Shared

class KarazhanResetEncounterStatesAction : public Action
{
public:
    KarazhanResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "karazhan reset encounter states") {}
    bool Execute(Event event) override;
};

class KarazhanSetTremorTotemAction : public Action
{
public:
    KarazhanSetTremorTotemAction(PlayerbotAI* botAI) : Action(botAI, "karazhan set tremor totem") {}
    bool Execute(Event event) override;
};

// Trash

class ManaWarpCastStunSpellAction : public Action
{
public:
    ManaWarpCastStunSpellAction(PlayerbotAI* botAI) : Action(botAI, "mana warp cast stun spell") {}
    bool Execute(Event event) override;
};

class ManaWarpStandByToStunAction : public MovementAction
{
public:
    ManaWarpStandByToStunAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mana warp stand by to stun") {}
    bool Execute(Event event) override;
};

// Attumen the Huntsman

class AttumenTheHuntsmanAssistTankPositionUnmountedAction : public AttackAction
{
public:
    AttumenTheHuntsmanAssistTankPositionUnmountedAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "attumen the huntsman assist tank position unmounted") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanAttackMidnightAction : public AttackAction
{
public:
    AttumenTheHuntsmanAttackMidnightAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "attumen the huntsman attack midnight") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanTankPositionMountedAction : public AttackAction
{
public:
    AttumenTheHuntsmanTankPositionMountedAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "attumen the huntsman tank position mounted") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanStackBehindAction : public MovementAction
{
public:
    AttumenTheHuntsmanStackBehindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "attumen the huntsman stack behind") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanStopAttackingAction : public Action
{
public:
    AttumenTheHuntsmanStopAttackingAction(PlayerbotAI* botAI)
        : Action(botAI, "attumen the huntsman stop attacking") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanSelectTargetForHoldAction : public Action
{
public:
    AttumenTheHuntsmanSelectTargetForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "attumen the huntsman select target for hold") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanManageDpsTimerAction : public Action
{
public:
    AttumenTheHuntsmanManageDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "attumen the huntsman manage dps timer") {}
    bool Execute(Event event) override;
};

// Moroes <Tower Steward>

class MoroesMarkTargetAction : public Action
{
public:
    MoroesMarkTargetAction(PlayerbotAI* botAI) : Action(botAI, "moroes mark target") {}
    bool Execute(Event event) override;
};

// Maiden of Virtue

class MaidenOfVirtueTankPositionBossAction : public AttackAction
{
public:
    MaidenOfVirtueTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "maiden of virtue tank position boss") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueMoveBossToStunnedHealerAction : public MovementAction
{
public:
    MaidenOfVirtueMoveBossToStunnedHealerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "maiden of virtue move boss to stunned healer") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtuePositionRangedBetweenPillarsAction : public MovementAction
{
public:
    MaidenOfVirtuePositionRangedBetweenPillarsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "maiden of virtue position ranged between pillars") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueSetGroundingTotemAction : public Action
{
public:
    MaidenOfVirtueSetGroundingTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "maiden of virtue set grounding totem") {}
    bool Execute(Event event) override;
};

// The Big Bad Wolf

class BigBadWolfTankPositionBossAction : public AttackAction
{
public:
    BigBadWolfTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "big bad wolf tank position boss") {}
    bool Execute(Event event) override;
};

class BigBadWolfRedHoodUseEscapeAbilityAction : public Action
{
public:
    BigBadWolfRedHoodUseEscapeAbilityAction(PlayerbotAI* botAI)
        : Action(botAI, "big bad wolf red hood use escape ability") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class BigBadWolfRedHoodRunAwayAction : public MovementAction
{
public:
    BigBadWolfRedHoodRunAwayAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "big bad wolf red hood run away") {}
    bool Execute(Event event) override;
    bool ResetRun()
    {
        _run = {};
        if (!_lastRunStepTime)
            return false;
        _lastRunStepTime = 0;
        return true;
    }

private:
    KaraHelpers::BigBadWolfRunPlan _run;
    uint32 _lastRunStepTime = 0;
};

class BigBadWolfProtectRedHoodAction : public Action
{
public:
    BigBadWolfProtectRedHoodAction(PlayerbotAI* botAI)
        : Action(botAI, "big bad wolf protect red hood") {}
    bool Execute(Event event) override;

private:
    bool CastOnTarget(char const* spell, Player* target, float range);
};

// Romulo and Julianne

class RomuloAndJulianneSkullHigherHealthAction : public Action
{
public:
    RomuloAndJulianneSkullHigherHealthAction(PlayerbotAI* botAI)
        : Action(botAI, "romulo and julianne skull higher health") {}
    bool Execute(Event event) override;
};

// The Wizard of Oz

class WizardOfOzMarkTargetAction : public Action
{
public:
    WizardOfOzMarkTargetAction(PlayerbotAI* botAI) : Action(botAI, "wizard of oz mark target") {}
    bool Execute(Event event) override;
};

class WizardOfOzCastScorchOnStrawmanAction : public MovementAction
{
public:
    WizardOfOzCastScorchOnStrawmanAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "wizard of oz cast scorch on strawman") {}
    bool Execute(Event event) override;
};

// The Curator

class TheCuratorMarkAstralFlareAction : public Action
{
public:
    TheCuratorMarkAstralFlareAction(PlayerbotAI* botAI)
        : Action(botAI, "the curator mark astral flare") {}
    bool Execute(Event event) override;
};

class TheCuratorTankPositionBossAction : public AttackAction
{
public:
    TheCuratorTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the curator tank position boss") {}
    bool Execute(Event event) override;
};

class TheCuratorSpreadRangedAction : public MovementAction
{
public:
    TheCuratorSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the curator spread ranged") {}
    bool Execute(Event event) override;
};

// Terestian Illhoof

class TerestianIllhoofMarkTargetAction : public Action
{
public:
    TerestianIllhoofMarkTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "terestian illhoof mark target") {}
    bool Execute(Event event) override;
};

// Shade of Aran

class ShadeOfAranRunAwayFromArcaneExplosionAction : public MovementAction
{
public:
    ShadeOfAranRunAwayFromArcaneExplosionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran run away from arcane explosion") {}
    bool Execute(Event event) override;
};

class ShadeOfAranMoveBehindBlizzardAction : public MovementAction
{
public:
    ShadeOfAranMoveBehindBlizzardAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran move behind blizzard") {}
    bool Execute(Event event) override;
};

class ShadeOfAranStopMovingAction : public MovementAction
{
public:
    ShadeOfAranStopMovingAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran stop moving") {}
    bool Execute(Event event) override;
};

class ShadeOfAranMarkElementalAction : public Action
{
public:
    ShadeOfAranMarkElementalAction(PlayerbotAI* botAI)
        : Action(botAI, "shade of aran mark elemental") {}
    bool Execute(Event event) override;
};

class ShadeOfAranRangedMaintainDistanceAction : public MovementAction
{
public:
    ShadeOfAranRangedMaintainDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran ranged maintain distance") {}
    bool Execute(Event event) override;
};

// Netherspite

class NetherspiteBlockRedBeamAction : public MovementAction
{
public:
    NetherspiteBlockRedBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite block red beam") {}
    bool Execute(Event event) override;
    bool ResetRedBeamState()
    {
        bool const wasSet = _redBeamTimerWasSet || _lastBeamMoveSideways;
        _redBeamMoveTimer = 0;
        _lastBeamMoveSideways = false;
        _redBeamTimerWasSet = false;
        return wasSet;
    }

private:
    uint32 _redBeamMoveTimer = 0;
    bool _lastBeamMoveSideways = false;
    bool _redBeamTimerWasSet = false;
};

class NetherspiteBlockBlueOrGreenBeamAction : public MovementAction
{
public:
    NetherspiteBlockBlueOrGreenBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite block blue or green beam") {}
    bool Execute(Event event) override;
};

class NetherspiteWaitToBlockBeamAction : public MovementAction
{
public:
    NetherspiteWaitToBlockBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite wait to block beam") {}
    bool Execute(Event event) override;
};

class NetherspiteMoveBetweenBlueAndGreenBeamsAction : public MovementAction
{
public:
    NetherspiteMoveBetweenBlueAndGreenBeamsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite move between blue and green beams") {}
    bool Execute(Event event) override;
    bool ResetDestination()
    {
        bool const wasSet = _destination.has_value();
        _destination.reset();
        return wasSet;
    }

private:
    std::optional<Position> _destination;
};

class NetherspiteMoveToBanishSlotAction : public MovementAction
{
public:
    NetherspiteMoveToBanishSlotAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite move to banish slot") {}
    bool Execute(Event event) override;
};

class NetherspiteStopAttackingAction : public Action
{
public:
    NetherspiteStopAttackingAction(PlayerbotAI* botAI)
        : Action(botAI, "netherspite stop attacking") {}
    bool Execute(Event event) override;
};

class NetherspiteSelectTargetForHoldAction : public Action
{
public:
    NetherspiteSelectTargetForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "netherspite select target for hold") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class NetherspiteSetPetStanceAction : public Action
{
public:
    NetherspiteSetPetStanceAction(PlayerbotAI* botAI)
        : Action(botAI, "netherspite set pet stance") {}
    bool Execute(Event event) override;
    bool ResetPetStance();

private:
    std::vector<EncounterHelpers::SavedMinionStance> _previousReactStates;
};

class NetherspiteManageTimersAndTrackersAction : public Action
{
public:
    NetherspiteManageTimersAndTrackersAction(PlayerbotAI* botAI)
        : Action(botAI, "netherspite manage timers and trackers") {}
    bool Execute(Event event) override;

private:
    std::vector<Player*> GetBeamCandidates(
        KaraHelpers::NetherspiteBeamData const& data,
        KaraHelpers::NetherspiteBeamState const& beamState,
        std::vector<ObjectGuid> const& taken);
    bool IsNextPositionGood(
        Unit* netherspite, Position const& portal, KaraHelpers::KaraInstanceState const& state,
        float side, Position const& position, float low, bool isHunter);
    std::optional<Position> FindNextPosition(
        Unit* netherspite, Position const& portal, KaraHelpers::KaraInstanceState const& state,
        float side, float low);
    std::optional<Position> ShiftNextPositionAlongLine(
        Unit* netherspite, Position const& portal, KaraHelpers::KaraInstanceState const& state,
        float side, Position const& position, float low, bool isHunter);
    std::optional<Position> GetFloorPosition(Unit* netherspite, float x, float y);
    void UpdateBeamBlockers(
        Unit* netherspite, KaraHelpers::KaraInstanceState& state,
        KaraHelpers::NetherspiteBeamData const& data, std::vector<ObjectGuid>& taken);
    Position GetBanishPosition(
        Unit& netherspite, KaraHelpers::KaraInstanceState const& state, float angle,
        float startRadius);
    void UpdateBanishSlots(Unit* netherspite, KaraHelpers::KaraInstanceState& state);
    void UpdateVoidZones(KaraHelpers::KaraInstanceState& state);
};

// Prince Malchezaar

class PrinceMalchezaarEnfeebledBotRunToSafetyAction : public MovementAction
{
public:
    PrinceMalchezaarEnfeebledBotRunToSafetyAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "prince malchezaar enfeebled bot run to safety") {}
    bool Execute(Event event) override;
    bool HasDestinationFor(time_t enfeebleApplyTime, bool forShadowNova) const
    {
        return _destination.has_value() && _enfeebleApplyTime == enfeebleApplyTime &&
            _forShadowNova == forShadowNova;
    }
    bool ResetDestination()
    {
        bool const wasSet = _destination.has_value();
        _destination.reset();
        _enfeebleApplyTime = 0;
        _forShadowNova = false;
        return wasSet;
    }

private:
    std::optional<Position> _destination;
    time_t _enfeebleApplyTime = 0;
    bool _forShadowNova = false;
};

class PrinceMalchezaarMoveClearOfHazardsAction : public MovementAction
{
public:
    PrinceMalchezaarMoveClearOfHazardsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "prince malchezaar move clear of hazards") {}
    bool Execute(Event event) override;
    bool HasDestination() const { return _destination.has_value(); }
    bool ResetDestination()
    {
        bool const wasSet = _destination.has_value();
        _destination.reset();
        return wasSet;
    }

private:
    std::optional<Position> _destination;
    size_t _searchedInfernalCount = 0;
};

class PrinceMalchezaarTankPositionBossAction : public AttackAction
{
public:
    PrinceMalchezaarTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "prince malchezaar tank position boss") {}
    bool Execute(Event event) override;
    bool ResetWalk()
    {
        bool const wasSet = _destination.has_value() || _searchedInfernalCount.has_value();
        _destination.reset();
        _destinationClearance = 0.0f;
        _searchedInfernalCount.reset();
        return wasSet;
    }

private:
    std::optional<Position> _destination;
    float _destinationClearance = 0.0f;
    std::optional<size_t> _searchedInfernalCount;
};

// Nightbane

class NightbaneTankPositionBossAction : public AttackAction
{
public:
    NightbaneTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "nightbane tank position boss") {}
    bool Execute(Event event) override;
};

class NightbaneRangedMoveFromCharredEarthAction : public MovementAction
{
public:
    NightbaneRangedMoveFromCharredEarthAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane ranged move from charred earth") {}
    bool Execute(Event event) override;
};

class NightbaneMeleeMoveFromCharredEarthAction : public MovementAction
{
public:
    NightbaneMeleeMoveFromCharredEarthAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane melee move from charred earth") {}
    bool Execute(Event event) override;
};

class NightbaneSetPetStanceAction : public Action
{
public:
    NightbaneSetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "nightbane set pet stance") {}
    bool Execute(Event event) override;
    bool ResetPetStance();

private:
    std::vector<EncounterHelpers::SavedMinionStance> _previousReactStates;
};

class NightbaneStopAttackingAction : public Action
{
public:
    NightbaneStopAttackingAction(PlayerbotAI* botAI) : Action(botAI, "nightbane stop attacking") {}
    bool Execute(Event event) override;
};

class NightbaneSelectTargetForHoldAction : public Action
{
public:
    NightbaneSelectTargetForHoldAction(PlayerbotAI* botAI)
        : Action(botAI, "nightbane select target for hold") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class NightbaneStackAwayFromRainOfBonesAction : public MovementAction
{
public:
    NightbaneStackAwayFromRainOfBonesAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane stack away from rain of bones") {}
    bool Execute(Event event) override;
};

class NightbaneLeaveCombatForLandingAction : public Action
{
public:
    NightbaneLeaveCombatForLandingAction(PlayerbotAI* botAI)
        : Action(botAI, "nightbane leave combat for landing") {}
    bool Execute(Event event) override;
};

class NightbaneManageTimersAndTrackersAction : public Action
{
public:
    NightbaneManageTimersAndTrackersAction(PlayerbotAI* botAI)
        : Action(botAI, "nightbane manage timers and trackers") {}
    bool Execute(Event event) override;

private:
    bool UpdateMeleePosition(
        KaraHelpers::KaraInstanceState& state, Unit& nightbane,
        std::vector<Position> const& charredEarths);
    bool UpdateRangedPosition(
        KaraHelpers::KaraInstanceState& state, Unit& nightbane,
        std::vector<Position> const& charredEarths);
};

#endif
