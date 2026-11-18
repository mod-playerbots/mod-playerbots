/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCACTIONS_JARAXXUS_H

#include <string>

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_Shared.h"

class JaraxxusInterruptFelFireballAction : public Action
{
public:
    JaraxxusInterruptFelFireballAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus interrupt fel fireball") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusAvoidLegionFlameAction : public MovementAction
{
public:
    JaraxxusAvoidLegionFlameAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus avoid legion flame") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;

private:
    void ClearLatches();
    bool IssueLeg(Position const& spot, uint32 now);

    Position _spot;
    Position _legStart;
    Position _lastIssued;
    bool _hasSpot = false;
    uint32 _issuedMs = 0;
    float _heading = 0.0f;
    bool _hasHeading = false;
    uint32 _lastTickMs = 0;
};

class JaraxxusBreakPinnedCastAction : public Action
{
public:
    JaraxxusBreakPinnedCastAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus break pinned cast") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusLeavePortalAction : public MovementAction
{
public:
    JaraxxusLeavePortalAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus leave portal") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;

private:
    Position _spot;
    bool _hasSpot = false;
    uint32 _issuedMs = 0;
};

class JaraxxusCastRaidCoverAction : public Action
{
public:
    JaraxxusCastRaidCoverAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus cast raid cover") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusCastPersonalDefensiveAction : public Action
{
public:
    JaraxxusCastPersonalDefensiveAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus cast personal defensive") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusMainTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    JaraxxusMainTankHoldBossAction(PlayerbotAI* botAI, std::string const name = "jaraxxus main tank hold boss")
        : ToCMainTankHoldAction(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusAssistTankHoldAction : public AttackAction
{
public:
    JaraxxusAssistTankHoldAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {};

protected:
    bool HoldAdd(uint8 index);
};

class JaraxxusAssistTankHoldAddAction : public JaraxxusAssistTankHoldAction
{
public:
    JaraxxusAssistTankHoldAddAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus assist tank hold add")
        : JaraxxusAssistTankHoldAction(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusAssistTankHoldSecondAddAction : public JaraxxusAssistTankHoldAction
{
public:
    JaraxxusAssistTankHoldSecondAddAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus assist tank hold second add")
        : JaraxxusAssistTankHoldAction(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusRemoveNetherPowerAction : public Action
{
public:
    JaraxxusRemoveNetherPowerAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus remove nether power") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusDpsTargetAction : public RaidDirectAttackAction
{
public:
    JaraxxusDpsTargetAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus dps target") : RaidDirectAttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class JaraxxusHealIncinerateTargetAction : public Action
{
public:
    JaraxxusHealIncinerateTargetAction(
        PlayerbotAI* botAI, std::string const name = "jaraxxus heal incinerate target") : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class ToCJaraxxusActionContext : public NamedObjectContext<Action>
{
public:
    ToCJaraxxusActionContext()
    {
        creators["jaraxxus interrupt fel fireball"] =
            &ToCJaraxxusActionContext::jaraxxus_interrupt_fel_fireball;
        creators["jaraxxus avoid legion flame"] =
            &ToCJaraxxusActionContext::jaraxxus_avoid_legion_flame;
        creators["jaraxxus break pinned cast"] =
            &ToCJaraxxusActionContext::jaraxxus_break_pinned_cast;
        creators["jaraxxus leave portal"] =
            &ToCJaraxxusActionContext::jaraxxus_leave_portal;
        creators["jaraxxus cast raid cover"] =
            &ToCJaraxxusActionContext::jaraxxus_cast_raid_cover;
        creators["jaraxxus cast personal defensive"] =
            &ToCJaraxxusActionContext::jaraxxus_cast_personal_defensive;
        creators["jaraxxus main tank hold boss"] =
            &ToCJaraxxusActionContext::jaraxxus_main_tank_hold_boss;
        creators["jaraxxus assist tank hold add"] =
            &ToCJaraxxusActionContext::jaraxxus_assist_tank_hold_add;
        creators["jaraxxus assist tank hold second add"] =
            &ToCJaraxxusActionContext::jaraxxus_assist_tank_hold_second_add;
        creators["jaraxxus remove nether power"] =
            &ToCJaraxxusActionContext::jaraxxus_remove_nether_power;
        creators["jaraxxus dps target"] =
            &ToCJaraxxusActionContext::jaraxxus_dps_target;
        creators["jaraxxus heal incinerate target"] =
            &ToCJaraxxusActionContext::jaraxxus_heal_incinerate_target;
    }

private:
    static Action* jaraxxus_interrupt_fel_fireball(PlayerbotAI* botAI) {
        return new JaraxxusInterruptFelFireballAction(botAI);
    }

    static Action* jaraxxus_avoid_legion_flame(PlayerbotAI* botAI) {
        return new JaraxxusAvoidLegionFlameAction(botAI);
    }

    static Action* jaraxxus_break_pinned_cast(PlayerbotAI* botAI) {
        return new JaraxxusBreakPinnedCastAction(botAI);
    }

    static Action* jaraxxus_leave_portal(PlayerbotAI* botAI) {
        return new JaraxxusLeavePortalAction(botAI);
    }

    static Action* jaraxxus_cast_raid_cover(PlayerbotAI* botAI) {
        return new JaraxxusCastRaidCoverAction(botAI);
    }

    static Action* jaraxxus_cast_personal_defensive(PlayerbotAI* botAI) {
        return new JaraxxusCastPersonalDefensiveAction(botAI);
    }

    static Action* jaraxxus_main_tank_hold_boss(PlayerbotAI* botAI) {
        return new JaraxxusMainTankHoldBossAction(botAI);
    }

    static Action* jaraxxus_assist_tank_hold_add(PlayerbotAI* botAI) {
        return new JaraxxusAssistTankHoldAddAction(botAI);
    }

    static Action* jaraxxus_assist_tank_hold_second_add(PlayerbotAI* botAI) {
        return new JaraxxusAssistTankHoldSecondAddAction(botAI);
    }

    static Action* jaraxxus_remove_nether_power(PlayerbotAI* botAI) {
        return new JaraxxusRemoveNetherPowerAction(botAI);
    }

    static Action* jaraxxus_dps_target(PlayerbotAI* botAI) {
        return new JaraxxusDpsTargetAction(botAI);
    }

    static Action* jaraxxus_heal_incinerate_target(PlayerbotAI* botAI) {
        return new JaraxxusHealIncinerateTargetAction(botAI);
    }
};

#endif
