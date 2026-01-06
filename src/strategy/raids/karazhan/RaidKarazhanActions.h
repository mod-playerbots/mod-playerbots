#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class ManaWarpStunCreatureBeforeWarpBreachAction : public AttackAction
{
public:
    ManaWarpStunCreatureBeforeWarpBreachAction(
        PlayerbotAI* botAI, std::string const name = "mana warp stun creature before warp breach") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanMarkTargetAction : public AttackAction
{
public:
    AttumenTheHuntsmanMarkTargetAction(
        PlayerbotAI* botAI, std::string const name = "attumen the huntsman mark target") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanSplitBossesAction : public AttackAction
{
public:
    AttumenTheHuntsmanSplitBossesAction(
        PlayerbotAI* botAI, std::string const name = "attumen the huntsman split bosses") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanStackBehindAction : public MovementAction
{
public:
    AttumenTheHuntsmanStackBehindAction(
        PlayerbotAI* botAI, std::string const name = "attumen the huntsman stack behind") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanManageDpsTimerAction : public Action
{
public:
    AttumenTheHuntsmanManageDpsTimerAction(
        PlayerbotAI* botAI, std::string const name = "attumen the huntsman manage dps timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MoroesMainTankAttackBossAction : public AttackAction
{
public:
    MoroesMainTankAttackBossAction(
        PlayerbotAI* botAI, std::string const name = "moroes main tank attack boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MoroesMarkTargetAction : public Action
{
public:
    MoroesMarkTargetAction(
        PlayerbotAI* botAI, std::string const name = "moroes mark target") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueMoveBossToHealerAction : public AttackAction
{
public:
    MaidenOfVirtueMoveBossToHealerAction(
        PlayerbotAI* botAI, std::string const name = "maiden of virtue move boss to healer") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MaidenOfVirtuePositionRangedAction : public MovementAction
{
public:
    MaidenOfVirtuePositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "maiden of virtue position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BigBadWolfPositionBossAction : public AttackAction
{
public:
    BigBadWolfPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "big bad wolf position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BigBadWolfRunAwayFromBossAction : public MovementAction
{
public:
    BigBadWolfRunAwayFromBossAction(
        PlayerbotAI* botAI, std::string const name = "big bad wolf run away from boss") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class RomuloAndJulianneMarkTargetAction : public Action
{
public:
    RomuloAndJulianneMarkTargetAction(
        PlayerbotAI* botAI, std::string const name = "romulo and julianne mark target") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class WizardOfOzMarkTargetAction : public Action
{
public:
    WizardOfOzMarkTargetAction(
        PlayerbotAI* botAI, std::string const name = "wizard of oz mark target") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class WizardOfOzScorchStrawmanAction : public Action
{
public:
    WizardOfOzScorchStrawmanAction(
        PlayerbotAI* botAI, std::string const name = "wizard of oz scorch strawman") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class TheCuratorMarkAstralFlareAction : public Action
{
public:
    TheCuratorMarkAstralFlareAction(
        PlayerbotAI* botAI, std::string const name = "the curator mark astral flare") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class TheCuratorPositionBossAction : public AttackAction
{
public:
    TheCuratorPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "the curator position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TheCuratorSpreadRangedAction : public MovementAction
{
public:
    TheCuratorSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "the curator spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TerestianIllhoofMarkTargetAction : public Action
{
public:
    TerestianIllhoofMarkTargetAction(
        PlayerbotAI* botAI, std::string const name = "terestian illhoof mark target") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class ShadeOfAranRunAwayFromArcaneExplosionAction : public MovementAction
{
public:
    ShadeOfAranRunAwayFromArcaneExplosionAction(
        PlayerbotAI* botAI, std::string const name = "shade of aran run away from arcane explosion") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ShadeOfAranStopMovingDuringFlameWreathAction : public MovementAction
{
public:
    ShadeOfAranStopMovingDuringFlameWreathAction(
        PlayerbotAI* botAI, std::string const name = "shade of aran stop moving during flame wreath") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ShadeOfAranMarkConjuredElementalAction : public Action
{
public:
    ShadeOfAranMarkConjuredElementalAction(
        PlayerbotAI* botAI, std::string const name = "shade of aran mark conjured elemental") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class ShadeOfAranRangedMaintainDistanceAction : public MovementAction
{
public:
    ShadeOfAranRangedMaintainDistanceAction(
        PlayerbotAI* botAI, std::string const name = "shade of aran ranged maintain distance") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NetherspiteBlockRedBeamAction : public MovementAction
{
public:
    NetherspiteBlockRedBeamAction(
        PlayerbotAI* botAI, std::string const name = "netherspite block red beam") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Position GetPositionOnBeam(Unit* netherspite, Unit* portal, float distanceFromBoss);
    std::unordered_map<ObjectGuid, bool> _wasBlockingRedBeam;
};

class NetherspiteBlockBlueBeamAction : public MovementAction
{
public:
    NetherspiteBlockBlueBeamAction(
        PlayerbotAI* botAI, std::string const name = "netherspite block blue beam") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    std::unordered_map<ObjectGuid, bool> _wasBlockingBlueBeam;
};

class NetherspiteBlockGreenBeamAction : public MovementAction
{
public:
    NetherspiteBlockGreenBeamAction(
        PlayerbotAI* botAI, std::string const name = "netherspite block green beam") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    std::unordered_map<ObjectGuid, bool> _wasBlockingGreenBeam;
};

class NetherspiteAvoidBeamAndVoidZoneAction : public MovementAction
{
public:
    NetherspiteAvoidBeamAndVoidZoneAction(
        PlayerbotAI* botAI, std::string const name = "netherspite avoid beam and void zone") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    struct BeamAvoid
    {
        Unit* portal;
        float minDist, maxDist;
    };
    bool IsAwayFromBeams(float x, float y, const std::vector<BeamAvoid>& beams, Unit* netherspite);
};

class NetherspiteBanishPhaseAvoidVoidZoneAction : public MovementAction
{
public:
    NetherspiteBanishPhaseAvoidVoidZoneAction(
        PlayerbotAI* botAI, std::string const name = "netherspite banish phase avoid void zone") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NetherspiteManageTimersAndTrackersAction : public Action
{
public:
    NetherspiteManageTimersAndTrackersAction(
        PlayerbotAI* botAI, std::string const name = "netherspite manage timers and trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarEnfeebledAvoidHazardAction : public MovementAction
{
public:
    PrinceMalchezaarEnfeebledAvoidHazardAction(
        PlayerbotAI* botAI, std::string const name = "prince malchezaar enfeebled avoid hazard") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarNonTankAvoidInfernalAction : public MovementAction
{
public:
    PrinceMalchezaarNonTankAvoidInfernalAction(
        PlayerbotAI* botAI, std::string const name = "prince malchezaar non tank avoid infernal") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarMainTankMovementAction : public AttackAction
{
public:
    PrinceMalchezaarMainTankMovementAction(
        PlayerbotAI* botAI, std::string const name = "prince malchezaar main tank movement") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneGroundPhasePositionBossAction : public AttackAction
{
public:
    NightbaneGroundPhasePositionBossAction(
        PlayerbotAI* botAI, std::string const name = "nightbane ground phase position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneGroundPhaseRotateRangedPositionsAction : public MovementAction
{
public:
    NightbaneGroundPhaseRotateRangedPositionsAction(
        PlayerbotAI* botAI, std::string const name = "nightbane ground phase rotate ranged positions") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneCastFearWardOnMainTankAction : public Action
{
public:
    NightbaneCastFearWardOnMainTankAction(
        PlayerbotAI* botAI, std::string const name = "nightbane cast fear ward on main tank") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneControlPetAggressionAction : public Action
{
public:
    NightbaneControlPetAggressionAction(
        PlayerbotAI* botAI, std::string const name = "nightbane control pet aggression") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneFlightPhaseMovementAction : public MovementAction
{
public:
    NightbaneFlightPhaseMovementAction(
        PlayerbotAI* botAI, std::string const name = "nightbane flight phase movement") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NightbaneManageTimersAndTrackersAction : public Action
{
public:
    NightbaneManageTimersAndTrackersAction(
        PlayerbotAI* botAI, std::string const name = "nightbane manage timers and trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
