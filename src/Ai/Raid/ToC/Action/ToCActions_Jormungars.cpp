/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Jormungars.h"

#include <cmath>
#include <functional>
#include <vector>

#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCHelpers_Icehowl.h"
#include "ToCHelpers_Jormungars.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "Unit.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
constexpr float APPROACH_ARRIVE = 1.0f;
// The worm walks to its new spot 2.5 s into the submerge. Re-aiming on every step of that walk would
// churn the MotionMaster.
constexpr float APPROACH_REAIM_DISTANCE = 3.0f;
constexpr float SWEEP_RADIUS = 30.0f;
constexpr float SWEEP_DISTANCE_STEP = 2.0f;
constexpr float SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;
constexpr float WALK_SPOT_TOLERANCE = 0.5f;
// A spline issued last tick may not show in isMoving yet
constexpr uint32 WALK_STALL_MS = 500;
// The partner keeps moving, so a straight cure walk stops short of the full reach
constexpr float CURE_DIRECT_INSET = 1.0f;
constexpr float CURE_PARTIAL_TURN = static_cast<float>(M_PI) / 8.0f;
constexpr float FEET_CLEARANCE = 1.0f;

char const* const CLASS_TAUNTS[] = {"taunt", "hand of reckoning", "dark command", "growl"};

// CastSpell faces the target before the cast can fail, so a taunt on cooldown or out of range would
// turn the bot every tick
bool TryClassTaunt(PlayerbotAI* botAI, Player* bot, Unit* worm)
{
    for (char const* taunt : CLASS_TAUNTS)
    {
        if (!botAI->CanCastSpell(taunt, worm))
            continue;

        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", taunt)->Get();
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (info && bot->IsWithinCombatRange(worm, bot->GetSpellMaxRangeForTarget(worm, info)) &&
            bot->IsWithinLOSInMap(worm))
            return CastClassTaunt(botAI, worm);
    }

    return false;
}

bool BookedOn(LastMovement const& last, Position const& spot)
{
    return std::fabs(last.lastMoveToX - spot.GetPositionX()) <= WALK_SPOT_TOLERANCE &&
           std::fabs(last.lastMoveToY - spot.GetPositionY()) <= WALK_SPOT_TOLERANCE;
}

bool ClearOfCircles(std::vector<HazardCircle> const& circles, Position const& spot)
{
    for (HazardCircle const& circle : circles)
        if (circle.first.GetExactDist2d(spot) < circle.second)
            return false;

    return true;
}

bool OnWormFloor(float x, float y) { return ARENA_CENTER.GetExactDist2d(x, y) <= WORM_FLOOR_RADIUS; }

bool OnWormEdge(float x, float y)
{
    float const dist = ARENA_CENTER.GetExactDist2d(x, y);
    return dist >= WORM_EDGE_MIN && dist <= WORM_EDGE_MAX;
}

// A walk waits behind a booking of equal or higher priority until it runs out
void DropForeignBooking(LastMovement& last, Position const& own, MovementPriority upTo)
{
    if (last.msTime && last.priority <= upTo && !BookedOn(last, own))
        last.clear();
}

// The sweep's rays fan 22.5 degrees apart, so past ~30 yd they can straddle the partner's reach and
// find nothing
bool DirectCureSpot(Player* bot, WormMovePlan const& plan, Position& spot)
{
    Position const& partner = plan.partnerPos;
    float const reach = plan.partnerReach - CURE_DIRECT_INSET;
    float const step = 2.0f * static_cast<float>(M_PI) / WORM_CURE_DIRECT_BEARINGS;
    bool found = false;
    float nearest = 0.0f;
    for (uint32 bearing = 0; bearing < WORM_CURE_DIRECT_BEARINGS; ++bearing)
    {
        float const angle = step * bearing;
        Position const candidate(partner.GetPositionX() + reach * std::cos(angle),
                                 partner.GetPositionY() + reach * std::sin(angle), partner.GetPositionZ());
        float const walk = bot->GetExactDist2d(candidate);
        if ((found && walk >= nearest) || !WormSpotStillSafe(plan, candidate))
            continue;

        spot = candidate;
        nearest = walk;
        found = true;
    }

    return found;
}

