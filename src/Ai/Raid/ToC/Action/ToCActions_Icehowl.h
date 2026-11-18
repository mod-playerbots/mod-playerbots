/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_ICEHOWL_H
#define PLAYERBOTS_RAID_TOCACTIONS_ICEHOWL_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "ToCActions_NorthrendBeasts.h"

class IcehowlTankHoldBossAction : public AttackAction
{
public:
    IcehowlTankHoldBossAction(PlayerbotAI* botAI, std::string const name = "icehowl tank hold boss")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IcehowlClearChargePathAction : public MovementAction
{
public:
    IcehowlClearChargePathAction(
        PlayerbotAI* botAI, std::string const name = "icehowl clear charge path") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    MoveOutcome IssueDodge(Position const& spot, bool replacing);

    Position _dodgeSpot;
    uint32 _dodgeSpotMs = 0;
    bool _hasDodgeSpot = false;
    bool _dodgeParked = false;  // stood still because it already cleared the tight line
};

class IcehowlTankDefensiveAction : public Action
{
public:
    IcehowlTankDefensiveAction(PlayerbotAI* botAI) : Action(botAI, "icehowl tank defensive") {}
    bool Execute(Event event) override;
};

class IcehowlMoveToBreathStandAction : public NorthrendBeastsWalkAction
{
public:
    IcehowlMoveToBreathStandAction(PlayerbotAI* botAI, std::string const name = "icehowl move to breath stand")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IcehowlDazeNitroAction : public Action
{
public:
    IcehowlDazeNitroAction(PlayerbotAI* botAI) : Action(botAI, "icehowl daze nitro") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _triedMs = 0;
    bool _tried = false;
};

class ToCIcehowlActionContext : public NamedObjectContext<Action>
{
public:
    ToCIcehowlActionContext()
    {
        creators["icehowl daze nitro"] =
            &ToCIcehowlActionContext::icehowl_daze_nitro;
        creators["icehowl tank hold boss"] =
            &ToCIcehowlActionContext::icehowl_tank_hold_boss;
        creators["icehowl clear charge path"] =
            &ToCIcehowlActionContext::icehowl_clear_charge_path;
        creators["icehowl tank defensive"] =
            &ToCIcehowlActionContext::icehowl_tank_defensive;
        creators["icehowl move to breath stand"] =
            &ToCIcehowlActionContext::icehowl_move_to_breath_stand;
        creators["icehowl dps target"] =
            &ToCIcehowlActionContext::icehowl_dps_target;
    }

private:
    static Action* icehowl_tank_hold_boss(PlayerbotAI* botAI) {
        return new IcehowlTankHoldBossAction(botAI);
    }

    static Action* icehowl_clear_charge_path(PlayerbotAI* botAI) {
        return new IcehowlClearChargePathAction(botAI);
    }

    static Action* icehowl_tank_defensive(PlayerbotAI* botAI) {
        return new IcehowlTankDefensiveAction(botAI);
    }

    static Action* icehowl_move_to_breath_stand(PlayerbotAI* botAI) {
        return new IcehowlMoveToBreathStandAction(botAI);
    }

    static Action* icehowl_daze_nitro(PlayerbotAI* botAI) {
        return new IcehowlDazeNitroAction(botAI);
    }

    static Action* icehowl_dps_target(PlayerbotAI* botAI) {
        return new NorthrendBeastsDpsTargetAction(botAI, "icehowl dps target");
    }
};

#endif
