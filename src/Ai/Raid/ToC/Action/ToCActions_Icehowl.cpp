/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Icehowl.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Creature.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "RaidTankDefensive.h"
#include "Timer.h"
#include "ToCHelpers_Icehowl.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
constexpr float DODGE_SEARCH_RADIUS = 30.0f;
constexpr float DODGE_ARRIVE = 1.0f;
// Ceiling on holding the tick for one walk, so a bot rooted mid walk still gets a fresh spot
constexpr uint32 DODGE_LATCH_MS = 5000;
// A spline issued last tick may not show in isMoving yet
constexpr uint32 DODGE_STALL_MS = 500;
constexpr float DODGE_SPOT_TOLERANCE = 0.5f;
// Circles along the whole line, so the sweep throws out a spot inside the corridor before paying for
// its collision check. At this spacing they miss under 0.2 yd of it, and accept catches the rest.
constexpr float LANE_CIRCLE_SPACING = 4.0f;
// The daze's length: one try per daze, since a use that fails leaves the boots ready and the trigger
// would take every tick from the walk
constexpr uint32 DAZE_NITRO_RETRY_MS = 15000;

std::vector<HazardCircle> LaneCircles(Position const& start, Position const& end, float clearance)
{
    float const dx = end.GetPositionX() - start.GetPositionX();
    float const dy = end.GetPositionY() - start.GetPositionY();
    float const length = std::hypot(dx, dy);
    uint32 const steps = std::max<uint32>(1, static_cast<uint32>(std::ceil(length / LANE_CIRCLE_SPACING)));

    std::vector<HazardCircle> circles;
    circles.reserve(steps + 1);
    for (uint32 i = 0; i <= steps; ++i)
    {
        float const t = static_cast<float>(i) / static_cast<float>(steps);
        circles.emplace_back(
            Position(start.GetPositionX() + t * dx, start.GetPositionY() + t * dy, start.GetPositionZ()), clearance);
    }

    return circles;
}

Position FindLaneClearSpot(Player* bot, Position const& start, Position const& end, float clearance,
                           std::vector<HazardCircle> const& pools, HazardSweepCache* sweep)
{
    std::vector<HazardCircle> hazards = LaneCircles(start, end, clearance);
    hazards.insert(hazards.end(), pools.begin(), pools.end());
    return FindNearestPositionClearOfHazards(
        bot, hazards, DODGE_SEARCH_RADIUS, 2.0f, static_cast<float>(M_PI) / 8.0f, nullptr,
        [&start, &end, clearance](float x, float y) { return DistanceToIcehowlLane(start, end, x, y) >= clearance; },
        sweep);
}

bool InAnyCircle(std::vector<HazardCircle> const& circles, Position const& at)
{
    return std::any_of(circles.begin(), circles.end(),
                       [&at](HazardCircle const& circle) { return circle.first.GetExactDist2d(at) < circle.second; });
}
}  // namespace

bool IcehowlTankHoldBossAction::Execute(Event /*event*/)
{
    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    if (!icehowl)
        return false;

    // Passive only inside the charge cycle, where he takes no victim at all
    Creature* creature = icehowl->ToCreature();
    bool const passive = creature && creature->GetReactState() == REACT_PASSIVE;
    Unit* victim = icehowl->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    bool const victimIsTank = victimPlayer && IsBeastsTank(victimPlayer);
    if (!passive && !victimIsTank && CastClassTaunt(botAI, icehowl))
        return true;

    if (AI_VALUE(Unit*, "current target") != icehowl)
        return Attack(icehowl);

    return false;
}

