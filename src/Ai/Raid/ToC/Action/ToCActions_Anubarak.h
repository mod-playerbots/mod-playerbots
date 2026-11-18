/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_ANUBARAK_H
#define PLAYERBOTS_RAID_TOCACTIONS_ANUBARAK_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "RaidRedirectThreat.h"
#include "ToCActions_Shared.h"
#include "ToCHelpers_Anubarak.h"

class AnubarakKiterImmunityAction : public Action
{
public:
    AnubarakKiterImmunityAction(
        PlayerbotAI* botAI, std::string const name = "anubarak kiter immunity") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakProtectKiterAction : public Action
{
public:
    AnubarakProtectKiterAction(
        PlayerbotAI* botAI, std::string const name = "anubarak protect kiter") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakFreeKiterAction : public Action
{
public:
    AnubarakFreeKiterAction(
        PlayerbotAI* botAI, std::string const name = "anubarak free kiter") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakHopEscortAction : public MovementAction
{
public:
    AnubarakHopEscortAction(PlayerbotAI* botAI, std::string const name = "anubarak hop escort")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    AnubarakRedirectThreatAction(PlayerbotAI* botAI, std::string const name = "anubarak redirect threat")
        : RaidRedirectThreatAction(botAI, name) {};

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

class AnubarakKiteSpikeToPermafrostAction : public MovementAction
{
public:
    AnubarakKiteSpikeToPermafrostAction(PlayerbotAI* botAI,
                                        std::string const name = "anubarak kite spike to permafrost")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakAvoidSpikeAction : public MovementAction
{
public:
    AnubarakAvoidSpikeAction(
        PlayerbotAI* botAI, std::string const name = "anubarak avoid spike") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;

private:
    bool WalkToDodge(Position const& spot);
};

class AnubarakInterruptShadowStrikeAction : public MovementAction
{
public:
    AnubarakInterruptShadowStrikeAction(PlayerbotAI* botAI, std::string const name = "anubarak interrupt shadow strike")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakTankDefensiveAction : public Action
{
public:
    AnubarakTankDefensiveAction(PlayerbotAI* botAI, std::string const name = "anubarak tank defensive",
                                bool physicalOnly = false)
        : Action(botAI, name), _physicalOnly(physicalOnly) {};
    bool Execute(Event event) override;

private:
    bool _physicalOnly;
};

class AnubarakColdSelfSaveAction : public Action
{
public:
    AnubarakColdSelfSaveAction(PlayerbotAI* botAI, std::string const name = "anubarak penetrating cold self save")
        : Action(botAI, name) {};
    bool Execute(Event event) override;

private:
    // The bubble this cast and when, so one the class node put up at low health stays
    uint32 _bubbleSpell = 0;
    uint32 _bubbleMs = 0;
};

class AnubarakDestroyFrostSphereAction : public AttackAction
{
public:
    AnubarakDestroyFrostSphereAction(
        PlayerbotAI* botAI, std::string const name = "anubarak destroy frost sphere") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakDpsTargetAction : public RaidDirectAttackAction
{
public:
    AnubarakDpsTargetAction(
        PlayerbotAI* botAI, std::string const name = "anubarak dps target") : RaidDirectAttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakAssistTankHoldBurrowerAction : public AttackAction
{
public:
    AnubarakAssistTankHoldBurrowerAction(PlayerbotAI* botAI,
                                         std::string const name = "anubarak assist tank hold burrower")
        : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakSideTankReadyAction : public AttackAction
{
public:
    AnubarakSideTankReadyAction(
        PlayerbotAI* botAI, std::string const name = "anubarak side tank ready") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakSideTankAreaThreatAction : public Action
{
public:
    AnubarakSideTankAreaThreatAction(PlayerbotAI* botAI, std::string const name = "anubarak side tank area threat")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class AnubarakDropCleaveBuffAction : public Action
{
public:
    AnubarakDropCleaveBuffAction(PlayerbotAI* botAI, std::string const name = "anubarak drop cleave buff")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class AnubarakGripFrostSphereAction : public AttackAction
{
public:
    AnubarakGripFrostSphereAction(
        PlayerbotAI* botAI, std::string const name = "anubarak grip frost sphere") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakBattleResSideTankAction : public MovementAction
{
public:
    AnubarakBattleResSideTankAction(PlayerbotAI* botAI, std::string const name = "anubarak battle res side tank")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
    // Rebirth won't go out in cat or bear form
    std::vector<NextAction> getPrerequisites() override { return {NextAction("caster form")}; }
};

class AnubarakGatherAction : public MovementAction
{
public:
    AnubarakGatherAction(PlayerbotAI* botAI, std::string const name = "anubarak gather")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakHealerStandAction : public MovementAction
{
public:
    AnubarakHealerStandAction(PlayerbotAI* botAI, std::string const name = "anubarak healer stand")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakRangedStandAction : public MovementAction
{
public:
    AnubarakRangedStandAction(PlayerbotAI* botAI, std::string const name = "anubarak ranged stand")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakLeavePermafrostAction : public MovementAction
{
public:
    AnubarakLeavePermafrostAction(PlayerbotAI* botAI, std::string const name = "anubarak leave permafrost")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakMeleeBurrowerStandAction : public MovementAction
{
public:
    AnubarakMeleeBurrowerStandAction(PlayerbotAI* botAI, std::string const name = "anubarak melee burrower stand")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakBurrowerLeadAction : public MovementAction
{
public:
    AnubarakBurrowerLeadAction(PlayerbotAI* botAI, std::string const name = "anubarak burrower lead")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakKiteCatDropAction : public Action
{
public:
    AnubarakKiteCatDropAction(PlayerbotAI* botAI, std::string const name = "anubarak kite cat drop")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakTankRescueAction : public Action
{
public:
    AnubarakTankRescueAction(PlayerbotAI* botAI, std::string const name = "anubarak tank rescue")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakLustAction : public Action
{
public:
    AnubarakLustAction(PlayerbotAI* botAI, std::string const name = "anubarak lust") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakDemonicCircleAction : public MovementAction
{
public:
    AnubarakDemonicCircleAction(PlayerbotAI* botAI, std::string const name = "anubarak demonic circle")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakTankPickUpScarabAction : public AttackAction
{
public:
    AnubarakTankPickUpScarabAction(
        PlayerbotAI* botAI, std::string const name = "anubarak tank pick up scarab") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakMainTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    AnubarakMainTankHoldBossAction(PlayerbotAI* botAI, std::string const name = "anubarak main tank hold boss")
        : ToCMainTankHoldAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakHealPenetratingColdAction : public Action
{
public:
    AnubarakHealPenetratingColdAction(
        PlayerbotAI* botAI, std::string const name = "anubarak heal penetrating cold") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakSwarmHealAction : public Action
{
public:
    AnubarakSwarmHealAction(PlayerbotAI* botAI, std::string const name = "anubarak swarm heal") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakAuraMasteryAction : public Action
{
public:
    AnubarakAuraMasteryAction(PlayerbotAI* botAI, std::string const name = "anubarak aura mastery")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
    bool isUseful() override;
};

// The class's "innervate on healer" runs under the name "innervate" too, which the self Innervate guard blocks
class AnubarakInnervateHealerAction : public Action
{
public:
    AnubarakInnervateHealerAction(PlayerbotAI* botAI, std::string const name = "anubarak innervate healer")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
    bool isUseful() override;
    std::vector<NextAction> getPrerequisites() override;
};

class AnubarakKiteScarabAction : public MovementAction
{
public:
    AnubarakKiteScarabAction(PlayerbotAI* botAI, std::string const name = "anubarak kite scarab")
        : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class AnubarakTopUpManaAction : public Action
{
public:
    AnubarakTopUpManaAction(PlayerbotAI* botAI, std::string const name = "anubarak top up mana")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

// The class Judgement of Light goes on the current target, which under the swarm is often a burrower
class AnubarakJudgeLightAction : public Action
{
public:
    AnubarakJudgeLightAction(PlayerbotAI* botAI, std::string const name = "anubarak judge light")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class ToCAnubarakActionContext : public NamedObjectContext<Action>
{
public:
    ToCAnubarakActionContext()
    {
        creators["anubarak kiter immunity"] =
            &ToCAnubarakActionContext::anubarak_kiter_immunity;
        creators["anubarak protect kiter"] =
            &ToCAnubarakActionContext::anubarak_protect_kiter;
        creators["anubarak free kiter"] =
            &ToCAnubarakActionContext::anubarak_free_kiter;
        creators["anubarak hop escort"] =
            &ToCAnubarakActionContext::anubarak_hop_escort;
        creators["anubarak redirect threat"] =
            &ToCAnubarakActionContext::anubarak_redirect_threat;
        creators["anubarak kite spike to permafrost"] =
            &ToCAnubarakActionContext::anubarak_kite_spike_to_permafrost;
        creators["anubarak avoid spike"] =
            &ToCAnubarakActionContext::anubarak_avoid_spike;
        creators["anubarak interrupt shadow strike"] =
            &ToCAnubarakActionContext::anubarak_interrupt_shadow_strike;
        creators["anubarak tank defensive"] =
            &ToCAnubarakActionContext::anubarak_tank_defensive;
        creators["anubarak side tank defensive"] =
            &ToCAnubarakActionContext::anubarak_side_tank_defensive;
        creators["anubarak penetrating cold self save"] =
            &ToCAnubarakActionContext::anubarak_penetrating_cold_self_save;
        creators["anubarak destroy frost sphere"] =
            &ToCAnubarakActionContext::anubarak_destroy_frost_sphere;
        creators["anubarak dps target"] =
            &ToCAnubarakActionContext::anubarak_dps_target;
        creators["anubarak assist tank hold burrower"] =
            &ToCAnubarakActionContext::anubarak_assist_tank_hold_burrower;
        creators["anubarak side tank ready"] =
            &ToCAnubarakActionContext::anubarak_side_tank_ready;
        creators["anubarak side tank area threat"] =
            &ToCAnubarakActionContext::anubarak_side_tank_area_threat;
        creators["anubarak drop cleave buff"] =
            &ToCAnubarakActionContext::anubarak_drop_cleave_buff;
        creators["anubarak grip frost sphere"] =
            &ToCAnubarakActionContext::anubarak_grip_frost_sphere;
        creators["anubarak battle res side tank"] =
            &ToCAnubarakActionContext::anubarak_battle_res_side_tank;
        creators["anubarak gather"] =
            &ToCAnubarakActionContext::anubarak_gather;
        creators["anubarak healer stand"] =
            &ToCAnubarakActionContext::anubarak_healer_stand;
        creators["anubarak ranged stand"] =
            &ToCAnubarakActionContext::anubarak_ranged_stand;
        creators["anubarak leave permafrost"] =
            &ToCAnubarakActionContext::anubarak_leave_permafrost;
        creators["anubarak melee burrower stand"] =
            &ToCAnubarakActionContext::anubarak_melee_burrower_stand;
        creators["anubarak burrower lead"] =
            &ToCAnubarakActionContext::anubarak_burrower_lead;
        creators["anubarak kite cat drop"] =
            &ToCAnubarakActionContext::anubarak_kite_cat_drop;
        creators["anubarak tank rescue"] =
            &ToCAnubarakActionContext::anubarak_tank_rescue;
        creators["anubarak lust"] =
            &ToCAnubarakActionContext::anubarak_lust;
        creators["anubarak demonic circle"] =
            &ToCAnubarakActionContext::anubarak_demonic_circle;
        creators["anubarak tank pick up scarab"] =
            &ToCAnubarakActionContext::anubarak_tank_pick_up_scarab;
        creators["anubarak main tank hold boss"] =
            &ToCAnubarakActionContext::anubarak_main_tank_hold_boss;
        creators["anubarak heal penetrating cold"] =
            &ToCAnubarakActionContext::anubarak_heal_penetrating_cold;
        creators["anubarak swarm heal"] =
            &ToCAnubarakActionContext::anubarak_swarm_heal;
        creators["anubarak aura mastery"] =
            &ToCAnubarakActionContext::anubarak_aura_mastery;
        creators["anubarak innervate healer"] =
            &ToCAnubarakActionContext::anubarak_innervate_healer;
        creators["anubarak kite scarab"] =
            &ToCAnubarakActionContext::anubarak_kite_scarab;
        creators["anubarak top up mana"] =
            &ToCAnubarakActionContext::anubarak_top_up_mana;
        creators["anubarak judge light"] =
            &ToCAnubarakActionContext::anubarak_judge_light;
    }

private:
    static Action* anubarak_kiter_immunity(PlayerbotAI* botAI) {
        return new AnubarakKiterImmunityAction(botAI);
    }

    static Action* anubarak_protect_kiter(PlayerbotAI* botAI) {
        return new AnubarakProtectKiterAction(botAI);
    }

    static Action* anubarak_free_kiter(PlayerbotAI* botAI) {
        return new AnubarakFreeKiterAction(botAI);
    }

    static Action* anubarak_hop_escort(PlayerbotAI* botAI) {
        return new AnubarakHopEscortAction(botAI);
    }

    static Action* anubarak_redirect_threat(PlayerbotAI* botAI) {
        return new AnubarakRedirectThreatAction(botAI);
    }

    static Action* anubarak_kite_spike_to_permafrost(PlayerbotAI* botAI) {
        return new AnubarakKiteSpikeToPermafrostAction(botAI);
    }

    static Action* anubarak_avoid_spike(PlayerbotAI* botAI) {
        return new AnubarakAvoidSpikeAction(botAI);
    }

    static Action* anubarak_interrupt_shadow_strike(PlayerbotAI* botAI) {
        return new AnubarakInterruptShadowStrikeAction(botAI);
    }

    static Action* anubarak_tank_defensive(PlayerbotAI* botAI) {
        return new AnubarakTankDefensiveAction(botAI);
    }

    // Burrower melee is physical, so Anti-Magic Shell would waste itself there. Phase 3 lets it through:
    // most of a side tank's damage is the swarm then.
    static Action* anubarak_side_tank_defensive(PlayerbotAI* botAI) {
        return new AnubarakTankDefensiveAction(botAI, "anubarak side tank defensive", true);
    }

    static Action* anubarak_penetrating_cold_self_save(PlayerbotAI* botAI) {
        return new AnubarakColdSelfSaveAction(botAI);
    }

    static Action* anubarak_destroy_frost_sphere(PlayerbotAI* botAI) {
        return new AnubarakDestroyFrostSphereAction(botAI);
    }

    static Action* anubarak_dps_target(PlayerbotAI* botAI) {
        return new AnubarakDpsTargetAction(botAI);
    }

    static Action* anubarak_assist_tank_hold_burrower(PlayerbotAI* botAI) {
        return new AnubarakAssistTankHoldBurrowerAction(botAI);
    }

    static Action* anubarak_side_tank_ready(PlayerbotAI* botAI) {
        return new AnubarakSideTankReadyAction(botAI);
    }

    static Action* anubarak_side_tank_area_threat(PlayerbotAI* botAI) {
        return new AnubarakSideTankAreaThreatAction(botAI);
    }

    static Action* anubarak_drop_cleave_buff(PlayerbotAI* botAI) {
        return new AnubarakDropCleaveBuffAction(botAI);
    }

    static Action* anubarak_grip_frost_sphere(PlayerbotAI* botAI) {
        return new AnubarakGripFrostSphereAction(botAI);
    }

    static Action* anubarak_battle_res_side_tank(PlayerbotAI* botAI) {
        return new AnubarakBattleResSideTankAction(botAI);
    }

    static Action* anubarak_gather(PlayerbotAI* botAI) {
        return new AnubarakGatherAction(botAI);
    }

    static Action* anubarak_healer_stand(PlayerbotAI* botAI) {
        return new AnubarakHealerStandAction(botAI);
    }

    static Action* anubarak_ranged_stand(PlayerbotAI* botAI) {
        return new AnubarakRangedStandAction(botAI);
    }

    static Action* anubarak_leave_permafrost(PlayerbotAI* botAI) {
        return new AnubarakLeavePermafrostAction(botAI);
    }

    static Action* anubarak_melee_burrower_stand(PlayerbotAI* botAI) {
        return new AnubarakMeleeBurrowerStandAction(botAI);
    }

    static Action* anubarak_burrower_lead(PlayerbotAI* botAI) {
        return new AnubarakBurrowerLeadAction(botAI);
    }

    static Action* anubarak_kite_cat_drop(PlayerbotAI* botAI) {
        return new AnubarakKiteCatDropAction(botAI);
    }

    static Action* anubarak_tank_rescue(PlayerbotAI* botAI) {
        return new AnubarakTankRescueAction(botAI);
    }

    static Action* anubarak_lust(PlayerbotAI* botAI) {
        return new AnubarakLustAction(botAI);
    }

    static Action* anubarak_demonic_circle(PlayerbotAI* botAI) {
        return new AnubarakDemonicCircleAction(botAI);
    }

    static Action* anubarak_tank_pick_up_scarab(PlayerbotAI* botAI) {
        return new AnubarakTankPickUpScarabAction(botAI);
    }

    static Action* anubarak_main_tank_hold_boss(PlayerbotAI* botAI) {
        return new AnubarakMainTankHoldBossAction(botAI);
    }

    static Action* anubarak_heal_penetrating_cold(PlayerbotAI* botAI) {
        return new AnubarakHealPenetratingColdAction(botAI);
    }

    static Action* anubarak_swarm_heal(PlayerbotAI* botAI) {
        return new AnubarakSwarmHealAction(botAI);
    }

    static Action* anubarak_aura_mastery(PlayerbotAI* botAI) {
        return new AnubarakAuraMasteryAction(botAI);
    }

    static Action* anubarak_innervate_healer(PlayerbotAI* botAI) {
        return new AnubarakInnervateHealerAction(botAI);
    }

    static Action* anubarak_kite_scarab(PlayerbotAI* botAI) {
        return new AnubarakKiteScarabAction(botAI);
    }

    static Action* anubarak_top_up_mana(PlayerbotAI* botAI) {
        return new AnubarakTopUpManaAction(botAI);
    }

    static Action* anubarak_judge_light(PlayerbotAI* botAI) {
        return new AnubarakJudgeLightAction(botAI);
    }
};

#endif
