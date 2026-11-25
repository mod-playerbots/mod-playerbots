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
#include "ObjectGuid.h"
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "RaidRedirectThreat.h"
#include "ToCActions_Shared.h"
#include "ToCHelpers_Anubarak.h"

class AnubarakKiterImmunityAction : public Action
{
public:
    static constexpr char const* Name = "anubarak kiter immunity";

    AnubarakKiterImmunityAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakProtectKiterAction : public Action
{
public:
    static constexpr char const* Name = "anubarak protect kiter";

    AnubarakProtectKiterAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakFreeKiterAction : public Action
{
public:
    static constexpr char const* Name = "anubarak free kiter";

    AnubarakFreeKiterAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakHopEscortAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak hop escort";

    AnubarakHopEscortAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "anubarak redirect threat";

    AnubarakRedirectThreatAction(PlayerbotAI* botAI) : RaidRedirectThreatAction(botAI, Name) {};

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

class AnubarakKiteSpikeToPermafrostAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak kite spike to permafrost";

    AnubarakKiteSpikeToPermafrostAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakAvoidSpikeAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak avoid spike";

    AnubarakAvoidSpikeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;

private:
    bool WalkToDodge(Position const& spot);
};

class AnubarakInterruptShadowStrikeAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak interrupt shadow strike";

    AnubarakInterruptShadowStrikeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakTankDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "anubarak tank defensive";

    AnubarakTankDefensiveAction(PlayerbotAI* botAI, std::string const name = Name,
                                bool physicalOnly = false)
        : Action(botAI, name), _physicalOnly(physicalOnly) {};
    bool Execute(Event event) override;

private:
    bool _physicalOnly;
};

class AnubarakColdSelfSaveAction : public Action
{
public:
    static constexpr char const* Name = "anubarak penetrating cold self save";

    AnubarakColdSelfSaveAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;

private:
    // The bubble this cast and when, so one the class node put up at low health stays
    uint32 _bubbleSpell = 0;
    uint32 _bubbleMs = 0;
};

class AnubarakDestroyFrostSphereAction : public AttackAction
{
public:
    static constexpr char const* Name = "anubarak destroy frost sphere";

    AnubarakDestroyFrostSphereAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakDpsTargetAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "anubarak dps target";

    AnubarakDpsTargetAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakAssistTankHoldBurrowerAction : public AttackAction
{
public:
    static constexpr char const* Name = "anubarak assist tank hold burrower";

    AnubarakAssistTankHoldBurrowerAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakSideTankReadyAction : public AttackAction
{
public:
    static constexpr char const* Name = "anubarak side tank ready";

    AnubarakSideTankReadyAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakSideTankAreaThreatAction : public Action
{
public:
    static constexpr char const* Name = "anubarak side tank area threat";

    AnubarakSideTankAreaThreatAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AnubarakDropCleaveBuffAction : public Action
{
public:
    static constexpr char const* Name = "anubarak drop cleave buff";

    AnubarakDropCleaveBuffAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AnubarakGripFrostSphereAction : public AttackAction
{
public:
    static constexpr char const* Name = "anubarak grip frost sphere";

    AnubarakGripFrostSphereAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakBattleResSideTankAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak battle res side tank";

    AnubarakBattleResSideTankAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
    // Rebirth won't go out in cat or bear form
    std::vector<NextAction> getPrerequisites() override { return {NextAction("caster form")}; }
};

class AnubarakGatherAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak gather";

    AnubarakGatherAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakHealerStandAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak healer stand";

    AnubarakHealerStandAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakRangedStandAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak ranged stand";

    AnubarakRangedStandAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakLeavePermafrostAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak leave permafrost";

    AnubarakLeavePermafrostAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakMeleeBurrowerStandAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak melee burrower stand";

    AnubarakMeleeBurrowerStandAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakBurrowerLeadAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak burrower lead";

    AnubarakBurrowerLeadAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakKiteCatDropAction : public Action
{
public:
    static constexpr char const* Name = "anubarak kite cat drop";

    AnubarakKiteCatDropAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakTankRescueAction : public Action
{
public:
    static constexpr char const* Name = "anubarak tank rescue";

    AnubarakTankRescueAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakLustAction : public Action
{
public:
    static constexpr char const* Name = "anubarak lust";

    AnubarakLustAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakDemonicCircleAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak demonic circle";

    AnubarakDemonicCircleAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakTankPickUpScarabAction : public AttackAction
{
public:
    static constexpr char const* Name = "anubarak tank pick up scarab";

    AnubarakTankPickUpScarabAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakMainTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    static constexpr char const* Name = "anubarak main tank hold boss";

    AnubarakMainTankHoldBossAction(PlayerbotAI* botAI) : ToCMainTankHoldAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakHealPenetratingColdAction : public Action
{
public:
    static constexpr char const* Name = "anubarak heal penetrating cold";

    AnubarakHealPenetratingColdAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakSwarmHealAction : public Action
{
public:
    static constexpr char const* Name = "anubarak swarm heal";

    AnubarakSwarmHealAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakAuraMasteryAction : public Action
{
public:
    static constexpr char const* Name = "anubarak aura mastery";

    AnubarakAuraMasteryAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
    bool isUseful() override;
};

// The class's "innervate on healer" runs under the name "innervate" too, which the self Innervate guard blocks
class AnubarakInnervateHealerAction : public Action
{
public:
    static constexpr char const* Name = "anubarak innervate healer";

    AnubarakInnervateHealerAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
    bool isUseful() override;
    std::vector<NextAction> getPrerequisites() override;
};

class AnubarakKiteScarabAction : public MovementAction
{
public:
    static constexpr char const* Name = "anubarak kite scarab";

    AnubarakKiteScarabAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class AnubarakTopUpManaAction : public Action
{
public:
    static constexpr char const* Name = "anubarak top up mana";

    AnubarakTopUpManaAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

// The class Judgement of Light goes on the current target, which under the swarm is often a burrower
class AnubarakJudgeLightAction : public Action
{
public:
    static constexpr char const* Name = "anubarak judge light";

    AnubarakJudgeLightAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

#endif
