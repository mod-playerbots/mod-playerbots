/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONS_H
#define PLAYERBOTS_AQ40ACTIONS_H

#include "Action.h"
#include "Aq40Helpers.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"
#include <string>

// Shared

// Marks the strategy's own actions, which the AQ40 multipliers leave alone.
class Aq40ActionTag
{
public:
    virtual ~Aq40ActionTag() = default;
};

// Movement shared by hazard escapes and encounter positions.
class Aq40MovementAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40MovementAction(PlayerbotAI* botAI, std::string const& name) : MovementAction(botAI, name) {}

protected:
    // Hazard escapes (forced) may turn around obstacles and interrupt the bot's cast.
    bool MoveToGoal(Position goal, bool hazard);
};

class Aq40ResetEncounterStatesAction : public Action, public Aq40ActionTag
{
public:
    Aq40ResetEncounterStatesAction(PlayerbotAI* botAI) : Action(botAI, "aq40 reset encounter states") {}
    bool Execute(Event event) override;
};

// Mechanic tracker bookkeeping. Never takes the bot's turn.
class Aq40TrackerAction : public Action, public Aq40ActionTag
{
public:
    using TrackFn = void (*)(Player* bot);

    Aq40TrackerAction(PlayerbotAI* botAI, std::string const& name, TrackFn track) : Action(botAI, name), _track(track)
    {
    }
    bool Execute(Event event) override;

private:
    TrackFn const _track;
};

class Aq40DropToGroundAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40DropToGroundAction(PlayerbotAI* botAI) : MovementAction(botAI, "aq40 drop to ground") {}
    bool Execute(Event event) override;
};

class Aq40EscapeHazardAction : public Aq40MovementAction
{
public:
    Aq40EscapeHazardAction(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn hazard)
        : Aq40MovementAction(botAI, name), _hazard(hazard)
    {
    }
    bool Execute(Event event) override;

private:
    Aq40Helpers::Aq40GoalFn const _hazard;
};

class Aq40MoveToFormationAction : public Aq40MovementAction
{
public:
    Aq40MoveToFormationAction(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn formation)
        : Aq40MovementAction(botAI, name), _formation(formation)
    {
    }
    bool Execute(Event event) override;

private:
    Aq40Helpers::Aq40GoalFn const _formation;
};

// Before the pull, neither "follow" (drags the bot back to the master) nor "stay" (keeps pulling it back mid-fight)
// works, so the bot walks to its spot and holds it.
class Aq40MoveToPrepullSpotAction : public MovementAction, public Aq40ActionTag
{
public:
    Aq40MoveToPrepullSpotAction(PlayerbotAI* botAI, std::string const& name, Aq40Helpers::Aq40GoalFn spot)
        : MovementAction(botAI, name), _spot(spot)
    {
    }
    bool Execute(Event event) override;

private:
    Aq40Helpers::Aq40GoalFn const _spot;
};

