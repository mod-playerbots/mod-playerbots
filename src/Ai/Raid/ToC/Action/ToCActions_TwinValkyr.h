/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_TWINVALKYR_H
#define PLAYERBOTS_RAID_TOCACTIONS_TWINVALKYR_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "RaidDirectTargeting.h"
#include "RaidRedirectThreat.h"
#include "ToCActions_Shared.h"
#include "ToCHelpers_TwinValkyr.h"

class TwinValkyrInterruptPactAction : public Action
{
public:
    TwinValkyrInterruptPactAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr interrupt pact") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Runs only while GetWantedEssence names this action's reason, and walks to that colour's portal.
class TwinValkyrEssenceActionBase : public MovementAction
{
public:
    TwinValkyrEssenceActionBase(PlayerbotAI* botAI, std::string const name,
                                TrialOfTheCrusaderHelpers::TwinEssenceReason reason)
        : MovementAction(botAI, name), _reason(reason) {}
    bool Execute(Event event) override;

protected:
    bool AcquireEssence(TrialOfTheCrusaderHelpers::TwinColour colour);

private:
    TrialOfTheCrusaderHelpers::TwinEssenceReason const _reason;
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrSwapEssenceForTouchAction : public TwinValkyrEssenceActionBase
{
public:
    TwinValkyrSwapEssenceForTouchAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr swap essence for touch")
        : TwinValkyrEssenceActionBase(botAI, name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Touch) {}
};

class TwinValkyrSwapEssenceForVortexAction : public TwinValkyrEssenceActionBase
{
public:
    TwinValkyrSwapEssenceForVortexAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr swap essence for vortex")
        : TwinValkyrEssenceActionBase(botAI, name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Vortex) {}
};

class TwinValkyrTakeBaseEssenceAction : public TwinValkyrEssenceActionBase
{
public:
    TwinValkyrTakeBaseEssenceAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr take base essence")
        : TwinValkyrEssenceActionBase(botAI, name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Base) {}
};

class TwinValkyrWalkToNextPortalAction : public MovementAction
{
public:
    TwinValkyrWalkToNextPortalAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr walk to next portal") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrCastVortexRaidCoverAction : public Action
{
public:
    TwinValkyrCastVortexRaidCoverAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr cast vortex raid cover") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class TwinValkyrCastVortexDefensiveAction : public Action
{
public:
    TwinValkyrCastVortexDefensiveAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr cast vortex defensive") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class TwinValkyrMoveToDoorStandAction : public MovementAction
{
public:
    TwinValkyrMoveToDoorStandAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr move to door stand") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrDodgeOrbAction : public MovementAction
{
public:
    TwinValkyrDodgeOrbAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr dodge orb") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Position _dodgeSpot;
    uint32 _dodgeSpotMs = 0;
};

// Waits on its door spot for its twin and taunts her in. Walks out only to fetch one that stopped
// short of taunt range, and back by a point 5 yd out of the spot so she parks on the arena side.
class TwinValkyrTankHoldAction : public ToCMainTankHoldAction
{
public:
    TwinValkyrTankHoldAction(PlayerbotAI* botAI, std::string const name) : ToCMainTankHoldAction(botAI, name) {}

protected:
    bool HoldTwin(Unit* twin);

private:
    bool FetchTwin(Unit* twin);
    bool WalkToDoor(Unit* twin);
    bool WalkFacingTwin(Position const& to, Unit* twin);

    bool _doorWalk = false;
    // back from a fetch, headed for the lead point first
    bool _leading = false;
    uint32 _leadReachedMs = 0;
};

class TwinValkyrMainTankHoldLightTwinAction : public TwinValkyrTankHoldAction
{
public:
    TwinValkyrMainTankHoldLightTwinAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr main tank hold light twin")
        : TwinValkyrTankHoldAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TwinValkyrAssistTankHoldDarkTwinAction : public TwinValkyrTankHoldAction
{
public:
    TwinValkyrAssistTankHoldDarkTwinAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr assist tank hold dark twin")
        : TwinValkyrTankHoldAction(botAI, name) {}
    bool Execute(Event event) override;
};

class TwinValkyrRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    TwinValkyrRedirectThreatAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr redirect threat")
        : RaidRedirectThreatAction(botAI, name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

class TwinValkyrFocusTwinAction : public RaidDirectAttackAction
{
public:
    TwinValkyrFocusTwinAction(
        PlayerbotAI* botAI, std::string const name = "twin valkyr focus twin") : RaidDirectAttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ToCTwinValkyrActionContext : public NamedObjectContext<Action>
{
public:
    ToCTwinValkyrActionContext()
    {
        creators["twin valkyr interrupt pact"] =
            &ToCTwinValkyrActionContext::twin_valkyr_interrupt_pact;
        creators["twin valkyr swap essence for touch"] =
            &ToCTwinValkyrActionContext::twin_valkyr_swap_essence_for_touch;
        creators["twin valkyr swap essence for vortex"] =
            &ToCTwinValkyrActionContext::twin_valkyr_swap_essence_for_vortex;
        creators["twin valkyr cast vortex raid cover"] =
            &ToCTwinValkyrActionContext::twin_valkyr_cast_vortex_raid_cover;
        creators["twin valkyr cast vortex defensive"] =
            &ToCTwinValkyrActionContext::twin_valkyr_cast_vortex_defensive;
        creators["twin valkyr dodge orb"] =
            &ToCTwinValkyrActionContext::twin_valkyr_dodge_orb;
        creators["twin valkyr walk to next portal"] =
            &ToCTwinValkyrActionContext::twin_valkyr_walk_to_next_portal;
        creators["twin valkyr take base essence"] =
            &ToCTwinValkyrActionContext::twin_valkyr_take_base_essence;
        creators["twin valkyr main tank hold light twin"] =
            &ToCTwinValkyrActionContext::twin_valkyr_main_tank_hold_light_twin;
        creators["twin valkyr assist tank hold dark twin"] =
            &ToCTwinValkyrActionContext::twin_valkyr_assist_tank_hold_dark_twin;
        creators["twin valkyr redirect threat"] =
            &ToCTwinValkyrActionContext::twin_valkyr_redirect_threat;
        creators["twin valkyr move to door stand"] =
            &ToCTwinValkyrActionContext::twin_valkyr_move_to_door_stand;
        creators["twin valkyr focus twin"] =
            &ToCTwinValkyrActionContext::twin_valkyr_focus_twin;
    }

private:
    static Action* twin_valkyr_interrupt_pact(PlayerbotAI* botAI) {
        return new TwinValkyrInterruptPactAction(botAI);
    }

    static Action* twin_valkyr_swap_essence_for_touch(PlayerbotAI* botAI) {
        return new TwinValkyrSwapEssenceForTouchAction(botAI);
    }

    static Action* twin_valkyr_swap_essence_for_vortex(PlayerbotAI* botAI) {
        return new TwinValkyrSwapEssenceForVortexAction(botAI);
    }

    static Action* twin_valkyr_dodge_orb(PlayerbotAI* botAI) {
        return new TwinValkyrDodgeOrbAction(botAI);
    }

    static Action* twin_valkyr_cast_vortex_raid_cover(PlayerbotAI* botAI) {
        return new TwinValkyrCastVortexRaidCoverAction(botAI);
    }

    static Action* twin_valkyr_cast_vortex_defensive(PlayerbotAI* botAI) {
        return new TwinValkyrCastVortexDefensiveAction(botAI);
    }

    static Action* twin_valkyr_walk_to_next_portal(PlayerbotAI* botAI) {
        return new TwinValkyrWalkToNextPortalAction(botAI);
    }

    static Action* twin_valkyr_take_base_essence(PlayerbotAI* botAI) {
        return new TwinValkyrTakeBaseEssenceAction(botAI);
    }

    static Action* twin_valkyr_main_tank_hold_light_twin(PlayerbotAI* botAI) {
        return new TwinValkyrMainTankHoldLightTwinAction(botAI);
    }

    static Action* twin_valkyr_assist_tank_hold_dark_twin(PlayerbotAI* botAI) {
        return new TwinValkyrAssistTankHoldDarkTwinAction(botAI);
    }

    static Action* twin_valkyr_redirect_threat(PlayerbotAI* botAI) {
        return new TwinValkyrRedirectThreatAction(botAI);
    }

    static Action* twin_valkyr_move_to_door_stand(PlayerbotAI* botAI) {
        return new TwinValkyrMoveToDoorStandAction(botAI);
    }

    static Action* twin_valkyr_focus_twin(PlayerbotAI* botAI) {
        return new TwinValkyrFocusTwinAction(botAI);
    }
};

#endif
