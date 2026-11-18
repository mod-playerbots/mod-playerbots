/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_JORMUNGARS_H
#define PLAYERBOTS_RAID_TOCACTIONS_JORMUNGARS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "Position.h"
#include "RaidRedirectThreat.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCHelpers_NorthrendBeasts.h"

// Holds the worm of one duty through its submerges. Its walks are latched: every MoveTo clears the
// MotionMaster, so a spot re-derived each tick from a bot that's still walking never lands.
class NorthrendWormsTankHoldAction : public AttackAction
{
public:
    bool Execute(Event event) override;

protected:
    NorthrendWormsTankHoldAction(PlayerbotAI* botAI, std::string const name,
                                 TrialOfTheCrusaderHelpers::BeastsTankDuty holdDuty)
        : AttackAction(botAI, name), _duty(holdDuty) {}

private:
    enum class WalkKind : uint8
    {
        None,
        Approach,
        Drag
    };

    bool ApproachSubmerged(Unit* worm);
    bool DragOffHazards(Unit* worm);
    // The latched spot while the bot is still walking a walk of this kind, else nullptr
    Position const* WalkInFlight(WalkKind kind);
    // False without booking anything while a cast pins the bot's feet
    bool WalkTo(WalkKind kind, Position const& spot);
    bool BookedOnWalkSpot();

    TrialOfTheCrusaderHelpers::BeastsTankDuty const _duty;
    Position _walkSpot;
    Position _approachFrom;  // where the worm stood when the approach was issued
    uint32 _walkIssuedMs = 0;
    WalkKind _walking = WalkKind::None;
};

class NorthrendWormsTankHoldMobileWormAction : public NorthrendWormsTankHoldAction
{
public:
    NorthrendWormsTankHoldMobileWormAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms tank hold mobile worm")
        : NorthrendWormsTankHoldAction(botAI, name, TrialOfTheCrusaderHelpers::BeastsTankDuty::WormMobile) {}
};

class NorthrendWormsTankHoldStationaryWormAction : public NorthrendWormsTankHoldAction
{
public:
    NorthrendWormsTankHoldStationaryWormAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms tank hold stationary worm")
        : NorthrendWormsTankHoldAction(botAI, name, TrialOfTheCrusaderHelpers::BeastsTankDuty::WormStationary) {}
};

class NorthrendWormsRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    NorthrendWormsRedirectThreatAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms redirect threat")
        : RaidRedirectThreatAction(botAI, name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

// Every worm hazard through one mover. The spot it walks to is held: a bot pushed off walks back to it
// rather than searching again from wherever it landed.
class NorthrendWormsRepositionAction : public MovementAction
{
public:
    NorthrendWormsRepositionAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms reposition") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    // Tells our own booking from another mover's once the hold it was for is gone
    Position _issuedSpot;
    bool _hasIssuedSpot = false;
};

// Out of a pool a worm left behind. The walk is latched, so the next tick doesn't re-aim it.
class NorthrendWormsLeavePoolAction : public MovementAction
{
public:
    NorthrendWormsLeavePoolAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms leave pool") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Position _exitSpot;
    uint32 _exitMs = 0;
    bool _hasExit = false;
};

// Heals the healer's Toxin carrier, one no Bile can cure
class NorthrendWormsHealToxinCarrierAction : public Action
{
public:
    NorthrendWormsHealToxinCarrierAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms heal toxin carrier") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Divine Shield or Ice Block off the bot's own uncurable Toxin
class NorthrendWormsPurgeToxinAction : public Action
{
public:
    NorthrendWormsPurgeToxinAction(
        PlayerbotAI* botAI, std::string const name = "northrend worms purge toxin") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class ToCJormungarsActionContext : public NamedObjectContext<Action>
{
public:
    ToCJormungarsActionContext()
    {
        creators["northrend worms tank hold mobile worm"] =
            &ToCJormungarsActionContext::northrend_worms_tank_hold_mobile_worm;
        creators["northrend worms tank hold stationary worm"] =
            &ToCJormungarsActionContext::northrend_worms_tank_hold_stationary_worm;
        creators["northrend worms redirect threat"] =
            &ToCJormungarsActionContext::northrend_worms_redirect_threat;
        creators["northrend worms reposition"] =
            &ToCJormungarsActionContext::northrend_worms_reposition;
        creators["northrend worms leave pool"] =
            &ToCJormungarsActionContext::northrend_worms_leave_pool;
        creators["northrend worms dps target"] =
            &ToCJormungarsActionContext::northrend_worms_dps_target;
        creators["northrend worms heal toxin carrier"] =
            &ToCJormungarsActionContext::northrend_worms_heal_toxin_carrier;
        creators["northrend worms purge toxin"] =
            &ToCJormungarsActionContext::northrend_worms_purge_toxin;
    }

private:
    static Action* northrend_worms_tank_hold_mobile_worm(PlayerbotAI* botAI) {
        return new NorthrendWormsTankHoldMobileWormAction(botAI);
    }

    static Action* northrend_worms_tank_hold_stationary_worm(PlayerbotAI* botAI) {
        return new NorthrendWormsTankHoldStationaryWormAction(botAI);
    }

    static Action* northrend_worms_redirect_threat(PlayerbotAI* botAI) {
        return new NorthrendWormsRedirectThreatAction(botAI);
    }

    static Action* northrend_worms_reposition(PlayerbotAI* botAI) {
        return new NorthrendWormsRepositionAction(botAI);
    }

    static Action* northrend_worms_leave_pool(PlayerbotAI* botAI) {
        return new NorthrendWormsLeavePoolAction(botAI);
    }

    static Action* northrend_worms_dps_target(PlayerbotAI* botAI) {
        return new NorthrendBeastsDpsTargetAction(botAI, "northrend worms dps target");
    }

    static Action* northrend_worms_heal_toxin_carrier(PlayerbotAI* botAI) {
        return new NorthrendWormsHealToxinCarrierAction(botAI);
    }

    static Action* northrend_worms_purge_toxin(PlayerbotAI* botAI) {
        return new NorthrendWormsPurgeToxinAction(botAI);
    }
};

#endif