// Partner out of search range and nothing clear round it: walk part of the way, bending off the
// straight line when that's blocked
bool PartialCureSpot(Player* bot, WormMovePlan const& plan, Position& spot)
{
    if (bot->GetExactDist2d(plan.partnerPos) <= WORM_SEARCH_RADIUS)
        return false;

    float const heading = bot->GetAngle(plan.partnerPos.GetPositionX(), plan.partnerPos.GetPositionY());
    for (float const turn : {0.0f, CURE_PARTIAL_TURN, -CURE_PARTIAL_TURN, 2.0f * CURE_PARTIAL_TURN,
                             -2.0f * CURE_PARTIAL_TURN})
    {
        float const x = bot->GetPositionX() + WORM_CURE_PARTIAL_STEP * std::cos(heading + turn);
        float const y = bot->GetPositionY() + WORM_CURE_PARTIAL_STEP * std::sin(heading + turn);
        Position const candidate(x, y, bot->GetPositionZ());
        if (ClearOfCircles(plan.circles, candidate) && (!plan.escapeAccept || plan.escapeAccept(x, y)))
        {
            spot = candidate;
            return true;
        }
    }

    return false;
}

// First tier with a spot: stand, escape when starting in a pool, full, pair, nospread, partner, escape,
// loose, direct, partial, cross
bool FindWormSpot(Player* bot, WormMovePlan const& plan, Position& spot, WormSpotTier& tier)
{
    auto const fits = [&plan](float x, float y)
    { return (!plan.accept || plan.accept(x, y)) && (!plan.roleAccept || plan.roleAccept(x, y)); };

    if (plan.hasStand && ClearOfCircles(plan.circles, plan.stand) && ClearOfCircles(plan.spreadCircles, plan.stand) &&
        fits(plan.stand.GetPositionX(), plan.stand.GetPositionY()))
    {
        spot = plan.stand;
        tier = WormSpotTier::Stand;
        return true;
    }

    HazardSweepCache sweep;
    // The sweep returns nothing for an empty list, and urgency from a Spew cone alone leaves the escape
    // no circle
    std::vector<HazardCircle> const ownSpot = {{bot->GetPosition(), 0.0f}};
    auto const search = [&](std::vector<HazardCircle> const& hazards, std::function<bool(float, float)> const& accept,
                            WormSpotTier found)
    {
        Position const clear =
            FindNearestPositionClearOfHazards(bot, hazards.empty() ? ownSpot : hazards, WORM_SEARCH_RADIUS,
                                              SWEEP_DISTANCE_STEP, SWEEP_ANGLE_STEP, &plan.preferNear, accept, &sweep);
        if (clear == Position())
            return false;

        spot = clear;
        tier = found;
        return true;
    };

    if (plan.escapeFirst && search(plan.escapeCircles, plan.escapeAccept, WormSpotTier::Escape))
        return true;

    std::vector<HazardCircle> withSpread = plan.circles;
    withSpread.insert(withSpread.end(), plan.spreadCircles.begin(), plan.spreadCircles.end());
    if (search(withSpread, fits, WormSpotTier::Full))
        return true;

    if (plan.pairable)
    {
        std::vector<HazardCircle> withPair = plan.circles;
        withPair.insert(withPair.end(), plan.pairCircles.begin(), plan.pairCircles.end());
        if (search(withPair, fits, WormSpotTier::Pair))
            return true;
    }

    if (!plan.spreadStrict && !plan.spreadCircles.empty() && search(plan.circles, fits, WormSpotTier::NoSpread))
        return true;

    bool const cureWalk = IsWormCureReason(plan.reason);
    if (cureWalk && search(plan.circles, plan.accept, WormSpotTier::Partner))
        return true;

    if (plan.urgent)
    {
        if (!plan.escapeFirst && search(plan.escapeCircles, plan.escapeAccept, WormSpotTier::Escape))
            return true;

        // Same list as the escape's unless a held out Bile carrier keeps off clean bots
        if (plan.looseCircles.size() != plan.escapeCircles.size() &&
            search(plan.looseCircles, plan.escapeAccept, WormSpotTier::Loose))
            return true;
    }

    if (cureWalk)
    {
        if (DirectCureSpot(bot, plan, spot))
        {
            tier = WormSpotTier::Direct;
            return true;
        }

        if (PartialCureSpot(bot, plan, spot))
        {
            tier = WormSpotTier::Partial;
            return true;
        }
    }

    // Standing still in the damage is worse than walking through it. Near the floor edge, beside the
    // up stationary worm or between two pools, every walk out can fail the path test.
    return plan.urgent && (search(plan.escapeCircles, plan.crossAccept, WormSpotTier::Cross) ||
                           (plan.looseCircles.size() != plan.escapeCircles.size() &&
                            search(plan.looseCircles, plan.crossAccept, WormSpotTier::Cross)));
}
}  // namespace

bool NorthrendWormsTankHoldAction::Execute(Event /*event*/)
{
    Unit* worm = GetBeastOfDuty(botAI, _duty);
    if (!worm)
    {
        _walking = WalkKind::None;
        return false;
    }

    bool const mobile = _duty == BeastsTankDuty::WormMobile;

    if (IsWormSubmerged(worm))
        return ApproachSubmerged(worm);

    if (_walking == WalkKind::Approach)
        _walking = WalkKind::None;

    // The submerge wipes its threat list, so it comes up on whoever threatens it first
    if (worm->GetVictim() != bot && TryClassTaunt(botAI, bot, worm))
        return true;

    if (AI_VALUE(Unit*, "current target") != worm)
        return Attack(worm);

    return mobile && DragOffHazards(worm);
}