// Each boss's kill order lives in the "aq40 assigned target" value; these actions attack it.
class Aq40AttackAssignedTargetAction : public AttackAction, public Aq40ActionTag
{
public:
    Aq40AttackAssignedTargetAction(PlayerbotAI* botAI, std::string const& name) : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class Aq40StopAttackingAction : public Action, public Aq40ActionTag
{
public:
    Aq40StopAttackingAction(PlayerbotAI* botAI, std::string const& name) : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class Aq40TankTauntAssignedTargetAction : public Action, public Aq40ActionTag
{
public:
    Aq40TankTauntAssignedTargetAction(PlayerbotAI* botAI) : Action(botAI, "aq40 tank taunt assigned target") {}
    bool Execute(Event event) override;
};

// The Prophet Skeram

class TheProphetSkeramCrowdControlMindControlledAction : public Action, public Aq40ActionTag
{
public:
    TheProphetSkeramCrowdControlMindControlledAction(PlayerbotAI* botAI)
        : Action(botAI, "the prophet skeram crowd control mind controlled")
    {
    }
    bool Execute(Event event) override;
};

// Battleguard Sartura

// Tanks leave too: a whirlwinding Sartura or guard is neither taunted nor damaged until it stops.
class BattleguardSarturaMoveAwayFromWhirlwindAction : public Aq40MovementAction
{
public:
    BattleguardSarturaMoveAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : Aq40MovementAction(botAI, "battleguard sartura move away from whirlwind")
    {
    }
    bool Execute(Event event) override;
};

// Viscidus

class ViscidusCastFrostSpellsAction : public Action, public Aq40ActionTag
{
public:
    ViscidusCastFrostSpellsAction(PlayerbotAI* botAI) : Action(botAI, "viscidus cast frost spells") {}
    bool Execute(Event event) override;
};

class ViscidusMeleeShatterAction : public AttackAction, public Aq40ActionTag
{
public:
    ViscidusMeleeShatterAction(PlayerbotAI* botAI) : AttackAction(botAI, "viscidus melee shatter") {}
    bool Execute(Event event) override;
};

// Princess Huhuran

class PrincessHuhuranTranquilizingShotAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranTranquilizingShotAction(PlayerbotAI* botAI) : Action(botAI, "princess huhuran tranquilizing shot") {}
    bool Execute(Event event) override;
};

class PrincessHuhuranCureWyvernStingAction : public Action, public Aq40ActionTag
{
public:
    PrincessHuhuranCureWyvernStingAction(PlayerbotAI* botAI) : Action(botAI, "princess huhuran cure wyvern sting") {}
    bool Execute(Event event) override;
};

// Twin Emperors

class TwinEmperorsDropEmperorThreatAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsDropEmperorThreatAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors drop emperor threat") {}
    bool Execute(Event event) override;
};

class EmperorVeknilashTankUseDefensiveCooldownAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeknilashTankUseDefensiveCooldownAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'nilash tank use defensive cooldown")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsHealEmperorTargetsAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsHealEmperorTargetsAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors heal emperor targets") {}
    bool Execute(Event event) override;
};

class TwinEmperorsWarlockSetPetPassiveAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsWarlockSetPetPassiveAction(PlayerbotAI* botAI) : Action(botAI, "twin emperors warlock set pet passive")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsWarlockPrepareForVeklorAction : public Action, public Aq40ActionTag
{
public:
    TwinEmperorsWarlockPrepareForVeklorAction(PlayerbotAI* botAI)
        : Action(botAI, "twin emperors warlock prepare for vek'lor")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWaitingTankTauntBossAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWaitingTankTauntBossAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor waiting tank taunt boss")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockTankCastSearingPainAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWarlockTankCastSearingPainAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor warlock tank cast searing pain")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorWarlockVoidwalkerSacrificeAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorWarlockVoidwalkerSacrificeAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor warlock voidwalker sacrifice")
    {
    }
    bool Execute(Event event) override;
};

class EmperorVeklorTankDemoralizingShoutAction : public Action, public Aq40ActionTag
{
public:
    EmperorVeklorTankDemoralizingShoutAction(PlayerbotAI* botAI)
        : Action(botAI, "emperor vek'lor tank demoralizing shout")
    {
    }
    bool Execute(Event event) override;
};

class TwinEmperorsLeadEmperorToPlatformAction : public MovementAction, public Aq40ActionTag
{
public:
    TwinEmperorsLeadEmperorToPlatformAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "twin emperors lead emperor to platform")
    {
    }
    bool Execute(Event event) override;
};

// C'Thun

class CthunHealersDamageCthunAction : public Action, public Aq40ActionTag
{
public:
    CthunHealersDamageCthunAction(PlayerbotAI* botAI) : Action(botAI, "c'thun healers damage c'thun") {}
    bool Execute(Event event) override;
};

class CthunHealersDamageFleshTentaclesAction : public Action, public Aq40ActionTag
{
public:
    CthunHealersDamageFleshTentaclesAction(PlayerbotAI* botAI) : Action(botAI, "c'thun healers damage flesh tentacles")
    {
    }
    bool Execute(Event event) override;
};

class CthunSendSpitOutTriggerAction : public Action, public Aq40ActionTag
{
public:
    CthunSendSpitOutTriggerAction(PlayerbotAI* botAI) : Action(botAI, "c'thun send spit-out trigger") {}
    bool Execute(Event event) override;
};

#endif