bool IcehowlClearChargePathAction::Execute(Event /*event*/)
{
    Position start;
    Position end;
    if (!IcehowlChargeLatched(botAI, start, end))
    {
        _hasDodgeSpot = false;
        return false;
    }

    // Massive Crash keeps the whole raid stunned through the gaze
    if (!botAI->CanMove())
        return false;

    float const fromLane = DistanceToIcehowlLane(start, end, bot->GetPositionX(), bot->GetPositionY());
    bool const inLane = fromLane < ICEHOWL_TRAMPLE_RADIUS;
    uint32 const now = getMSTime();

    // Every MoveTo clears the MotionMaster, so a walk still carrying the bot out is left alone.
    bool replacing = false;
    if (_hasDodgeSpot)
    {
        uint32 const age = getMSTimeDiff(_dodgeSpotMs, now);
        bool const spotClear = DistanceToIcehowlLane(start, end, _dodgeSpot.GetPositionX(),
                                                     _dodgeSpot.GetPositionY()) >= ICEHOWL_CHARGE_TIGHT_CLEARANCE;
        if (age < DODGE_LATCH_MS && spotClear)
        {
            if (bot->GetExactDist2d(_dodgeSpot.GetPositionX(), _dodgeSpot.GetPositionY()) <= DODGE_ARRIVE)
                return false;

            // A walk some other node issued since isn't ours, and it can be heading into the lane
            LastMovement& last = AI_VALUE(LastMovement&, "last movement");
            bool const bookedHere = std::fabs(last.lastMoveToX - _dodgeSpot.GetPositionX()) <= DODGE_SPOT_TOLERANCE &&
                                    std::fabs(last.lastMoveToY - _dodgeSpot.GetPositionY()) <= DODGE_SPOT_TOLERANCE;
            if (!_dodgeParked && bookedHere && !bot->IsMovementPreventedByCasting() &&
                (bot->isMoving() || age < DODGE_STALL_MS))
                return true;
        }

        replacing = !_dodgeParked;
        _hasDodgeSpot = false;
    }

    HazardSweepCache sweep;
    // The worms' pools can outlive them into this fight
    std::vector<HazardCircle> const pools = GetSlimePoolCircles(botAI, WORM_POOL_SOON_TICKS, WORM_POOL_CLEARANCE_PAD);
    Position spot = FindLaneClearSpot(bot, start, end, ICEHOWL_CHARGE_CLEARANCE, pools, &sweep);
    if (spot == Position())
    {
        // Already past the tight line, which is all the fallback could buy. Parked, so the next tick
        // skips the sweep.
        if (fromLane >= ICEHOWL_CHARGE_TIGHT_CLEARANCE && !InAnyCircle(pools, bot->GetPosition()))
        {
            _dodgeSpot = bot->GetPosition();
            _dodgeSpotMs = now;
            _hasDodgeSpot = true;
            _dodgeParked = true;
            return false;
        }

        spot = FindLaneClearSpot(bot, start, end, ICEHOWL_CHARGE_TIGHT_CLEARANCE, pools, &sweep);
        // Trample is 50k, a pool tick 8-10k
        if (spot == Position() && !pools.empty())
            spot = FindLaneClearSpot(bot, start, end, ICEHOWL_CHARGE_TIGHT_CLEARANCE, {}, &sweep);
    }

    if (spot == Position())
        return inLane;

    // PointMovementGenerator never starts a spline under a cast that pins the feet
    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);

    switch (IssueDodge(spot, replacing))
    {
        case MoveOutcome::Issued:
            break;
        case MoveOutcome::AlreadyThere:
            return false;
        default:
            return inLane;
    }

    _dodgeSpot = spot;
    _dodgeSpotMs = now;
    _hasDodgeSpot = true;
    _dodgeParked = false;

    return true;
}

MoveOutcome IcehowlClearChargePathAction::IssueDodge(Position const& spot, bool replacing)
{
    // Our own stale walk holds the booking at this priority and would refuse its replacement.
    if (replacing)
        AI_VALUE(LastMovement&, "last movement").clear();

    auto const issue = [this, &spot]()
    {
        return TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false,
                         false, false, true, MovementPriority::MOVEMENT_FORCED, true);
    };

    MoveOutcome const outcome = issue();
    if (outcome != MoveOutcome::Duplicate && outcome != MoveOutcome::Waiting)
        return outcome;

    if (bot->isMoving())
        return outcome == MoveOutcome::Duplicate ? MoveOutcome::Issued : outcome;

    // The knockback or stun ended the walk that still holds the booking, and the booking would refuse
    // this one until it expires.
    AI_VALUE(LastMovement&, "last movement").clear();
    return issue();
}

bool IcehowlTankDefensiveAction::Execute(Event /*event*/)
{
    uint32 const defensive = NextTankDefensive(botAI, bot);
    return defensive && botAI->CastSpell(defensive, bot);
}

bool IcehowlMoveToBreathStandAction::Execute(Event /*event*/)
{
    IcehowlBreathStand stand;
    if (!GetIcehowlBreathStand(botAI, stand) ||
        IsOnIcehowlBreathStand(stand, bot->GetPositionX(), bot->GetPositionY(), ICEHOWL_SPREAD_ARRIVE_DEG,
                               stand.arrive))
    {
        DropWalk();
        return false;
    }

    if (Position const* spot = WalkInFlight())
        if (IsOnIcehowlBreathStand(stand, spot->GetPositionX(), spot->GetPositionY(), ICEHOWL_SPREAD_TRIGGER_DEG,
                                   stand.trigger))
            return true;

    // A breath is healable, so a pinning cast finishes first
    return WalkTo(stand.spot);
}

bool IcehowlDazeNitroAction::isUseful()
{
    return !_tried || getMSTimeDiff(_triedMs, getMSTime()) >= DAZE_NITRO_RETRY_MS;
}

bool IcehowlDazeNitroAction::Execute(Event /*event*/)
{
    _tried = true;
    _triedMs = getMSTime();

    Unit* icehowl = GetEngagedBeast(botAI, NorthrendBeast::Icehowl);
    return icehowl && UseNitroBoosts(bot);
}