// By the form it surfaces in: mobile needs taunt range, a rooted stationary one melee of where it comes
// up. A lone survivor's holder keeps the mobile duty through its stationary emerges.
bool NorthrendWormsTankHoldAction::ApproachSubmerged(Unit* worm)
{
    float const stand = IsWormMobile(worm) ? WORM_TAUNT_STAND : WORM_STATIONARY_MELEE_RANGE - 2.0f;

    Position chargeStart;
    Position chargeEnd;
    if (bot->GetExactDist2d(worm) <= stand + APPROACH_ARRIVE || IcehowlChargeLatched(botAI, chargeStart, chargeEnd))
    {
        _walking = WalkKind::None;
        return false;
    }

    if (WalkInFlight(WalkKind::Approach) && worm->GetExactDist2d(_approachFrom) < APPROACH_REAIM_DISTANCE)
        return true;

    float const angle = worm->GetAngle(bot);
    Position const spot(worm->GetPositionX() + stand * std::cos(angle),
                        worm->GetPositionY() + stand * std::sin(angle), worm->GetPositionZ());
    if (!WalkTo(WalkKind::Approach, spot))
        return false;

    _approachFrom = worm->GetPosition();
    return true;
}

bool NorthrendWormsTankHoldAction::DragOffHazards(Unit* worm)
{
    Position chargeStart;
    Position chargeEnd;
    WormDragPlan plan;
    if (IcehowlChargeLatched(botAI, chargeStart, chargeEnd) || !GetMobileWormDrag(botAI, worm, plan))
    {
        _walking = WalkKind::None;
        return false;
    }

    Position const* inFlight = WalkInFlight(WalkKind::Drag);
    if (inFlight && ClearOfCircles(plan.poolsOnly, *inFlight))
        return true;

    // Toxin carriers walking over for this tank's Bile need it to stay put a bit
    if (!inFlight && MobileWormDragWaits(botAI))
        return false;

    HazardSweepCache sweep;
    Position spot;
    auto const search = [&](std::vector<HazardCircle> hazards, bool onEdge)
    {
        if (spot != Position() || (onEdge && !plan.edge))
            return;

        // The sweep finds nothing for an empty list, as on a heroic walk out before the first pool
        hazards.emplace_back(bot->GetPosition(), FEET_CLEARANCE);
        spot = FindNearestPositionClearOfHazards(bot, hazards, SWEEP_RADIUS, SWEEP_DISTANCE_STEP, SWEEP_ANGLE_STEP,
                                                 &plan.preferNear, onEdge ? OnWormEdge : OnWormFloor, &sweep);
    };

    // Heroic tries the edge first. Beside the stationary worm its melee would stand in Sweep, so keeping
    // off that one beats the edge.
    search(plan.hazards, true);
    search(plan.hazards, false);
    search(plan.poolsOnly, true);
    search(plan.poolsOnly, false);
    if (spot == Position())
        return false;

    return WalkTo(WalkKind::Drag, spot);
}

bool NorthrendWormsTankHoldAction::BookedOnWalkSpot()
{
    return BookedOn(AI_VALUE(LastMovement&, "last movement"), _walkSpot);
}

Position const* NorthrendWormsTankHoldAction::WalkInFlight(WalkKind kind)
{
    // Moving on some other node's walk doesn't count
    if (_walking != kind || !BookedOnWalkSpot())
        return nullptr;

    if (!bot->isMoving() && getMSTimeDiff(_walkIssuedMs, getMSTime()) >= WALK_STALL_MS)
        return nullptr;

    return &_walkSpot;
}

bool NorthrendWormsTankHoldAction::WalkTo(WalkKind kind, Position const& spot)
{
    // A walk issued mid cast never starts but still books its travel time, and the same spot is
    // refused until that runs out. Wait for the cast to end instead.
    if (bot->IsMovementPreventedByCasting())
        return false;

    // Our own earlier walk holds the booking at this priority and would refuse its replacement
    if (BookedOnWalkSpot())
        AI_VALUE(LastMovement&, "last movement").clear();

    _walkSpot = spot;
    _walking = kind;
    _walkIssuedMs = getMSTime();
    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT);
}

Player* NorthrendWormsRedirectThreatAction::GetRedirectTank()
{
    Player* holder = nullptr;
    return GetWormRedirectTarget(botAI, holder) ? holder : nullptr;
}

