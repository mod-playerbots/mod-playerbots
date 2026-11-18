/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_JORMUNGARS_H
#define PLAYERBOTS_RAID_TOCHELPERS_JORMUNGARS_H

#include <functional>
#include <vector>

#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "ToCData.h"

// Acidmaw and Dreadscale, read once per instance per ms while a worm is engaged. The pools are read with
// none engaged too, while the Beasts gate is open.
namespace TrialOfTheCrusaderHelpers
{

// Worm display ids per mobility form (boss_northrend_beasts.cpp Model)
enum class ToCDisplayIds : uint32
{
    MODEL_ACIDMAW_STATIONARY    = 29815,
    MODEL_ACIDMAW_MOBILE        = 29816,
    MODEL_DREADSCALE_STATIONARY = 26935,
    MODEL_DREADSCALE_MOBILE     = 24564,
};

// 10N ids; remap at the call where a row exists
inline constexpr uint32 SPELL_PARALYTIC_TOXIN = 66823;   // row 605
inline constexpr uint32 SPELL_BURNING_BILE = 66869;      // no row
inline constexpr uint32 SPELL_ACIDIC_SPEW = 66818;       // no row: the cast and the worm's 2.5 s aura
inline constexpr uint32 SPELL_MOLTEN_SPEW = 66821;       // no row
inline constexpr uint32 SPELL_ACIDIC_SPEW_TICK = 66819;  // row 602
inline constexpr uint32 SPELL_MOLTEN_SPEW_TICK = 66820;  // row 611
inline constexpr uint32 SPELL_SLIME_POOL_AURA = 66882;   // no row, one tick a second grows the pool
inline constexpr uint32 SPELL_PARALYTIC_SPRAY = 66901;   // row 604
inline constexpr uint32 SPELL_BURNING_SPRAY = 66902;     // row 608

// Reach follows the display, scale 1: mobile 24564 and 29816, stationary 26935 and 29815
inline constexpr float WORM_MOBILE_COMBAT_REACH = 12.0f;
inline constexpr float WORM_STATIONARY_COMBAT_REACH = 5.0f;
inline constexpr float WORM_MOBILE_MELEE_RANGE = PlayerMeleeRange(WORM_MOBILE_COMBAT_REACH);
inline constexpr float WORM_STATIONARY_MELEE_RANGE = PlayerMeleeRange(WORM_STATIONARY_COMBAT_REACH);
// The melee clump round the up mobile worm: its dealt stands plus a yard
inline constexpr float WORM_MELEE_RING = 9.3f;
inline constexpr float WORM_SPEW_RANGE = 55.0f;
inline constexpr float WORM_FLOOR_RADIUS = 35.0f;        // the worms surface up to 35 yd from ARENA_CENTER
// Bot spots only. 2 yd inside Icehowl's gate-side charge end, so the floor is known to reach it.
inline constexpr float WORM_SPOT_FLOOR_RADIUS = 44.0f;
inline constexpr float WORM_TAUNT_STAND = 20.0f;         // the mobile holder's reach under ground
// Mobile worm centre to its tank once it settles: inside melee range the chase stops at its next 400 ms
// check, short of contact plus both reaches
inline constexpr float WORM_TANK_OFFSET = 10.0f;

// Sweep: a 15 yd circle round the stationary worm, centre to centre
inline constexpr float WORM_SWEEP_RADIUS = 15.0f;
inline constexpr float WORM_SWEEP_TRIGGER = 16.0f;
inline constexpr float WORM_SWEEP_CLEARANCE = 18.0f;

// Burning Bile's pulse and the Spray's splash, both 10 yd centre to centre
inline constexpr float WORM_SPLASH_RADIUS = 10.0f;
inline constexpr float WORM_BILE_TRIGGER = 10.5f;
inline constexpr float WORM_BILE_CLEARANCE = 12.0f;
inline constexpr float WORM_SPREAD_TRIGGER = 10.0f;
inline constexpr float WORM_SPREAD_CLEARANCE = 13.0f;
// Spray's splash lands where its target stands as the cast ends
inline constexpr float WORM_SPRAY_TRIGGER = WORM_SPLASH_RADIUS + 2.0f;
inline constexpr float WORM_SPRAY_CLEARANCE = WORM_SPLASH_RADIUS + 3.0f;
// Casters keep off the melee round the up mobile worm as one circle on it
inline constexpr float WORM_MELEE_STACK_TRIGGER = WORM_MELEE_RING + WORM_SPREAD_TRIGGER;
inline constexpr float WORM_MELEE_STACK_CLEARANCE = WORM_MELEE_RING + WORM_SPREAD_CLEARANCE;
// Off the melee group itself, in every tier, spread given up or not
inline constexpr float WORM_CASTER_CLUMP_CLEARANCE = WORM_MELEE_RING + 2.0f;

// Dealt melee stands on the mobile worm
inline constexpr float WORM_MELEE_STAND = WORM_MELEE_RING - 1.0f;  // from its centre
inline constexpr float WORM_MELEE_GROUP_DEG = 54.0f;                // either side of its back: groups 13.4 yd apart
inline constexpr float WORM_MELEE_REAR_ARC_DEG = 108.0f;            // least angle off its facing for a melee spot

// A Toxin carrier stands this close to a Bile carrier, well inside the pulse
inline constexpr float WORM_CURE_REACH = 6.0f;
inline constexpr float WORM_CURE_TRIGGER = 8.0f;
// A runner's band round a carrier that can't move anymore. Further out still finds a spot when the
// carrier stands in a pool.
inline constexpr float WORM_RUN_REACH = 8.5f;
inline constexpr float WORM_RUN_TRIGGER = 9.0f;
// A cure or run walk past this spends Nitro Boosts: Toxin slows harder every second, so a long walk ends
// in Paralysis
inline constexpr float WORM_CURE_NITRO_FROM = 15.0f;
// Toxin's snare amount (EFFECT_0) at which its carrier waits for a runner instead of walking
inline constexpr int32 WORM_TOXIN_STUCK_SLOW = -70;
// A carrier no Bile can cure gets its healer's heal under this health
inline constexpr float WORM_TOXIN_HEAL_PCT = 80.0f;

// Held spots
inline constexpr float WORM_HOLD_ARRIVE = 1.0f;
inline constexpr float WORM_HOLD_TRIGGER = 2.5f;           // a parked bot pushed past this walks back
inline constexpr float WORM_HOLD_MELEE_DRIFT = 2.5f;       // anchor worm moved, melee hold
inline constexpr float WORM_HOLD_MELEE_TURN_DEG = 20.0f;   // anchor worm turned, melee hold
inline constexpr float WORM_HOLD_DRIFT = 6.0f;             // anchor worm moved, any other hold
inline constexpr uint32 WORM_HOLD_FRESH_MS = 1000;         // a hold the plan hasn't confirmed this long guards nothing
inline constexpr uint32 WORM_SEARCH_BACKOFF_MS = 1000;     // after a non-urgent search found no spot
// Reach into a pool or a Bile pulse stays zeroed this long through backed off searches, then walks in
inline constexpr uint32 WORM_REACH_HOLD_MS = 4000;

// A worm counts as settled once it hasn't walked or turned past these for WORM_SETTLE_MS
inline constexpr uint32 WORM_SETTLE_MS = 500;
inline constexpr float WORM_SETTLE_MOVE = 0.5f;
inline constexpr float WORM_SETTLE_TURN_DEG = 5.0f;

// The script's Spew: 11 s after the engage, at once on a mobile emerge, then 22 s after each cast start
inline constexpr uint32 WORM_SPEW_FIRST_MS = 11000;
inline constexpr uint32 WORM_SPEW_REPEAT_MS = 22000;
inline constexpr uint32 WORM_SPEW_LEAD_MS = 4000;

// Path test
inline constexpr float WORM_POOL_PATH_PAD = 1.0f;
inline constexpr float WORM_SWEEP_PATH_RADIUS = WORM_SWEEP_TRIGGER;

// Search
inline constexpr float WORM_SEARCH_RADIUS = 30.0f;
inline constexpr float WORM_PREFER_JITTER = 0.5f;
inline constexpr uint32 WORM_CURE_DIRECT_BEARINGS = 16;
inline constexpr float WORM_CURE_PARTIAL_STEP = 25.0f;

// Spew pads past the half-arc: stepping out triggers at the first, and the spot clears the second
inline constexpr float WORM_SPEW_TRIGGER_PAD = 5.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float WORM_SPEW_CLEAR_PAD = 12.0f * static_cast<float>(M_PI) / 180.0f;
// TARGET_UNIT_CONE_ENEMY_24, for the ids spell_cone has no row for
inline constexpr float WORM_SPEW_DEFAULT_HALF_ARC = 12.0f * static_cast<float>(M_PI) / 180.0f;

// Slime Pool radius is 2 + 0.3 x tick, centre to centre, till the pool despawns: 30 s on normal (11 yd),
// 45 s on 10H, 60 s on 25H (20 yd)
inline constexpr float WORM_POOL_BASE_RADIUS = 2.0f;
inline constexpr float WORM_POOL_GROWTH_PER_TICK = 0.3f;
inline constexpr uint32 WORM_POOL_SOON_TICKS = 5;

// Heroic pools outlive the 12 s between them, so the mobile holder walks its worm round the edge and
// they line the wall: its drag looks on this band first, 2 yd inside the spot floor
inline constexpr float WORM_EDGE_MIN = 36.0f;
inline constexpr float WORM_EDGE_MAX = WORM_SPOT_FLOOR_RADIUS - 2.0f;
// Further in, a heroic worm gets walked out before its next pool. It settles WORM_TANK_OFFSET short of
// its tank, so one walked out from the middle ends past this.
inline constexpr float WORM_EDGE_WORM_MIN = WORM_EDGE_MIN - WORM_TANK_OFFSET - 2.0f;
inline constexpr float WORM_POOL_TRIGGER_PAD = 1.0f;    // on the radius now
inline constexpr float WORM_POOL_CLEARANCE_PAD = 2.0f;  // on the radius 5 s on

// A reposition walk holds the tick at most this long, so a bot rooted mid walk still gets a new spot
inline constexpr uint32 WORM_WALK_LATCH_MS = 5000;
// A pool at the worm's feet takes ~20 s to reach the melee stands, so the drag can wait this long
inline constexpr uint32 WORM_DRAG_CURE_WAIT_MS = 6000;

static_assert(WORM_SWEEP_RADIUS < WORM_SWEEP_TRIGGER && WORM_SWEEP_TRIGGER < WORM_SWEEP_CLEARANCE, "sweep band");
static_assert(WORM_SPLASH_RADIUS < WORM_BILE_TRIGGER && WORM_BILE_TRIGGER < WORM_BILE_CLEARANCE, "bile band");
static_assert(WORM_SPREAD_TRIGGER < WORM_SPREAD_CLEARANCE, "spread band");
static_assert(WORM_SPRAY_TRIGGER < WORM_SPRAY_CLEARANCE, "spray band");
static_assert(WORM_SPEW_LEAD_MS < WORM_SPEW_FIRST_MS, "spew lead");
static_assert(WORM_EDGE_MIN < WORM_EDGE_MAX, "edge band");
static_assert(WORM_MELEE_STACK_TRIGGER < WORM_MELEE_STACK_CLEARANCE, "melee stack band");
static_assert(WORM_POOL_TRIGGER_PAD < WORM_POOL_CLEARANCE_PAD, "pool band");
static_assert(WORM_SPEW_TRIGGER_PAD < WORM_SPEW_CLEAR_PAD, "spew band");
static_assert(WORM_CURE_REACH < WORM_CURE_TRIGGER && WORM_CURE_TRIGGER < WORM_SPLASH_RADIUS,
              "a cure walk ends inside its trigger, and the trigger inside the pulse");
static_assert(WORM_CURE_REACH < WORM_RUN_REACH && WORM_RUN_REACH < WORM_RUN_TRIGGER &&
                  WORM_RUN_TRIGGER < WORM_SPLASH_RADIUS,
              "the far run band sits past the cure band and inside the pulse");
static_assert(WORM_CASTER_CLUMP_CLEARANCE < WORM_MELEE_STACK_TRIGGER,
              "the melee group circle sits inside the casters' spread circle on the worm");
static_assert(WORM_HOLD_ARRIVE < WORM_HOLD_TRIGGER, "a walk has to end inside its own trigger");
static_assert(WORM_MELEE_STAND <= WORM_MOBILE_MELEE_RANGE - 1.0f, "the melee stand is inside the role reach");
static_assert(180.0f - WORM_MELEE_GROUP_DEG >= WORM_MELEE_REAR_ARC_DEG, "the dealt stands sit in the rear arc");

bool IsWormSubmerged(Unit* worm);                 // UNIT_FLAG_NOT_SELECTABLE, submerge start to emerge
// Display form. Under ground, the form it comes up in once the other worm is under too; while the
// other is still up, the opposite of that one's. A survivor flips at every emerge too.
bool IsWormMobile(Unit* worm);
// A player's melee range by that form: under ground the transform model carries its own reach
float WormMeleeRange(Unit* worm);
float WormSpewHalfArc(PlayerbotAI* botAI);        // radians, half the cone the remapped tick uses

struct WormDragPlan
{
    std::vector<EncounterHelpers::HazardCircle> hazards;    // pools and the stationary worm
    std::vector<EncounterHelpers::HazardCircle> poolsOnly;  // fallback sweep
    Position preferNear;                                     // 10 yd on from the tank, away from the worm
    bool edge = false;                                       // heroic: the WORM_EDGE band first
};
// The mobile worm this bot holds must be walked out: true fills the plan for the tank's own spot
bool GetMobileWormDrag(PlayerbotAI* botAI, Unit* worm, WormDragPlan& plan);
// A drag this holder needs waits up to WORM_DRAG_CURE_WAIT_MS while Toxin carriers walk to it for a cure.
// Ask it only before starting a drag, never mid walk.
bool MobileWormDragWaits(PlayerbotAI* botAI);

// The worm this hunter or rogue redirects onto its duty holder right now, else nullptr
Unit* GetWormRedirectTarget(PlayerbotAI* botAI, Player*& holder);

// A bot Burning Bile carrier with no Beasts tank duty, runners too: reach melee would walk it back onto
// the stack, and a runner whose search finds nothing stands still instead
bool IsWormBileCarrierHeldOut(PlayerbotAI* botAI);

// In priority order: the first rule whose trigger the bot's spot fails names the move
enum class WormMoveReason : uint8
{
    None,
    Cure,
    Run,
    Pool,
    Spray,
    Bile,
    Sweep,
    Clump,  // a caster inside the melee group round the mobile worm
    Spew,
    Spread,
    Melee,
    Reach
};
char const* WormMoveReasonName(WormMoveReason reason);  // "cure", "run", "pool", ...

// The search that found a spot. Escape, loose, partial and cross spots are walked to, never parked on.
enum class WormSpotTier : uint8
{
    Stand,
    Full,
    Pair,  // spread from every caster but one with no other neighbour
    NoSpread,
    Partner,
    Direct,
    Partial,
    Escape,
    Loose,
    Cross  // urgent and nothing else found: the walk may cross a pool or Sweep
};
bool WormTierParks(WormSpotTier tier);
// A Spray dodge lets go once it arrives, so melee walk back to their stand after the cast
bool WormHoldParks(WormMoveReason reason, WormSpotTier tier);
bool IsWormCureReason(WormMoveReason reason);     // cure or run

struct WormHoldView
{
    Position spot;
    WormMoveReason reason = WormMoveReason::None;
    WormSpotTier tier = WormSpotTier::Full;
    uint32 issuedMs = 0;
    bool parked = false;
};

struct WormMovePlan
{
    WormMoveReason reason = WormMoveReason::None;
    bool urgent = false;        // damage landing on the bot's spot now: forced move, break a pinning cast
    bool held = false;          // a valid hold: walk back to hold.spot, never search
    WormHoldView hold;
    bool escapeFirst = false;   // inside a pool: the escape tier goes first
    bool hasStand = false;      // Melee: the dealt stand
    Position stand;
    Position preferNear;
    ObjectGuid partner;         // Cure: the Bile carrier; Run: the Toxin carrier
    Position partnerPos;
    float partnerReach = WORM_CURE_REACH;  // WORM_RUN_REACH on a run to a carrier that can't move
    std::vector<EncounterHelpers::HazardCircle> circles;        // hard: pools, Spray, Bile, Sweep, stomp, own feet
    std::vector<EncounterHelpers::HazardCircle> spreadCircles;  // casters' spread and the melee stack
    // spreadCircles less the pair partner's; empty when no peer can take a partner
    std::vector<EncounterHelpers::HazardCircle> pairCircles;
    bool pairable = false;      // there is a pair partner, even when its circle was the only spread one
    bool spreadStrict = false;  // heroic with Dreadscale stationary: no nospread tier
    std::vector<EncounterHelpers::HazardCircle> urgentCircles;  // pools, Spray, Bile, Sweep while cast
    // urgentCircles, a run's other Bile carriers, a held-out carrier's clean bots
    std::vector<EncounterHelpers::HazardCircle> escapeCircles;
    std::vector<EncounterHelpers::HazardCircle> looseCircles;   // escapeCircles less those clean bots
    std::function<bool(float, float)> accept;        // floor, Spew cones, path, partner reach, melee rear arc
    std::function<bool(float, float)> escapeAccept;  // floor, Spew cones, path
    std::function<bool(float, float)> crossAccept;   // floor, Spew cones
    std::function<bool(float, float)> roleAccept;    // the role's reach, in combat-reach terms
};
// False when the bot has no worm plan at all; reason None when its spot is fine or held. Trigger and
// action both read it. Parks and releases holds.
bool GetWormMovePlan(PlayerbotAI* botAI, WormMovePlan& plan);
// urgentCircles, then escapeAccept while urgent, else accept
bool WormSpotStillSafe(WormMovePlan const& plan, Position const& spot);

// The spot the action just walked the bot to, parked when it already stands there.
void SetWormHold(PlayerbotAI* botAI, Position const& spot, WormMoveReason reason, WormSpotTier tier, bool parked);
bool IsWormSpotHeld(PlayerbotAI* botAI);        // a hold the plan confirmed within WORM_HOLD_FRESH_MS
bool IsWormWalkInFlight(PlayerbotAI* botAI);    // unparked hold, under WORM_WALK_LATCH_MS, moving, booked on it
void BackOffWormSearch(PlayerbotAI* botAI);     // the plan stays quiet on non-urgent reasons for the backoff
// reach melee or reach spell would walk this bot through a pool or stop it in one (or a Bile pulse, with
// neither debuff). With no way round, only a stop inside counts, for WORM_REACH_HOLD_MS at most.
bool WormReachCrossesPool(PlayerbotAI* botAI);

// Toxin no Bile can cure (no worm engaged, or nobody carries Bile) ticks on to 60 s, so each such carrier
// gets one healer bot, nearest first, latched till its Toxin ends.
Player* GetWormToxinHealTarget(PlayerbotAI* botAI);  // the carrier this healer bot keeps up, else nullptr
// This non-tank bot's own such Toxin has it stuck: "divine shield" or "ice block", whose school immunity
// purges it, else nullptr. Before Paralysis, which leaves the bot unable to cast anything.
char const* GetWormToxinSelfPurge(PlayerbotAI* botAI);

// Slime Pools stay read while the Beasts gate is open, so the ones a worm leaves behind still count
// through the gap and Icehowl (60 s on 25H). Each as its radius `ticks` seconds on, plus pad.
std::vector<EncounterHelpers::HazardCircle> GetSlimePoolCircles(PlayerbotAI* botAI, uint32 ticks, float pad);
// The spot moved out to the edge of any pool, 5 s on plus the clearance pad, staying on the spot floor
Position PushOutOfSlimePools(PlayerbotAI* botAI, Position const& spot);
// No worm is engaged, so no reposition runs, and the bot stands in a pool or one reaching it before its
// pinned cast ends
bool InSlimePoolWithoutWorms(PlayerbotAI* botAI);
// Nearest spot clear of every pool 5 s on, in reach of its target where it can be, off a latched
// Icehowl charge lane
bool FindSlimePoolExit(PlayerbotAI* botAI, Position& spot);

}

#endif
