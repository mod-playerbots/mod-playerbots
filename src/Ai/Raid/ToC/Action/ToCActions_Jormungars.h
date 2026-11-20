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
    static constexpr char const* Name = "northrend worms tank hold mobile worm";

    NorthrendWormsTankHoldMobileWormAction(PlayerbotAI* botAI)
        : NorthrendWormsTankHoldAction(botAI, Name, TrialOfTheCrusaderHelpers::BeastsTankDuty::WormMobile) {}
};

class NorthrendWormsTankHoldStationaryWormAction : public NorthrendWormsTankHoldAction
{
public:
    static constexpr char const* Name = "northrend worms tank hold stationary worm";

    NorthrendWormsTankHoldStationaryWormAction(PlayerbotAI* botAI)
        : NorthrendWormsTankHoldAction(botAI, Name, TrialOfTheCrusaderHelpers::BeastsTankDuty::WormStationary) {}
};

class NorthrendWormsRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "northrend worms redirect threat";

    NorthrendWormsRedirectThreatAction(PlayerbotAI* botAI) : RaidRedirectThreatAction(botAI, Name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

// Every worm hazard through one mover. The spot it walks to is held: a bot pushed off walks back to it
// rather than searching again from wherever it landed.
class NorthrendWormsRepositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "northrend worms reposition";

    NorthrendWormsRepositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
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
    static constexpr char const* Name = "northrend worms leave pool";

    NorthrendWormsLeavePoolAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
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
    static constexpr char const* Name = "northrend worms heal toxin carrier";

    NorthrendWormsHealToxinCarrierAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Divine Shield or Ice Block off the bot's own uncurable Toxin
class NorthrendWormsPurgeToxinAction : public Action
{
public:
    static constexpr char const* Name = "northrend worms purge toxin";

    NorthrendWormsPurgeToxinAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