// Misdirection and Tricks go out under ground, but the shots wait for the worm to come up
Unit* NorthrendWormsRedirectThreatAction::GetThreatDumpTarget()
{
    Player* holder = nullptr;
    Unit* worm = GetWormRedirectTarget(botAI, holder);
    return worm && !IsWormSubmerged(worm) ? worm : nullptr;
}

bool NorthrendWormsRepositionAction::Execute(Event /*event*/)
{
    WormMovePlan plan;
    if (!GetWormMovePlan(botAI, plan) || plan.reason == WormMoveReason::None)
        return false;

    if (!botAI->CanMove())
        return false;

    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    Position spot;
    WormMoveReason reason = plan.reason;
    WormSpotTier tier = WormSpotTier::Full;
    if (plan.held)
    {
        // Every MoveTo clears the MotionMaster, so the walk still carrying the bot to its spot is left alone
        uint32 const age = getMSTimeDiff(plan.hold.issuedMs, getMSTime());
        if (BookedOn(last, plan.hold.spot) && age < WORM_WALK_LATCH_MS && (bot->isMoving() || age < WALK_STALL_MS))
            return true;

        spot = plan.hold.spot;
        reason = plan.hold.reason;
        tier = plan.hold.tier;
    }

    // PointMovementGenerator never starts a spline under a cast that pins the feet, and only damage
    // landing on the spot is worth losing the cast for
    bool const pinned = bot->IsMovementPreventedByCasting();
    if (pinned && !plan.urgent)
        return false;

    if (!plan.held && !FindWormSpot(bot, plan, spot, tier))
    {
        if (plan.urgent)
            return true;

        BackOffWormSearch(botAI);
        return false;
    }

    if (pinned)
        bot->InterruptNonMeleeSpells(true);

    // Our own stale walk holds the booking and would refuse its replacement
    if ((_hasIssuedSpot && BookedOn(last, _issuedSpot)) || (plan.held && BookedOn(last, plan.hold.spot)))
        last.clear();

    DropForeignBooking(last, spot, MovementPriority::MOVEMENT_COMBAT);

    MovementPriority const priority =
        plan.urgent || IsWormCureReason(reason) ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;
    float const travel = bot->GetExactDist2d(spot);
    switch (TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                      false, true, priority, true))
    {
        case MoveOutcome::Issued:
            SetWormHold(botAI, spot, reason, tier, false);
            _issuedSpot = spot;
            _hasIssuedSpot = true;
            // after the walk is issued: the boots refuse a bot mid cast
            if (IsWormCureReason(reason) && travel > WORM_CURE_NITRO_FROM)
                UseNitroBoosts(bot);

            return true;
        case MoveOutcome::AlreadyThere:
            if (WormHoldParks(reason, tier))
                SetWormHold(botAI, spot, reason, tier, true);

            return false;
        default:
            return plan.urgent;
    }
}

bool NorthrendWormsLeavePoolAction::Execute(Event /*event*/)
{
    if (!InSlimePoolWithoutWorms(botAI))
    {
        _hasExit = false;
        return false;
    }

    if (!botAI->CanMove())
        return false;

    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    uint32 const now = getMSTime();
    if (_hasExit)
    {
        // Every MoveTo clears the MotionMaster, so a walk still carrying the bot out is left alone
        uint32 const age = getMSTimeDiff(_exitMs, now);
        bool const exitClear = ClearOfCircles(GetSlimePoolCircles(botAI, 0, WORM_POOL_TRIGGER_PAD), _exitSpot);
        if (exitClear && BookedOn(last, _exitSpot) && age < WORM_WALK_LATCH_MS &&
            (bot->isMoving() || age < WALK_STALL_MS))
            return true;

        // Our own stale walk holds the booking and would refuse its replacement
        if (BookedOn(last, _exitSpot))
            last.clear();

        _hasExit = false;
    }

    Position spot;
    if (!FindSlimePoolExit(botAI, spot))
        return false;

    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);

    DropForeignBooking(last, spot, MovementPriority::MOVEMENT_COMBAT);
    switch (TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true))
    {
        case MoveOutcome::Issued:
            _exitSpot = spot;
            _exitMs = now;
            _hasExit = true;
            return true;
        case MoveOutcome::AlreadyThere:
            return false;
        default:
            return true;
    }
}

bool NorthrendWormsHealToxinCarrierAction::Execute(Event /*event*/)
{
    Player* carrier = GetWormToxinHealTarget(botAI);
    return carrier && CastRaidThroughputHeal(botAI, carrier);
}

bool NorthrendWormsPurgeToxinAction::Execute(Event /*event*/)
{
    char const* purge = GetWormToxinSelfPurge(botAI);
    return purge && botAI->CastSpell(purge, bot);
}
