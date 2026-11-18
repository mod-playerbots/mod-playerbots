/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_Anubarak.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <deque>
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>

#include "BossAuraTriggers.h"
#include "Creature.h"
#include "DBCStores.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "PaladinHelper.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCAnubarakKitePlanner.h"
#include "ToCAnubarakPursuit.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Shared.h"
#include "ToCHelpers_Staging.h"
#include "Unit.h"

using namespace EncounterHelpers;

namespace TrialOfTheCrusaderHelpers
{
const Position ANUBARAK_ROOM_CENTER = {745.0f, 135.0f, 142.6f};
}

namespace
{
using namespace TrialOfTheCrusaderHelpers;

// Some bot reads the phase every tick of a pull, so a gap this long means the last pull ended unseen
constexpr uint32 ANUBARAK_PULL_GAP_MS = 5000;
// HealthBelowPct(30) in the script
constexpr float ANUBARAK_SWARM_HEALTH_PCT = 30.0f;
// How far ValidateFloorPoint may pull a stand back before it counts as off the floor
constexpr float ANUBARAK_FLOOR_XY_TOLERANCE = 1.5f;
constexpr float ANUBARAK_FLOOR_Z_TOLERANCE = 3.0f;
// Detour corners sit this far outside the slow reach, so the leg along a square side stays clear
constexpr float ANUBARAK_DETOUR_MARGIN = 0.5f;
constexpr float ANUBARAK_PATH_CLIP_SLACK = 0.1f;
constexpr float ANUBARAK_DEG_TO_RAD = static_cast<float>(M_PI) / 180.0f;

// A leg starting inside an obstacle's clearance passes while it gets no closer than this past its start
constexpr float ANUBARAK_KITE_LANE_START_SLACK = 0.5f;
// Gathered bots stand on 3 yd slots round the gather point and up to 6 yd off them, out of Impale's
// reach with a margin
constexpr float ANUBARAK_KITE_GATHER_CLEARANCE =
    ANUBARAK_GATHER_SPREAD + ANUBARAK_GATHER_ARRIVE + ANUBARAK_IMPALE_REACH + 3.5f;
// The spike's run keeps this far off a player outside the gather: Impale's reach plus 2.5
constexpr float ANUBARAK_KITE_PLAYER_CLEARANCE = 10.0f;
// Permafrost slows 80% inside its reach on heroic, 30% on normal
constexpr float ANUBARAK_PERMAFROST_HEROIC_SLOW = 0.2f;
constexpr float ANUBARAK_PERMAFROST_NORMAL_SLOW = 0.7f;
// No catch read from the plan: the Hand waits as if it came this late
constexpr float ANUBARAK_KITE_CATCH_HORIZON_S = 20.0f;
// The spike tests for a patch, else Impales, every 1.5 s at its base speed
constexpr float ANUBARAK_SPIKE_BASE_TEST_S = 1.5f;
constexpr uint32 ANUBARAK_KITE_PLAN_JUDGE_MS = 250;
// The kiter aims this far past its nearest track point, at most AHEAD_S on, that point looked for SLACK_TICKS
// either side of the plan's age. Yards, not seconds: a time lead on a fast run cuts the ring
constexpr float ANUBARAK_KITE_AHEAD_YD = 8.4f;
constexpr float ANUBARAK_KITE_AHEAD_S = 1.2f;
constexpr int ANUBARAK_KITE_TRACK_SLACK_TICKS = 15;
// A human kiter's run is read off how far it moved between plans this far apart
constexpr uint32 ANUBARAK_KITE_MOTION_MIN_MS = 100;
constexpr uint32 ANUBARAK_KITE_MOTION_MAX_MS = 1000;
// Hand of Protection goes out this long before the catch, an own immunity this long: Feign Death needs
// the hunter stopped first
constexpr float ANUBARAK_KITE_HAND_LEAD_S = 1.0f;
constexpr float ANUBARAK_KITE_OWN_LEAD_S = 0.6f;
// The emerge despawns the spike a minute after he goes down, 2 s before he is selectable again
constexpr uint32 ANUBARAK_SPIKE_LIFE_MS = 60000;
// The submerge whose kites the first two keep a patch for
constexpr uint32 ANUBARAK_LAST_KITE_SUBMERGE = 3;
// The submerge the first keeps a Hand for; a Hand back by this long before its spike despawns is spare
constexpr uint32 ANUBARAK_HAND_KEEP_SUBMERGE = 2;
constexpr uint32 ANUBARAK_HAND_SPARE_TAIL_MS = 10000;
// A druid main tank stays in bear form from this long before the emerge
constexpr int32 ANUBARAK_EMERGE_BEAR_MS = 10000;
// The paladin due to cast the Hand keeps its GCD free from this long before the catch
constexpr float ANUBARAK_KITE_HAND_RESERVE_S = 2.5f;
// The Hand also waits for the spike within Impale's reach plus this much of its closing speed
constexpr float ANUBARAK_KITE_HAND_CLOSE_S = 1.5f;
// A non-paladin's own immunity after a Hand goes out this long before the Hand ends
constexpr float ANUBARAK_KITE_OWN_AFTER_HAND_S = 0.8f;
// No boost while a Hand has more than this left: the spike sits on the kiter either way. That's for Blink
// and the circle, which jump; a speed boost goes earlier (BoostLeadS), never past the max
constexpr float ANUBARAK_KITE_BOOST_HAND_S = 0.5f;
constexpr float ANUBARAK_KITE_BOOST_HAND_MAX_S = 2.0f;
// A speed boost under a Hand puts the kiter this far past Impale's reach as the Hand ends. A hand kite
// also holds its boost for the Hand until the spike is that close
constexpr float ANUBARAK_KITE_BOOST_MARGIN = 1.5f;
constexpr float ANUBARAK_HAND_DURATION_S = 10.0f;
// An own kite this hurt goes immune at once: scarab ticks can kill it before the catch
constexpr float ANUBARAK_KITE_OWN_LOW_HEALTH_PCT = 50.0f;
// After a fail the spike restarts from the spent patch, so the gather steps this far off it, inside the
// floor's walkable 45 yd
constexpr float ANUBARAK_GATHER_STEP = 12.0f;
constexpr float ANUBARAK_GATHER_STEP_REACH = 45.0f;

// The spike's speed auras. The first two last 7 s and the next one goes out as each runs out.
constexpr uint32 SPELL_SPIKE_SPEED1 = 65920;
constexpr uint32 SPELL_SPIKE_SPEED2 = 65922;
constexpr uint32 SPELL_SPIKE_SPEED3 = 65923;

constexpr float ANUBARAK_SCARAB_PICKUP_RANGE = 30.0f;
// A scarab runs 6.3 yd/s to a player's 7 until Determination doubles it. Its victim within REACH steps STEP
// away, out of MELEE_REACH or no closer than SLACK inside the gap it starts at, and wide of the burrowers
constexpr float ANUBARAK_SCARAB_KITE_REACH = 15.0f;
constexpr float ANUBARAK_SCARAB_KITE_STEP = 10.0f;
constexpr float ANUBARAK_SCARAB_MELEE_REACH = 7.7f;
constexpr float ANUBARAK_SCARAB_KITE_SLACK = 0.5f;
constexpr float ANUBARAK_SCARAB_KITE_BURROWER_CLEARANCE = 8.0f;
constexpr int ANUBARAK_SCARAB_KITE_HEADINGS = 16;
constexpr uint32 SPELL_SCARAB_DETERMINATION = 66092;

// Heroic phase 3: the swarm heals him by what it drains, so heals go to the tanks and Penetrating Cold only.
// The rest live on their own, party and raid heals, with a HoT from another healer under ANUBARAK_SWARM_HOT_HP
constexpr float ANUBARAK_SWARM_TANK_URGENT_PCT = 50.0f;
constexpr float ANUBARAK_SWARM_TANK_TOPOFF_PCT = 90.0f;
constexpr float ANUBARAK_SWARM_COLD_TOPOFF_PCT = 50.0f;
// A cold duty healer leaves the tanks to the tank healers down to here, or to the tank line above with
// none of them alive
constexpr float ANUBARAK_SWARM_COLD_HEALER_TANK_PCT = 35.0f;
// The swarm never takes less than this a tick (spell_gen_leeching_swarm_aura), so a carrier left with
// less after a Penetrating Cold tick dies to the next swarm tick anyway
constexpr int32 ANUBARAK_SWARM_MIN_DRAIN = 250;
// Four of those ticks plus some margin, so a HoT's first tick 3 s later still lands
constexpr uint32 ANUBARAK_SWARM_HOT_HP = 1200;
// A direct heal on a carrier at risk has to land this long before its next tick
constexpr int32 ANUBARAK_COLD_RESCUE_MARGIN_MS = 300;
// Aura Mastery goes on a Penetrating Cold round this fresh, so its 6 s cover the round's first two ticks
constexpr int32 ANUBARAK_MASTERY_ROUND_MS = 1500;
constexpr uint32 ANUBARAK_MASTERY_MIN_CARRIERS = 2;
constexpr uint32 SPELL_WEAKENED_SOUL = 6788;
// Heroic burst waits for phase 3 from this health, unless at his pace he'd still be over 30% BEHIND_MARGIN
// before the submerge: a 2 min cooldown spent in the last phase 1 is only back 60-90 s into phase 3
constexpr float ANUBARAK_BURST_HOLD_PCT = 50.0f;
constexpr float ANUBARAK_BURST_BEHIND_MARGIN_S = 5.0f;
// Phase 3 only starts while he's up. From WINDOW before a submerge every DPS bot goes on him once his loss
// since he came up, scaled to all of them on him and by DISCOUNT, gets him under TARGET in time
constexpr uint32 ANUBARAK_PUSH_WINDOW_MS = 30000;
constexpr uint32 ANUBARAK_PUSH_MIN_SPAN_MS = 15000;
constexpr uint32 ANUBARAK_PUSH_SAMPLE_MS = 1000;
constexpr float ANUBARAK_PUSH_TARGET_PCT = 29.5f;
constexpr float ANUBARAK_PUSH_DISCOUNT = 0.75f;
constexpr float ANUBARAK_PUSH_SHARE_FLOOR = 0.25f;
// Heroic's submerge drag waits while his loss since the push latched still gets him under TARGET with
// DRAG_MARGIN to spare. The latch's own estimate stands in for the first LIVE_MS.
constexpr float ANUBARAK_PUSH_DRAG_MARGIN_S = 2.0f;
constexpr uint32 ANUBARAK_PUSH_LIVE_MS = 3000;
// Phase 3's lust and burst wait for the burrowers up at its start to die, at most this long
constexpr uint32 ANUBARAK_SWARM_BURST_WAIT_MS = 15000;
// Heroic phase 3: a wave is left to the side tanks only while he'd die within this at the raid's pace over
// the last 10 s
constexpr float ANUBARAK_SWARM_IGNORE_S = 10.0f;
constexpr uint32 ANUBARAK_SWARM_TTD_WINDOW_MS = 10000;
constexpr uint32 ANUBARAK_SWARM_TTD_MIN_MS = 2000;
constexpr uint32 ANUBARAK_SWARM_HEALTH_SAMPLE_MS = 1000;
constexpr uint32 ANUBARAK_SWARM_WAVE_READ_MS = 2000;
// Berserk comes 10 min into the pull. Heroic phase 3 melee stay on him while at his pace since the swarm he'd
// still be up MARGIN before it, read from TTD_WINDOW in: a whole-phase pace, so a 10 s dip swaps no melee
constexpr uint32 ANUBARAK_BERSERK_MS = 600000;
constexpr float ANUBARAK_SWARM_BERSERK_MARGIN_S = 30.0f;

// The floor is about 60 yd round the room centre. The Web Door is 83 yd out, and anyone who never
// landed is still up near z 395.
constexpr float ANUBARAK_PIT_RADIUS = 60.0f;
constexpr float ANUBARAK_PIT_CEILING_Z = 200.0f;
constexpr float ANUBARAK_SUBMERGE_SPOT_SEARCH = 40.0f;
constexpr float ANUBARAK_SUBMERGE_SPOT_FLOOR = 40.0f;
// Heroic drags him far off the raid point: the spot is searched this long after the pull or an emerge, on
// these rings round the centre (walkable out to 45 yd), 50+ yd off R first, farthest in bands
constexpr uint32 ANUBARAK_SUBMERGE_FAR_SEARCH_MS = 40000;
constexpr float ANUBARAK_SUBMERGE_FAR_RINGS[] = {45.0f, 40.0f, 35.0f};
constexpr uint32 ANUBARAK_SUBMERGE_FAR_HEADINGS = 24;
constexpr float ANUBARAK_SUBMERGE_FAR_MIN = 50.0f;
constexpr float ANUBARAK_SUBMERGE_FAR_BAND = 5.0f;
// The drag leaves with under LEAD_S to go, as a long one pulls healers off the side tanks, so the spot lies
// within its reach. Freezing Slash stuns the tank 3 s every 15-20 s: nearly every drag loses STUN_S to it
constexpr float ANUBARAK_SUBMERGE_DRAG_SPEED = 3.0f;
constexpr float ANUBARAK_SUBMERGE_DRAG_MARGIN_S = 3.0f;
constexpr float ANUBARAK_SUBMERGE_DRAG_STUN_S = 3.0f;
constexpr float ANUBARAK_SUBMERGE_DRAG_LEAD_S = 18.0f;
constexpr float ANUBARAK_SUBMERGE_DRAG_REACH =
    (ANUBARAK_SUBMERGE_DRAG_LEAD_S - ANUBARAK_SUBMERGE_DRAG_MARGIN_S - ANUBARAK_SUBMERGE_DRAG_STUN_S) *
    ANUBARAK_SUBMERGE_DRAG_SPEED;
// Far from every patch first: the spike summoned on him runs a long way before it can reach one
constexpr float ANUBARAK_SUBMERGE_CLEARANCES[] = {30.0f, 25.0f, ANUBARAK_SUBMERGE_PATCH_CLEARANCE,
                                                  ANUBARAK_SUBMERGE_PATCH_CLEARANCE_MIN};
constexpr float ANUBARAK_TANK_STEP_SEARCH = 15.0f;
// Challenging Roar and Challenging Shout reach 10 yd
constexpr float ANUBARAK_SIDE_SHOUT_RADIUS = 10.0f;
// A human side tank takes the other side once he stands this close to its spot, this much nearer it
// than to his own, for the dwell
constexpr float ANUBARAK_HUMAN_SIDE_REACH = 8.0f;
constexpr float ANUBARAK_HUMAN_SIDE_MARGIN = 4.0f;
constexpr uint32 ANUBARAK_HUMAN_SIDE_DWELL_MS = 3000;
// A human side tank's burrower still loose this long after it came out goes to the bot side tank
constexpr uint32 ANUBARAK_HUMAN_PICKUP_GRACE_MS = 5000;

// His chase stops on the 400 ms recheck after the tank does, half of it on average
constexpr float ANUBARAK_DRAG_STOP_LAG_S = 0.2f;
constexpr float ANUBARAK_DRAG_SPOT_ARRIVE = 1.0f;
// isMoving can miss a short step between two reads, so a still boss that drifts this far moved
constexpr float ANUBARAK_STILL_DRIFT = 0.3f;
// The tank stands a few yards off his line to the point, so a patch this close to the line has the drag
// routed round it, through waypoints this far either side of it
constexpr float ANUBARAK_DRAG_DETOUR_TRIGGER = ANUBARAK_TANK_KEEPOUT + 3.0f;
constexpr float ANUBARAK_DRAG_DETOUR_STANDOFF = 12.0f;
// Held further off the anchor than this for this long with no leg, the drag tries a ring round it too
constexpr float ANUBARAK_DRAG_STALL_RANGE = 8.0f;
constexpr uint32 ANUBARAK_DRAG_STALL_MS = 5000;

// A patch landing this close to the spike, or to him before it spawns on him, burns before a kite can use it
constexpr float ANUBARAK_STOCK_SPIKE_CLEARANCE = ANUBARAK_SUBMERGE_PATCH_CLEARANCE_MIN;
// Heroic: six spheres never respawn, the east pair tanks, so each of two submerges gets two of the four left
constexpr uint32 ANUBARAK_KITE_SPHERES_PER_SUBMERGE = 2;
// Both go from this long before he submerges: the two share the shooters, and each patch takes the kill plus
// a 1.5 s fall
constexpr float ANUBARAK_KITE_PRESHOT_LEFT_S = 30.0f;
// Its patch lands inside the floor's walkable 50 yd
constexpr float ANUBARAK_KITE_SPHERE_FLOOR = 50.0f;
// Preferred off every tank patch: the restart reach plus the ~14 yd the spike walks on after a fail. Heroic
// has four kite spheres, so this is only a preference
constexpr float ANUBARAK_KITE_SPHERE_TANK_CLEARANCE = ANUBARAK_SUBMERGE_PATCH_CLEARANCE_MIN + 14.0f;
// The stand sits this far inside the reach, so a drifting sphere doesn't walk the shooter every tick
constexpr float ANUBARAK_SPHERE_STAND_SLACK = 1.0f;

// A claimed protector with no immunity on the kiter by then passes the claim on
constexpr uint32 ANUBARAK_PROTECT_CLAIM_MS = 1500;

// The pull's first 10 s: the first hunter and the first rogue bot send theirs to the main tank so he
// holds him from the start, and the others keep theirs for the first wave
constexpr uint32 ANUBARAK_REDIRECT_PULL_MS = 10000;
constexpr float ANUBARAK_TRICKS_RANGE = 20.0f;
// A side tank walks back to its hold unless the spike could get there within its walk plus this
constexpr float ANUBARAK_SPIKE_WALK_MARGIN_S = 2.0f;
// Burrowers sit about this far off their patch centre toward the hold, and the two sides' are kept this
// far apart: Spider Frenzy hastes those within 12 yd of each other
constexpr float ANUBARAK_SIDE_HOLD_INSET = 4.0f;
constexpr float ANUBARAK_FRENZY_APART = 14.0f;

// The boss script's Frost Sphere spawns, AnubLocs 5 to 10, at floor height
const Position ANUBARAK_SPHERE_HOMES[] = {{786.64f, 108.25f, 142.6f}, {806.84f, 150.59f, 142.6f},
                                          {759.14f, 163.97f, 142.6f}, {744.37f, 119.52f, 142.6f},
                                          {710.02f, 120.82f, 142.6f}, {706.64f, 161.53f, 142.6f}};
constexpr uint8 ANUBARAK_SPHERE_HOME_COUNT = 6;
// The boss script's churning ground, AnubLocs 1 to 4, where burrowers come up
const Position ANUBARAK_NORTH_EXITS[] = {{694.50f, 185.36f, 142.12f}, {740.18f, 193.44f, 142.12f}};
const Position ANUBARAK_SOUTH_EXITS[] = {{694.89f, 102.48f, 142.12f}, {731.99f, 83.38f, 142.12f}};
// Heroic's tank pair, the east homes: the four nearer the centre are the kite spheres
constexpr uint8 ANUBARAK_TANK_HOME_NORTH = 1;
constexpr uint8 ANUBARAK_TANK_HOME_SOUTH = 0;
// Heroic refills: an open spot at most this far from the held patch, an open pair pulled in to this apart
constexpr float ANUBARAK_REFILL_MAX = 30.0f;
// Refill spots are pulled in to this far from the room centre, inside the floor's walkable 45 yd
constexpr float ANUBARAK_REFILL_FLOOR = 45.0f;
// The raid point sits this far off the anchor, at right angles to the line through the tank spots
constexpr float ANUBARAK_RAID_POINT_OFFSET = 18.0f;
constexpr float ANUBARAK_RAID_POINT_SLACK = 0.5f;
// The slow's 7.5 yd plus a yard
constexpr float ANUBARAK_HEALER_PATCH_CLEARANCE = 8.5f;
constexpr uint32 ANUBARAK_HEALER_SLOT_TRIES = 12;
// Tank patches this far apart split the healers in two groups, each this far off its side tank, the
// group kept within the reach of the main tank too
constexpr float ANUBARAK_HEALER_SPLIT_GAP = 40.0f;
constexpr float ANUBARAK_HEALER_SIDE_OFFSET = 17.5f;
constexpr float ANUBARAK_HEALER_MAIN_TANK_REACH = 36.0f;
// The spike is summoned where he goes down, so that is kept this far from the raid point
constexpr float ANUBARAK_SUBMERGE_RAID_CLEARANCE = 25.0f;
// Heroic phase 2: the main tank's gather slot, off the raid point away from where he went down
constexpr float ANUBARAK_GATHER_TANK_EDGE = 6.0f;

std::vector<uint32> const SWEEP_ENTRIES = {
    static_cast<uint32>(ToCNpcs::NPC_FROST_SPHERE),
    static_cast<uint32>(ToCNpcs::NPC_PURSUING_SPIKE),
    static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER),
    static_cast<uint32>(ToCNpcs::NPC_SWARM_SCARAB),
};

struct SphereDuty
{
    ObjectGuid shooter;
    ObjectGuid sphere;
    char const* label = "none";
};

struct StrikeDuty
{
    ObjectGuid caster;
    ObjectGuid interrupter;
};

struct DragLegLatch
{
    bool valid = false;
    ObjectGuid tank;
    Position point;
    Position tankSpot;
    // Walked first when the tank's own walk to its spot would cross a patch's slow
    bool preStep = false;
    Position preStepSpot;
    uint32 plannedMs = 0;
    // He was planned on standing still, so only a stop after moving means he followed
    bool bossMoved = false;
    bool arrived = false;
    uint32 arrivedMs = 0;
};

struct DodgeHold
{
    Position spot;
    // Its gather slot, else where it stood: held until the spike's path keeps off it and the walk back
    Position home;
    ObjectGuid kiter;
    uint32 setMs = 0;
    // Came within ANUBARAK_DODGE_ARRIVE once, so another walk off the spot isn't pulled back to it
    bool arrived = false;
};

struct ThreatCapLatch
{
    bool capped = false;
    bool memoValid = false;
    uint32 memoMs = 0;
    bool value = false;
};

struct ProtectClaim
{
    bool memoValid = false;
    uint32 memoMs = 0;
    ObjectGuid kiter;
    ObjectGuid protector;
    uint32 claimMs = 0;
    // Paladins whose claim lapsed this mark window
    std::vector<ObjectGuid> passed;
    bool need = false;
    bool spikeInImmunityRange = false;
    // The spike catches the kiter within the catch horizon, its speed steps in: no immunity goes on one
    // that doesn't
    bool spikeFaster = false;
    // The kiter gets to its patch's stand before the spike gets to it
    bool standReachable = false;
    // Heroic: the kite plan says the kiter casts its own Hand or immunity now
    bool heroic = false;
    bool selfDue = false;
};

// Heroic: the paladin bot keeping the kiter inside Hand of Protection's reach
struct EscortRead
{
    bool memoValid = false;
    uint32 memoMs = 0;
    ObjectGuid paladin;
    bool spotValid = false;
    Position spot;
};

// The paladin bot casting Hand of Freedom on a kiter marked in Permafrost
struct FreedomRead
{
    bool memoValid = false;
    uint32 memoMs = 0;
    ObjectGuid kiter;
    ObjectGuid paladin;
};

struct GripDuty
{
    ObjectGuid dk;
    ObjectGuid sphere;
    uint8 side = 0;
    // Where the DK grips from: the side's spot, up to ANUBARAK_GRIP_STEP_OUT off it, FarGripStand's, or a
    // relay link's post
    Position stand;
    // A relay link: grips only once the grip before it has landed
    bool relay = false;
};

// A far grip's next link: a DK posted nearer the spot grips the sphere again once the grip before it has
// brought it down, before it rises back out of reach
struct GripRelay
{
    ObjectGuid dk;
    Position post;
    // Where the grip before it brings the sphere down
    Position from;
    // When that grip landed, 0 while it's still out
    uint32 landedMs = 0;
    // No further link for this sphere: its next landing is killed where it is
    bool closed = false;
};

struct GripCast
{
    bool valid = false;
    ObjectGuid dk;
    ObjectGuid sphere;
    uint32 castMs = 0;
    // Where the DK stood: the sphere comes down there
    Position from;
    bool landed = false;
    // The sphere's distance from the DK as the grip went out
    float startDist = 0.0f;
    // A relay link's grip, and the landed grip before it, which stands again if this one pulls nothing
    bool relayLink = false;
    ObjectGuid prevDk;
    Position prevFrom;
};

struct BurrowerExit
{
    uint8 side = 0;
    uint32 seenMs = 0;
    // Its spawn or its last emerge
    uint32 upMs = 0;
    bool down = false;
};

struct SideNeed
{
    bool open = false;
    uint32 sinceMs = 0;
};

// Heroic: the kiter's stage, memoised per instance per ms
struct KiteStageRead
{
    bool valid = false;
    uint32 ms = 0;
    ObjectGuid kiter;
    AnubarakKiteStage stage = AnubarakKiteStage::None;
    // Set only when it goes out now
    AnubarakKiteBoost boost;
};

// The kite planner's pick for the marked kiter, re-planned every 250 ms, its track starting at judgedMs
struct KitePlan
{
    bool valid = false;
    ObjectGuid kiter;
    uint32 judgedMs = 0;
    // When the candidate held now was first picked
    uint32 pickedMs = 0;
    AnubarakKitePlanner::Candidate candidate;
    AnubarakKitePlanner::Outcome outcome;
    // The paladin whose Hand of Protection the plan lands, kept once it has landed
    ObjectGuid hand;
    bool handLanded = false;
    ObjectGuid patchGuid;
    Position patchPos;
    // A human kiter's last spot, for its motion
    Position lastPos;
    uint32 lastPosMs = 0;
};

// Where the escort waits for the catch, kept until the catch point it was built on drifts
struct EscortPostLatch
{
    bool valid = false;
    ObjectGuid kiter;
    Position catchPoint;
    Position post;
};

// Where the escort waits by the patch of a kiter that boosted, kept to the kite's end
struct EscortPatchLatch
{
    bool valid = false;
    ObjectGuid kiter;
    ObjectGuid patch;
    Position post;
};

// Since when tanks within ANUBARAK_FOCUS_SETTLE have held a burrower
struct SettleWatch
{
    uint32 sinceMs = 0;
};

struct MeleeStandLatch
{
    ObjectGuid burrower;
    // Where the burrower stood at the pick
    Position at;
    Position stand;
};

// What the refill spots were built from: the patches held, the spawn slots of the spheres left
struct RefillKey
{
    ObjectGuid held[2];
    uint8 homes = 0;
    bool firstPairDone = false;

    bool operator==(RefillKey const& other) const
    {
        return held[0] == other.held[0] && held[1] == other.held[1] && homes == other.homes &&
               firstPairDone == other.firstPairDone;
    }
};

struct PushSample
{
    uint32 ms = 0;
    float healthPct = 100.0f;
    // Living DPS bots with him selected, of all of them
    float share = 0.0f;
};

// A positive cast or channel in progress and the unit it's aimed at
struct HealOnTheWay
{
    ObjectGuid caster;
    ObjectGuid target;
};

// Living hunter bots in group order, and the first living rogue bot
struct RedirectRoster
{
    std::vector<ObjectGuid> hunters;
    ObjectGuid firstRogue;
};

struct AnubarakState
{
    ObjectGuid patch0{};
    ObjectGuid patch1{};
    // Each side's burrower DPS focus
    ObjectGuid focus0{};
    ObjectGuid focus1{};
    uint32 wave{};
    // Each living DPS bot's side for the burrowers, split at the wave that set it
    std::unordered_map<ObjectGuid, uint32> dpsSide;
    bool dpsSideValid = false;
    uint32 dpsSideWave = 0;

    // 0 is a real getMSTime value, hence the valid flags
    bool engagedSeen = false;
    uint32 lastEngagedMs = 0;
    uint32 pullMs = 0;
    bool swarm = false;
    // Heroic phase 3's burst waits on the burrowers alive when the swarm latched
    uint32 swarmMs = 0;
    float swarmHpPct = 30.0f;
    std::unordered_set<ObjectGuid> swarmBurrowers;
    // Heroic phase 3: his health once a second over ANUBARAK_SWARM_TTD_WINDOW_MS, and whether the swarm's
    // burrowers are left to the side tanks, read per wave and every ANUBARAK_SWARM_WAVE_READ_MS while one lives
    std::deque<std::pair<uint32, uint32>> swarmHealth;
    bool waveLeave = false;
    uint32 waveReadMs = 0;
    size_t waveAlive = 0;
    // The pull or the last emerge: the script submerges him 80 s after either
    uint32 surfacedMs = 0;
    float surfacedHpPct = 100.0f;
    // Once a second while he's up, until the push latches. Cleared by the submerge and the swarm.
    std::deque<PushSample> pushSamples;
    bool push = false;
    // Read only while push holds: the latch time, his health then and the latch's loss a second
    uint32 pushMs = 0;
    float pushHpPct = 100.0f;
    float pushPerS = 0.0f;
    uint32 submergedMs = 0;
    // Submerges this pull: heroic's first two keep a kite patch and spare Hands for the third
    uint32 submerges = 0;
    bool submergeSpotValid = false;
    Position submergeSpot;
    // Heroic: the drag to the submerge spot has left the anchor this window
    bool submergeDragStarted = false;
    ObjectGuid mainTank;
    ObjectGuid sideTanks[2];
    // A human side tank has stood at the other side's spot since then
    bool humanSwapValid = false;
    uint32 humanSwapMs = 0;
    // Side tanks that died this pull, for the battle res
    std::vector<ObjectGuid> deadSideTanks;
    // Last marked player seen, so the kite's latches reset when the mark moves on
    ObjectGuid kiter;
    // Heroic: the kiter that turned for its kite patch, until the mark moves on
    ObjectGuid kitePatchKiter;

    // Every burrower seen this pull: the side of the exit it came out of, and when
    std::unordered_map<ObjectGuid, BurrowerExit> exits;
    bool waveSeen = false;
    uint32 lastWaveMs = 0;
    bool ownerMemoValid = false;
    uint32 ownerMemoMs = 0;
    std::unordered_map<ObjectGuid, uint8> owners;

    // A melee bot's stand on its side's focus, held while that burrower lives and stays put
    std::unordered_map<ObjectGuid, MeleeStandLatch> meleeStands;
    // Each burrower held by a tank right by it
    std::unordered_map<ObjectGuid, SettleWatch> settles;

    // Heroic: spheres flying as a submerge's kite budget opened, 20 s before it, and the kite patch already
    // down then, so the submerge downs at most its two kite spheres counting that one
    bool kiteBudgetSet = false;
    uint32 kiteBudgetFlying = 0;
    uint32 kiteLeftover = 0;
    // Heroic: the landed patches that aren't tank patches, and how many the spike took this submerge
    std::unordered_set<ObjectGuid> kitePatchesSeen;
    // CountSpentKitePatches' scratch, reused for its buckets and read nowhere else
    std::unordered_set<ObjectGuid> kitePatchesLanded;
    uint32 kitePatchesSpent = 0;
    // Each side's tank patch where it was last seen, for the gather to step off once it's spent
    bool tankPatchSeen[2] = {false, false};
    Position tankPatchPos[2];
    KiteStageRead kiteStage;
    KitePlan kitePlan;
    EscortPostLatch escortPost;
    EscortPatchLatch escortPatch;
    // The paladin bot shadowing standbyKiter while its plan lands no other paladin's Hand
    ObjectGuid standby;
    ObjectGuid standbyKiter;
    // Where he went down, for the heroic phase 2 gather
    bool submergePosValid = false;
    Position submergePos;

    SideNeed needs[2];
    GripCast grips[2];
    GripRelay relays[2];
    std::vector<GripDuty> gripDuties;
    // A sphere that landed on a DK, and the side it was gripped for. A gripped sphere drifts back toward
    // its route while it dies, so its patch can come down nearer the other side's spot.
    std::unordered_map<ObjectGuid, uint8> grippedFor;
    // Gripped from past the step-out, so its patch serves its side however far from the spot it lands
    std::unordered_set<ObjectGuid> farGripped;

    // Each sphere's slot in ANUBARAK_SPHERE_HOMES, read at its first sample. Heroic never respawns one.
    std::unordered_map<ObjectGuid, uint8> sphereHomes;
    // Both sides have held a patch this pull
    bool firstPairDone = false;
    // Heroic after the first pair: an open side's spot by the spheres left
    bool refillKeyValid = false;
    RefillKey refillKey;
    bool refillValid[2] = {false, false};
    Position refill[2];
    Position spot0{};
    Position spot1{};

    // The healer stand while he is up, and heroic's phase 2 gather. Held for a submerge, so a tank patch
    // spent while he is down doesn't move the gather under the raid.
    bool raidPointValid = false;
    Position raidPointAnchor;
    Position raidPoint;
    bool raidPointHeld = false;
    uint32 raidPointHeldMs = 0;

    bool phaseMemoValid = false;
    uint32 phaseMemoMs = 0;
    AnubarakPhase phaseValue = AnubarakPhase::None;

    bool sweepMemoValid = false;
    uint32 sweepMemoMs = 0;
    std::vector<ObjectGuid> spheres;
    std::vector<ObjectGuid> burrowers;
    std::vector<ObjectGuid> scarabs;
    ObjectGuid spikeGuid;

    bool dutyMemoValid = false;
    uint32 dutyMemoMs = 0;
    ObjectGuid dutyGroup;
    std::vector<SphereDuty> duties;

    bool strikeMemoValid = false;
    uint32 strikeMemoMs = 0;
    ObjectGuid strikeGroup;
    std::vector<StrikeDuty> strikes;

    bool bossStillValid = false;
    uint32 bossStillMs = 0;
    Position bossStillPos;
    DragLegLatch drag;
    // Last time he stood near the anchor or a leg was latched, for the drag's stall guard
    bool dragFreeValid = false;
    uint32 dragFreeMs = 0;
    float submergeSpotClearance = 0.0f;

    std::unordered_map<ObjectGuid, DodgeHold> dodgeHolds;
    bool dodgeTrackMemoValid = false;
    uint32 dodgeTrackMemoMs = 0;
    std::vector<AnubarakPursuit::Point> dodgeTrack;
    std::unordered_map<ObjectGuid, ThreatCapLatch> threatCaps;
    // Per DPS bot and burrower: capped on its side tank's threat
    std::map<std::pair<ObjectGuid, ObjectGuid>, bool> burrowerCaps;
    // Per melee DPS bot and burrower: over ANUBARAK_MELEE_AREA_CAP of its side tank's threat
    std::map<std::pair<ObjectGuid, ObjectGuid>, bool> burrowerAreaCaps;
    std::unordered_map<ObjectGuid, ThreatCapLatch> sideBossCaps;
    // Heroic phase 3: the healers' and ranged DPS bots' stands behind the tank patches, and which healers
    // have set off for theirs
    bool behindSet = false;
    uint32 behindTriedMs = 0;
    std::map<ObjectGuid, Position> behindStands;
    std::unordered_set<ObjectGuid> behindGone;
    // Heroic phase 3: each healer bot's duty, and the Penetrating Cold carrier at risk each one keeps up
    std::unordered_map<ObjectGuid, AnubarakHealDuty> healDuty;
    std::unordered_map<ObjectGuid, ObjectGuid> coldOf;
    // Most urgent first
    std::vector<ObjectGuid> coldAtRisk;
    bool healDutySet = false;
    bool coldMemoValid = false;
    uint32 coldMemoMs = 0;
    uint32 coldHealersAlive = 0;
    uint32 tankHealersAlive = 0;
    ProtectClaim protect;
    EscortRead escort;
    FreedomRead freedom;
    // Phase 3: the paladin bot keeping Judgement of Light on him, picked again each time it comes due
    ObjectGuid lightJudge;
    bool lightDue = false;
    bool lightMemoValid = false;
    uint32 lightMemoMs = 0;

    // Group walks behind triggers every bot, healer or hunter asks each tick
    ToCGroupMemo<ObjectGuid> marked;
    ToCGroupMemo<ObjectGuid> resDruid;
    ToCGroupMemo<ObjectGuid> resHealer;
    ToCGroupMemo<std::vector<ObjectGuid>> coldCarriers;
    ToCGroupMemo<std::vector<ObjectGuid>> tankPlayers;
    ToCGroupMemo<std::vector<HealOnTheWay>> healsOnTheWay;
    ToCGroupMemo<std::vector<ObjectGuid>> swarmHealable;
    ToCGroupMemo<std::vector<ObjectGuid>> healerBots;
    ToCGroupMemo<RedirectRoster> redirect;
};

RaidInstanceState<AnubarakState> anubarakStates;

struct Vec2
{
    float x;
    float y;
};

Vec2 Delta(Position const& from, Position const& to)
{
    return {to.GetPositionX() - from.GetPositionX(), to.GetPositionY() - from.GetPositionY()};
}

float Length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

Vec2 Rotate(Vec2 v, float angle)
{
    float const c = std::cos(angle);
    float const s = std::sin(angle);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

Position Offset(Position const& from, Vec2 dir, float distance)
{
    return Position(from.GetPositionX() + dir.x * distance, from.GetPositionY() + dir.y * distance,
                    from.GetPositionZ());
}

float Dist2d(Position const& a, Position const& b) { return a.GetExactDist2d(b.GetPositionX(), b.GetPositionY()); }

float SegmentDistance2d(Position const& a, Position const& b, Position const& point)
{
    Vec2 const ab = Delta(a, b);
    Vec2 const ap = Delta(a, point);
    float const lengthSq = ab.x * ab.x + ab.y * ab.y;
    float t = lengthSq > 0.0f ? (ap.x * ab.x + ap.y * ab.y) / lengthSq : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);
    return point.GetExactDist2d(a.GetPositionX() + ab.x * t, a.GetPositionY() + ab.y * t);
}

bool InToCInstance(Player* bot)
{
    return bot && bot->IsInWorld() && bot->GetMapId() == TRIAL_OF_THE_CRUSADER_MAP_ID && bot->GetInstanceId();
}

bool AnubarakHeroic(Player* bot)
{
    Map* map = bot->GetMap();
    return map && map->IsHeroic();
}

Creature* ResolveCreature(Player* bot, ObjectGuid guid)
{
    if (guid.IsEmpty())
        return nullptr;

    Map* map = bot->GetMap();
    return map ? map->GetCreature(guid) : nullptr;
}

// fn returns true to stop. No group: the bot alone.
template <typename Fn>
void ForEachGroupMember(Player* bot, Fn&& fn)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        fn(bot);
        return;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsInMap(bot) && fn(member))
            return;
    }
}

// A memo's players in its order, any gone from the map skipped. fn returns true to stop.
template <typename Fn>
void ForEachListed(Player* bot, std::vector<ObjectGuid> const& guids, Fn&& fn)
{
    for (ObjectGuid const& guid : guids)
        if (Player* member = ObjectAccessor::GetPlayer(*bot, guid))
            if (fn(member))
                return;
}

// Null off a ToC instance
AnubarakState* AnubarakStateOf(Player* bot)
{
    return InToCInstance(bot) ? &anubarakStates.For(bot->GetInstanceId()) : nullptr;
}

// One grid visit per instance per ms. From the boss when he resolves, so a bot that never landed in
// the pit can't blank it for everyone.
AnubarakState* Sweep(Player* bot)
{
    if (!InToCInstance(bot))
        return nullptr;

    AnubarakState& state = anubarakStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.sweepMemoValid && state.sweepMemoMs == now)
        return &state;

    state.sweepMemoValid = true;
    state.sweepMemoMs = now;
    state.spheres.clear();
    state.burrowers.clear();
    state.scarabs.clear();
    state.spikeGuid.Clear();

    WorldObject* origin = GetAnubarak(bot);
    if (!origin)
        origin = bot;

    static ToCEntrySet const sweepSet(SWEEP_ENTRIES);
    std::vector<Creature*> creatures;
    GetCreatureListWithEntriesInGrid(origin, creatures, sweepSet, ANUBARAK_ROOM_RADIUS);
    for (Creature* creature : creatures)
    {
        if (!creature->IsAlive())
            continue;

        switch (static_cast<ToCNpcs>(creature->GetEntry()))
        {
            case ToCNpcs::NPC_FROST_SPHERE:
                state.spheres.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_NERUBIAN_BURROWER:
                state.burrowers.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_SWARM_SCARAB:
                state.scarabs.push_back(creature->GetGUID());
                break;
            case ToCNpcs::NPC_PURSUING_SPIKE:
                state.spikeGuid = creature->GetGUID();
                break;
            default:
                break;
        }
    }

    return &state;
}

// A few of each kind live at once, so the usual list never touches the heap
using AliveCreatures = boost::container::small_vector<Creature*, 16>;

// A copy, never a view of the guid list: a sweep nested in the caller's loop clears that list
template <class List = AliveCreatures>
List ResolveAlive(Player* bot, std::vector<ObjectGuid> const& guids)
{
    List creatures;
    creatures.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
        if (Creature* creature = ResolveCreature(bot, guid))
            if (creature->IsAlive())
                creatures.push_back(creature);

    return creatures;
}

std::vector<Creature*> ResolveSpheres(Player* bot, AnubarakState const& state)
{
    return ResolveAlive<std::vector<Creature*>>(bot, state.spheres);
}

Creature* FindSphere(std::vector<Creature*> const& spheres, ObjectGuid guid)
{
    if (guid.IsEmpty())
        return nullptr;

    for (Creature* sphere : spheres)
        if (sphere->GetGUID() == guid)
            return sphere;

    return nullptr;
}

bool FallingNear(std::vector<Creature*> const& spheres, Position const& point)
{
    for (Creature* sphere : spheres)
        if (IsFrostSphereFalling(sphere) &&
            sphere->GetExactDist2d(point.GetPositionX(), point.GetPositionY()) <= ANUBARAK_TANK_PATCH_LATCH)
            return true;

    return false;
}

bool AnyBurrower(Player* bot, AnubarakState const& state) { return !ResolveAlive(bot, state.burrowers).empty(); }

// A shooter out here can neither reach nor see a sphere, and its need would never pass on
bool InAnubarakPit(Player* player)
{
    return player->GetPositionZ() < ANUBARAK_PIT_CEILING_Z &&
           Dist2d(player->GetPosition(), ANUBARAK_ROOM_CENTER) <= ANUBARAK_PIT_RADIUS;
}

void ClearPullLatches(AnubarakState& state)
{
    state.swarm = false;
    state.swarmBurrowers.clear();
    state.swarmHealth.clear();
    state.pushSamples.clear();
    state.push = false;
    state.waveLeave = false;
    state.waveReadMs = 0;
    state.waveAlive = 0;
    state.phaseValue = AnubarakPhase::None;
    state.phaseMemoValid = false;
    state.dutyMemoValid = false;
    state.strikeMemoValid = false;
    state.submergeSpotValid = false;
    state.submergeDragStarted = false;
    state.sideTanks[0].Clear();
    state.sideTanks[1].Clear();
    state.humanSwapValid = false;
    state.deadSideTanks.clear();
    state.patch0 = ObjectGuid::Empty;
    state.patch1 = ObjectGuid::Empty;
    state.focus0 = ObjectGuid::Empty;
    state.focus1 = ObjectGuid::Empty;
    state.wave = 0;
    state.dpsSide.clear();
    state.dpsSideValid = false;
    state.exits.clear();
    state.waveSeen = false;
    state.ownerMemoValid = false;
    state.owners.clear();
    state.meleeStands.clear();
    state.settles.clear();
    state.kiteBudgetSet = false;
    state.kiteBudgetFlying = 0;
    state.kiteLeftover = 0;
    state.kitePatchesSeen.clear();
    state.kitePatchesSpent = 0;
    state.tankPatchSeen[0] = false;
    state.tankPatchSeen[1] = false;
    state.kitePatchKiter.Clear();
    state.kiteStage = KiteStageRead();
    state.kitePlan = KitePlan();
    state.escortPost = EscortPostLatch();
    state.escortPatch = EscortPatchLatch();
    state.standby.Clear();
    state.standbyKiter.Clear();
    state.submerges = 0;
    state.submergePosValid = false;
    state.needs[0] = SideNeed();
    state.needs[1] = SideNeed();
    for (GripCast& cast : state.grips)
        cast = GripCast();
    state.gripDuties.clear();
    state.grippedFor.clear();
    state.farGripped.clear();
    state.sphereHomes.clear();
    state.firstPairDone = false;
    state.refillKeyValid = false;
    state.refillValid[0] = false;
    state.refillValid[1] = false;
    state.raidPointValid = false;
    state.raidPointHeld = false;
    state.duties.clear();
    state.bossStillValid = false;
    state.drag = DragLegLatch();
    state.dragFreeValid = false;
    state.dodgeHolds.clear();
    state.dodgeTrackMemoValid = false;
    state.threatCaps.clear();
    state.burrowerCaps.clear();
    state.burrowerAreaCaps.clear();
    state.sideBossCaps.clear();
    state.behindSet = false;
    state.behindStands.clear();
    state.behindGone.clear();
    state.healDuty.clear();
    state.coldOf.clear();
    state.coldAtRisk.clear();
    state.healDutySet = false;
    state.coldMemoValid = false;
    state.protect = ProtectClaim();
    state.escort = EscortRead();
    state.freedom = FreedomRead();
    state.lightJudge.Clear();
    state.lightDue = false;
    state.lightMemoValid = false;
}

void TrackBossStillness(AnubarakState& state, Creature* boss, uint32 now)
{
    bool const drifted =
        state.bossStillValid && Dist2d(boss->GetPosition(), state.bossStillPos) > ANUBARAK_STILL_DRIFT;
    if (boss->isMoving() || drifted)
    {
        state.bossStillValid = false;
        if (state.drag.valid)
            state.drag.bossMoved = true;

        return;
    }

    if (!state.bossStillValid)
    {
        state.bossStillValid = true;
        state.bossStillMs = now;
        state.bossStillPos = boss->GetPosition();
    }
}

bool BossStillFor(AnubarakState const& state, uint32 now, uint32 ms)
{
    return state.bossStillValid && getMSTimeDiff(state.bossStillMs, now) >= ms;
}

// The flagged main tank, else the first living tank with the spec asked too, so it can't blink. walked: no
// flag, so every living member ahead of the answer (all of them when it's empty) is known not to be a tank
ObjectGuid StableMainTankGuid(Group* group, bool& walked)
{
    walked = false;
    for (Group::MemberSlot const& slot : group->GetMemberSlots())
        if (slot.flags & MEMBER_FLAG_MAINTANK)
            return slot.guid;

    walked = true;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && IsAnubarakTankPlayer(member))
            return member->GetGUID();
    }

    return ObjectGuid::Empty;
}

// Living tanks other than the main tank, bots first so a human only takes a side no bot can, assistants
// first within each. walked as StableMainTankGuid set it skips the tank test for members it turned down
std::vector<ObjectGuid> RankSideTanks(Player* bot, Group* group, ObjectGuid const& mainTank, bool walked)
{
    // Bot assistants, other bots, human assistants, other humans
    std::vector<ObjectGuid> ranks[4];
    bool turnedDown = walked;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetGUID() == mainTank)
            turnedDown = false;

        if (!member || !member->IsAlive() || !member->IsInMap(bot) || member->GetGUID() == mainTank ||
            turnedDown || !IsAnubarakTankPlayer(member))
            continue;

        uint8 const rank = (GET_PLAYERBOT_AI(member) ? 0 : 2) + (group->IsAssistant(member->GetGUID()) ? 0 : 1);
        ranks[rank].push_back(member->GetGUID());
    }

    std::vector<ObjectGuid> roster;
    for (std::vector<ObjectGuid> const& rank : ranks)
        roster.insert(roster.end(), rank.begin(), rank.end());

    return roster;
}

// Each side keeps its tank for the pull while it lives. Re-reading the assist tank index every tick
// swaps sides whenever IsTank blinks. placed says which sides took a new tank.
void RefreshSideTanks(Player* bot, AnubarakState& state, bool placed[2])
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        state.mainTank.Clear();
        state.sideTanks[0].Clear();
        state.sideTanks[1].Clear();
        return;
    }

    bool walked = false;
    state.mainTank = StableMainTankGuid(group, walked);
    std::vector<ObjectGuid> const roster = RankSideTanks(bot, group, state.mainTank, walked);

    for (ObjectGuid& held : state.sideTanks)
    {
        if (held.IsEmpty() || std::find(roster.begin(), roster.end(), held) != roster.end())
            continue;

        Player* gone = ObjectAccessor::GetPlayer(*bot, held);
        if (gone && !gone->IsAlive() &&
            std::find(state.deadSideTanks.begin(), state.deadSideTanks.end(), held) == state.deadSideTanks.end())
            state.deadSideTanks.push_back(held);

        held.Clear();
    }

    state.deadSideTanks.erase(std::remove_if(state.deadSideTanks.begin(), state.deadSideTanks.end(),
                                             [bot](ObjectGuid const& guid)
                                             {
                                                 Player* tank = ObjectAccessor::GetPlayer(*bot, guid);
                                                 return !tank || tank->IsAlive();
                                             }),
                              state.deadSideTanks.end());

    for (uint8 side = 0; side < 2; ++side)
    {
        if (!state.sideTanks[side].IsEmpty())
            continue;

        for (ObjectGuid const& guid : roster)
        {
            if (guid != state.sideTanks[1 - side])
            {
                state.sideTanks[side] = guid;
                placed[side] = true;
                break;
            }
        }
    }
}

void TrackKiter(AnubarakState& state, Player* target)
{
    ObjectGuid const guid = target ? target->GetGUID() : ObjectGuid::Empty;
    if (guid == state.kiter)
        return;

    state.kiter = guid;
    state.kitePatchKiter.Clear();
    state.kitePlan = KitePlan();
    state.escortPost = EscortPostLatch();
    state.escortPatch = EscortPatchLatch();

    // A bot waits out a cast in progress before any action runs, its kite included
    PlayerbotAI* kiterAI = target ? GET_PLAYERBOT_AI(target) : nullptr;
    if (kiterAI &&
        (target->GetCurrentSpell(CURRENT_GENERIC_SPELL) || target->GetCurrentSpell(CURRENT_CHANNELED_SPELL)))
        kiterAI->RequestSpellInterrupt();
}

// The boss only ever casts Leeching Swarm below 30% while surfaced, so any of these means phase 3
bool SwarmStarted(Player* bot, Creature* boss)
{
    if (boss->GetHealthPct() < ANUBARAK_SWARM_HEALTH_PCT)
        return true;

    uint32 const swarm = sSpellMgr->GetSpellIdForDifficulty(SPELL_LEECHING_SWARM, boss);
    if (boss->FindCurrentSpellBySpellId(swarm))
        return true;

    bool carried = false;
    ForEachGroupMember(bot, [&](Player* member)
    {
        carried = member->HasAura(swarm);
        return carried;
    });

    return carried;
}

bool OnFloor(Player* bot, Position const& raw, Position& valid)
{
    valid = ValidateFloorPoint(bot, raw);
    return Dist2d(valid, raw) <= ANUBARAK_FLOOR_XY_TOLERANCE &&
           std::fabs(valid.GetPositionZ() - raw.GetPositionZ()) <= ANUBARAK_FLOOR_Z_TOLERANCE;
}

void RecordSphereHomes(AnubarakState& state, std::vector<Creature*> const& spheres)
{
    for (Creature* sphere : spheres)
    {
        if (!IsFrostSphereFlying(sphere) || state.sphereHomes.count(sphere->GetGUID()))
            continue;

        uint8 nearest = 0;
        for (uint8 slot = 1; slot < ANUBARAK_SPHERE_HOME_COUNT; ++slot)
            if (Dist2d(sphere->GetPosition(), ANUBARAK_SPHERE_HOMES[slot]) <
                Dist2d(sphere->GetPosition(), ANUBARAK_SPHERE_HOMES[nearest]))
                nearest = slot;

        state.sphereHomes[sphere->GetGUID()] = nearest;
    }
}

// Pulled in toward the room centre to the refill floor. False off the navmesh.
bool RefillOnFloor(Player* bot, Position raw, Position& out)
{
    Vec2 const fromCenter = Delta(ANUBARAK_ROOM_CENTER, raw);
    float const radius = Length(fromCenter);
    if (radius > ANUBARAK_REFILL_FLOOR)
        raw = Offset(ANUBARAK_ROOM_CENTER, {fromCenter.x / radius, fromCenter.y / radius}, ANUBARAK_REFILL_FLOOR);

    raw.m_positionZ = ANUBARAK_ROOM_CENTER.GetPositionZ();
    return OnFloor(bot, raw, out);
}

std::array<AnubarakPursuit::Point, 2> PairSpotsOf(Position const& north, Position const& south)
{
    return AnubarakPursuit::PairSpots({north.GetPositionX(), north.GetPositionY()},
                                      {south.GetPositionX(), south.GetPositionY()}, ANUBARAK_REFILL_MAX,
                                      {ANUBARAK_ROOM_CENTER.GetPositionX(), ANUBARAK_ROOM_CENTER.GetPositionY()},
                                      ANUBARAK_REFILL_FLOOR);
}

// The east pair's spots, (789.6, 141.1) and (786.4, 117.5), both walkable as refill spots
std::array<AnubarakPursuit::Point, 2> HeroicPairSpots()
{
    return PairSpotsOf(ANUBARAK_SPHERE_HOMES[ANUBARAK_TANK_HOME_NORTH], ANUBARAK_SPHERE_HOMES[ANUBARAK_TANK_HOME_SOUTH]);
}

// The two homes pulled in until 30 apart and to the refill floor, both floor-checked
bool PairOnFloor(Player* bot, Position const& north, Position const& south, Position (&spots)[2])
{
    std::array<AnubarakPursuit::Point, 2> const pair = PairSpotsOf(north, south);
    float const z = ANUBARAK_ROOM_CENTER.GetPositionZ();
    return RefillOnFloor(bot, Position(pair[0].x, pair[0].y, z), spots[0]) &&
           RefillOnFloor(bot, Position(pair[1].x, pair[1].y, z), spots[1]) &&
           Dist2d(spots[0], spots[1]) >= ANUBARAK_TANK_PATCH_GAP;
}

// Heroic refill spots, by the spawns the spheres still fly round. Run after the latches: a falling sphere
// keeps its spawn counted, so the spot it falls for holds until its patch has latched
void UpdateRefillSpots(Player* bot, AnubarakState& state, std::vector<Creature*> const& spheres)
{
    RefillKey key;
    key.firstPairDone = state.firstPairDone;
    key.held[0] = state.patch0;
    key.held[1] = state.patch1;
    for (ObjectGuid& guid : key.held)
        if (!IsPermafrostPatch(FindSphere(spheres, guid)))
            guid.Clear();

    std::vector<uint8> homes;
    for (Creature* sphere : spheres)
    {
        auto const home = state.sphereHomes.find(sphere->GetGUID());
        if (home == state.sphereHomes.end() || !(IsFrostSphereFlying(sphere) || IsFrostSphereFalling(sphere)) ||
            (key.homes & (1u << home->second)))
            continue;

        key.homes = static_cast<uint8>(key.homes | (1u << home->second));
        homes.push_back(home->second);
    }

    if (state.refillKeyValid && state.refillKey == key)
        return;

    state.refillKeyValid = true;
    state.refillKey = key;
    state.refillValid[0] = false;
    state.refillValid[1] = false;
    if (!AnubarakHeroic(bot))
        return;

    if (!key.firstPairDone)
    {
        Position spots[2];
        if (PairOnFloor(bot, ANUBARAK_SPHERE_HOMES[ANUBARAK_TANK_HOME_NORTH],
                        ANUBARAK_SPHERE_HOMES[ANUBARAK_TANK_HOME_SOUTH], spots))
            for (uint8 side = 0; side < 2; ++side)
            {
                state.refill[side] = spots[side];
                state.refillValid[side] = true;
            }

        return;
    }

    bool const open[2] = {key.held[0].IsEmpty(), key.held[1].IsEmpty()};
    if (open[0] && open[1])
    {
        int8 north = -1;
        int8 south = -1;
        float apart = 0.0f;
        for (uint8 a : homes)
            for (uint8 b : homes)
            {
                Position const& n = ANUBARAK_SPHERE_HOMES[a];
                Position const& s = ANUBARAK_SPHERE_HOMES[b];
                float const dist = Dist2d(n, s);
                if (n.GetPositionY() <= s.GetPositionY() || dist < ANUBARAK_TANK_PATCH_GAP ||
                    (north >= 0 && dist >= apart))
                    continue;

                north = static_cast<int8>(a);
                south = static_cast<int8>(b);
                apart = dist;
            }

        if (north < 0)
            return;

        Position spots[2];
        if (PairOnFloor(bot, ANUBARAK_SPHERE_HOMES[north], ANUBARAK_SPHERE_HOMES[south], spots))
            for (uint8 side = 0; side < 2; ++side)
            {
                state.refill[side] = spots[side];
                state.refillValid[side] = true;
            }

        return;
    }

    if (open[0] == open[1])
        return;

    uint8 const side = open[0] ? 0 : 1;
    Position const kept = FindSphere(spheres, key.held[1 - side])->GetPosition();
    int8 best = -1;
    float bestDist = 0.0f;
    for (uint8 slot : homes)
    {
        Position const& home = ANUBARAK_SPHERE_HOMES[slot];
        bool const past = side == 0 ? home.GetPositionY() > kept.GetPositionY()
                                    : home.GetPositionY() < kept.GetPositionY();
        float const dist = Dist2d(home, kept);
        if (!past || dist < ANUBARAK_TANK_PATCH_GAP || (best >= 0 && dist >= bestDist))
            continue;

        best = static_cast<int8>(slot);
        bestDist = dist;
    }

    if (best < 0)
        return;

    Vec2 const toward = Delta(kept, ANUBARAK_SPHERE_HOMES[best]);
    Position const raw = Offset(kept, {toward.x / bestDist, toward.y / bestDist},
                                std::min(bestDist, ANUBARAK_REFILL_MAX));
    Position spot;
    if (RefillOnFloor(bot, raw, spot) && Dist2d(spot, kept) >= ANUBARAK_TANK_PATCH_GAP)
    {
        state.refill[side] = spot;
        state.refillValid[side] = true;
    }
}

// The side's grip landed and not yet killed, or its gripped sphere falling
Creature* GripPending(AnubarakState const& state, std::vector<Creature*> const& spheres, uint8 side)
{
    GripCast const& cast = state.grips[side];
    if (cast.valid && cast.landed)
        if (Creature* sphere = FindSphere(spheres, cast.sphere); IsFrostSphereFlying(sphere))
            return sphere;

    for (auto const& [guid, gripped] : state.grippedFor)
        if (gripped == side)
            if (Creature* sphere = FindSphere(spheres, guid); IsFrostSphereFalling(sphere))
                return sphere;

    return nullptr;
}

// The side's patch, else heroic's refill spot, else its side point moved with the other side's patch or
// pending grip, so a patch shot off its spot drags the other spot along and the pair keeps its spacing
Position SideSpot(AnubarakState const& state, std::vector<Creature*> const& spheres, ObjectGuid const held[2],
                  uint8 side)
{
    Creature* own = FindSphere(spheres, held[side]);
    if (IsPermafrostPatch(own))
        return own->GetPosition();

    if (state.refillValid[side])
        return state.refill[side];

    Position const point = GetAnubarakTankSidePoint(side);
    Creature* other = FindSphere(spheres, held[1 - side]);
    if (!IsPermafrostPatch(other))
        other = GripPending(state, spheres, side) ? nullptr : GripPending(state, spheres, 1 - side);

    if (!other)
        return point;

    Position const otherPoint = GetAnubarakTankSidePoint(1 - side);
    return Position(other->GetPositionX() + point.GetPositionX() - otherPoint.GetPositionX(),
                    other->GetPositionY() + point.GetPositionY() - otherPoint.GetPositionY(), point.GetPositionZ());
}

// At right angles to the line through the tank spots, on the room centre's side of the anchor, west on a
// tie. Follows the anchor while he is up, held from the first read of a submerge to the emerge.
void UpdateRaidPoint(Player* bot, AnubarakState& state, Position const& spot0, Position const& spot1)
{
    bool const down = state.phaseValue == AnubarakPhase::Submerged;
    if (down && state.raidPointValid && state.raidPointHeld && state.raidPointHeldMs == state.submergedMs)
        return;

    // The gather may have stepped off a spent patch while he was down
    bool const released = !down && state.raidPointHeld;
    state.raidPointHeld = down;
    state.raidPointHeldMs = state.submergedMs;
    Position const anchor((spot0.GetPositionX() + spot1.GetPositionX()) / 2.0f,
                          (spot0.GetPositionY() + spot1.GetPositionY()) / 2.0f,
                          (spot0.GetPositionZ() + spot1.GetPositionZ()) / 2.0f);
    if (!released && state.raidPointValid && Dist2d(anchor, state.raidPointAnchor) < ANUBARAK_RAID_POINT_SLACK)
        return;

    Vec2 const along = Delta(spot0, spot1);
    float const length = Length(along);
    Vec2 normal = length > 1.0f ? Vec2{-along.y / length, along.x / length} : Vec2{-1.0f, 0.0f};
    Vec2 const toCenter = Delta(anchor, ANUBARAK_ROOM_CENTER);
    float const lean = normal.x * toCenter.x + normal.y * toCenter.y;
    if (lean <= -1.0f || (lean < 1.0f && normal.x > 0.0f))
        normal = {-normal.x, -normal.y};

    state.raidPointValid = true;
    state.raidPointAnchor = anchor;
    state.raidPoint = ValidateFloorPoint(bot, Offset(anchor, normal, ANUBARAK_RAID_POINT_OFFSET));
}

// Heroic: a tank patch spent while he is down moves the held gather straight away from it
void StepGatherOff(Player* bot, AnubarakState& state, Position const& patch)
{
    if (!state.raidPointValid || !state.raidPointHeld)
        return;

    Vec2 away = Delta(patch, state.raidPoint);
    float length = Length(away);
    if (length < 0.5f)
    {
        away = Delta(patch, ANUBARAK_ROOM_CENTER);
        length = Length(away);
    }

    if (length < 0.5f)
        return;

    Position const raw = Offset(state.raidPoint, {away.x / length, away.y / length}, ANUBARAK_GATHER_STEP);
    Position valid;
    if (Dist2d(raw, ANUBARAK_ROOM_CENTER) <= ANUBARAK_GATHER_STEP_REACH && OnFloor(bot, raw, valid))
        state.raidPoint = valid;
}

// Within reach of the spot and clearly nearer it than the other one
bool StandsAtSpot(Player* player, Position const& spot, Position const& otherSpot)
{
    float const dist = Dist2d(player->GetPosition(), spot);
    return dist <= ANUBARAK_HUMAN_SIDE_REACH &&
           dist + ANUBARAK_HUMAN_SIDE_MARGIN <= Dist2d(player->GetPosition(), otherSpot);
}

// The side whose human tank stands at the other side's spot, else -1. Never between two humans: there's
// no bot side tank to move.
int8 HumanOnOtherSpot(Player* bot, ObjectGuid const sides[2], Position const spots[2])
{
    Player* tanks[2];
    for (uint8 side = 0; side < 2; ++side)
        tanks[side] = sides[side].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, sides[side]);

    for (uint8 side = 0; side < 2; ++side)
    {
        Player* other = tanks[1 - side];
        if (tanks[side] && !GET_PLAYERBOT_AI(tanks[side]) && (!other || GET_PLAYERBOT_AI(other)) &&
            StandsAtSpot(tanks[side], spots[1 - side], spots[side]))
            return static_cast<int8>(side);
    }

    return -1;
}

// Bots can't tell a human his side, so it's the tank spot he stands at. Swaps at once on a new side tank,
// else only after the dwell, so walking past doesn't swap
void FollowHumanSideTank(Player* bot, AnubarakState& state, std::vector<Creature*> const& spheres,
                         bool const placed[2], uint32 now)
{
    ObjectGuid const held[2] = {state.patch0, state.patch1};
    Position const spots[2] = {SideSpot(state, spheres, held, 0), SideSpot(state, spheres, held, 1)};
    if (HumanOnOtherSpot(bot, state.sideTanks, spots) < 0)
    {
        state.humanSwapValid = false;
        return;
    }

    if (!placed[0] && !placed[1])
    {
        if (!state.humanSwapValid)
        {
            state.humanSwapValid = true;
            state.humanSwapMs = now;
        }

        if (getMSTimeDiff(state.humanSwapMs, now) < ANUBARAK_HUMAN_SIDE_DWELL_MS)
            return;
    }

    std::swap(state.sideTanks[0], state.sideTanks[1]);
    state.humanSwapValid = false;
}

// Within the latch of this side's spot and nearer it than the other side's, so a patch far off its spot
// never latches to the wrong side
bool ServesSide(Unit* sphere, Position const& spot, Position const& otherSpot)
{
    float const dist = Dist2d(sphere->GetPosition(), spot);
    return dist <= ANUBARAK_TANK_PATCH_LATCH && dist <= Dist2d(sphere->GetPosition(), otherSpot);
}

int8 GrippedSide(AnubarakState const& state, ObjectGuid const& guid)
{
    auto const it = state.grippedFor.find(guid);
    return it == state.grippedFor.end() ? -1 : static_cast<int8>(it->second);
}

// A gripped sphere is the side's it was gripped for, wherever it drifted to, within the latch of the spot
// unless it was gripped from far off. Any other by ServesSide.
bool PatchFor(AnubarakState const& state, Unit* sphere, uint8 side, Position const& spot, Position const& otherSpot)
{
    int8 const gripped = GrippedSide(state, sphere->GetGUID());
    if (gripped >= 0)
        return gripped == side && (state.farGripped.count(sphere->GetGUID()) ||
                                   Dist2d(sphere->GetPosition(), spot) <= ANUBARAK_TANK_PATCH_LATCH);

    return ServesSide(sphere, spot, otherSpot);
}

bool FallingFor(AnubarakState const& state, std::vector<Creature*> const& spheres, uint8 side, Position const& spot,
                Position const& otherSpot)
{
    return std::any_of(spheres.begin(), spheres.end(), [&](Creature* sphere)
                       { return IsFrostSphereFalling(sphere) && PatchFor(state, sphere, side, spot, otherSpot); });
}

// The side's patch, else the sphere about to make it: its grip's landing or one falling for it
Creature* PatchToBe(AnubarakState const& state, std::vector<Creature*> const& spheres, ObjectGuid const held[2],
                    uint8 side)
{
    if (Creature* patch = FindSphere(spheres, held[side]); IsPermafrostPatch(patch))
        return patch;

    if (Creature* gripped = GripPending(state, spheres, side))
        return gripped;

    Position const spot = SideSpot(state, spheres, held, side);
    Position const otherSpot = SideSpot(state, spheres, held, 1 - side);
    for (Creature* sphere : spheres)
        if (IsFrostSphereFalling(sphere) && PatchFor(state, sphere, side, spot, otherSpot))
            return sphere;

    return nullptr;
}

// Holds each side's patch while it lasts. Never both sides on one patch, and a gripped one only where it
// keeps the gap to the other side's.
void RefreshTankPatches(AnubarakState& state, std::vector<Creature*> const& spheres)
{
    ObjectGuid held[2] = {state.patch0, state.patch1};
    for (ObjectGuid& guid : held)
        if (!IsPermafrostPatch(FindSphere(spheres, guid)))
            guid.Clear();

    for (uint8 side = 0; side < 2; ++side)
    {
        if (!held[side].IsEmpty())
            continue;

        Position const point = SideSpot(state, spheres, held, side);
        Position const otherPoint = SideSpot(state, spheres, held, 1 - side);
        Creature* other = FindSphere(spheres, held[1 - side]);
        Creature* nearest = nullptr;
        float nearestDist = 0.0f;
        bool nearestGripped = false;
        for (Creature* sphere : spheres)
        {
            if (!IsPermafrostPatch(sphere) || sphere == other || !PatchFor(state, sphere, side, point, otherPoint))
                continue;

            bool const gripped = GrippedSide(state, sphere->GetGUID()) == static_cast<int8>(side);
            if (gripped && other && Dist2d(sphere->GetPosition(), other->GetPosition()) < ANUBARAK_TANK_PATCH_GAP)
                continue;

            float const dist = Dist2d(sphere->GetPosition(), point);
            if (!nearest || (gripped && !nearestGripped) || (gripped == nearestGripped && dist < nearestDist))
            {
                nearest = sphere;
                nearestDist = dist;
                nearestGripped = gripped;
            }
        }

        if (nearest)
            held[side] = nearest->GetGUID();
    }

    for (auto it = state.grippedFor.begin(); it != state.grippedFor.end();)
    {
        bool const done = !FindSphere(spheres, it->first) || it->first == held[0] || it->first == held[1];
        if (done)
            state.farGripped.erase(it->first);

        it = done ? state.grippedFor.erase(it) : std::next(it);
    }

    state.patch0 = held[0];
    state.patch1 = held[1];
}

// The side the north exits feed, by fewer yards from exits to spots: a patch re-latched across the room
// moves its spot, and burrowers fed to the far tank get handed over while they hit the healers
uint8 NorthExitSide(AnubarakState const& state)
{
    Position const spots[2] = {state.spot0, state.spot1};
    auto const yards = [&spots](uint8 north)
    {
        float sum = 0.0f;
        for (Position const& exit : ANUBARAK_NORTH_EXITS)
            sum += Dist2d(exit, spots[north]);

        for (Position const& exit : ANUBARAK_SOUTH_EXITS)
            sum += Dist2d(exit, spots[1 - north]);

        return sum;
    };

    return yards(1) < yards(0) ? 1 : 0;
}

// A burrower is seen first at its exit, which fixes its side (NorthExitSide). A new one more than 10 s
// after the last opens a wave. It comes up there, and again at each emerge.
void TrackWaves(Player* bot, AnubarakState& state, uint32 now)
{
    uint8 const northSide = NorthExitSide(state);
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
    {
        ObjectGuid const guid = burrower->GetGUID();
        bool const down = burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
        auto const known = state.exits.find(guid);
        if (known != state.exits.end())
        {
            BurrowerExit& exit = known->second;
            if (exit.down && !down)
                exit.upMs = now;

            exit.down = down;
            continue;
        }

        bool const north = burrower->GetPositionY() >= ANUBARAK_ROOM_CENTER.GetPositionY();
        uint8 const side = north ? northSide : static_cast<uint8>(1 - northSide);
        state.exits[guid] = BurrowerExit{side, now, now, down};
        if (!state.waveSeen || getMSTimeDiff(state.lastWaveMs, now) > ANUBARAK_WAVE_GAP_MS)
        {
            state.waveSeen = true;
            state.lastWaveMs = now;
            ++state.wave;
        }
    }
}

// When the next wave spawns, the earliest the script allows. None while he is down, and the wave due
// after his submerge comes 5 s after the emerge instead.
uint32 NextWaveMs(AnubarakState const& state)
{
    uint32 const emerge = state.submergedMs + ANUBARAK_SUBMERGED_MS + ANUBARAK_WAVE_FIRST_MS;
    if (state.phaseValue == AnubarakPhase::Submerged)
        return emerge;

    bool const sinceSurface =
        state.waveSeen && static_cast<int32>(state.lastWaveMs - state.surfacedMs) >= 0;
    uint32 const next = sinceSurface ? state.lastWaveMs + ANUBARAK_WAVE_INTERVAL_MS
                                     : state.surfacedMs + ANUBARAK_WAVE_FIRST_MS;
    if (state.phaseValue != AnubarakPhase::Surface)
        return next;

    uint32 const submerge = state.surfacedMs + ANUBARAK_SURFACED_MS;
    return static_cast<int32>(next - submerge) > 0 ? submerge + ANUBARAK_SUBMERGED_MS + ANUBARAK_WAVE_FIRST_MS
                                                   : next;
}

int32 MsUntilWave(AnubarakState const& state, uint32 now) { return static_cast<int32>(NextWaveMs(state) - now); }

// His health over its net loss a second across the samples kept, -1 while he isn't losing any or the
// samples span under ANUBARAK_SWARM_TTD_MIN_MS
float SwarmTimeToDieS(AnubarakState const& state)
{
    if (state.swarmHealth.size() < 2)
        return -1.0f;

    auto const& [firstMs, firstHealth] = state.swarmHealth.front();
    auto const& [lastMs, lastHealth] = state.swarmHealth.back();
    uint32 const span = getMSTimeDiff(firstMs, lastMs);
    if (span < ANUBARAK_SWARM_TTD_MIN_MS || lastHealth >= firstHealth)
        return -1.0f;

    float const perS = static_cast<float>(firstHealth - lastHealth) * IN_MILLISECONDS / static_cast<float>(span);
    return static_cast<float>(lastHealth) / perS;
}

bool SwarmBerserkBehind(AnubarakState const& state, Creature* boss, uint32 now)
{
    uint32 const since = getMSTimeDiff(state.swarmMs, now);
    if (!state.swarm || since < ANUBARAK_SWARM_TTD_WINDOW_MS)
        return false;

    float const health = boss->GetHealthPct();
    float const perS = (state.swarmHpPct - health) * IN_MILLISECONDS / since;
    float const berserkLeftS =
        static_cast<float>(static_cast<int32>(state.pullMs + ANUBARAK_BERSERK_MS - now)) / IN_MILLISECONDS;
    return perS <= 0.0f || health / perS > berserkLeftS - ANUBARAK_SWARM_BERSERK_MARGIN_S;
}

// Heroic phase 3: samples his health once a second and re-reads the wave every ANUBARAK_SWARM_WAVE_READ_MS,
// so a wave left to the side tanks still becomes the focus once his health loss stalls
void ReadSwarmWave(Player* bot, AnubarakState& state, Creature* boss, uint32 now)
{
    if (state.swarmHealth.empty() ||
        getMSTimeDiff(state.swarmHealth.back().first, now) >= ANUBARAK_SWARM_HEALTH_SAMPLE_MS)
    {
        state.swarmHealth.emplace_back(now, boss->GetHealth());
        while (state.swarmHealth.size() > 2 &&
               getMSTimeDiff(state.swarmHealth.front().first, now) > ANUBARAK_SWARM_TTD_WINDOW_MS)
            state.swarmHealth.pop_front();
    }

    size_t alive = 0;
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
        alive += state.swarmBurrowers.count(burrower->GetGUID()) ? 0 : 1;

    bool const arrived = alive > state.waveAlive;
    state.waveAlive = alive;
    if (!alive || (!arrived && getMSTimeDiff(state.waveReadMs, now) < ANUBARAK_SWARM_WAVE_READ_MS))
        return;

    state.waveReadMs = now;
    float const ttd = SwarmTimeToDieS(state);
    state.waveLeave = ttd >= 0.0f && ttd < ANUBARAK_SWARM_IGNORE_S;
}

// Permafrost stays until the spike takes it, so a kite patch gone from the sweep while he's down is spent
void CountSpentKitePatches(AnubarakState& state, std::vector<Creature*> const& spheres)
{
    ObjectGuid const tanks[2] = {state.patch0, state.patch1};
    std::unordered_set<ObjectGuid>& landed = state.kitePatchesLanded;
    landed.clear();
    for (Creature* sphere : spheres)
        if (IsPermafrostPatch(sphere) && sphere->GetGUID() != tanks[0] && sphere->GetGUID() != tanks[1])
            landed.insert(sphere->GetGUID());

    if (state.phaseValue == AnubarakPhase::Submerged)
        for (ObjectGuid const& guid : state.kitePatchesSeen)
            if (!landed.count(guid) && guid != tanks[0] && guid != tanks[1])
                ++state.kitePatchesSpent;

    state.kitePatchesSeen.swap(landed);
}

// While he's up: samples his health and the share of DPS bots on him once a second, and latches the push
// once all of them on him would get him under 30% before he submerges
void ReadPush(Player* bot, AnubarakState& state, Creature* boss, uint32 now)
{
    if (state.push ||
        (!state.pushSamples.empty() && getMSTimeDiff(state.pushSamples.back().ms, now) < ANUBARAK_PUSH_SAMPLE_MS))
        return;

    uint32 dps = 0;
    uint32 on = 0;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (member->IsAlive() && GET_PLAYERBOT_AI(member) && IsToCDpsBot(member))
        {
            ++dps;
            on += member->GetTarget() == boss->GetGUID() ? 1 : 0;
        }

        return false;
    });

    float const health = boss->GetHealthPct();
    state.pushSamples.push_back({now, health, dps ? static_cast<float>(on) / dps : 0.0f});

    PushSample const& first = state.pushSamples.front();
    uint32 const span = getMSTimeDiff(first.ms, now);
    int32 const leftMs = static_cast<int32>(state.surfacedMs + ANUBARAK_SURFACED_MS - now);
    float const lost = first.healthPct - health;
    if (span < ANUBARAK_PUSH_MIN_SPAN_MS || leftMs <= 0 || leftMs > static_cast<int32>(ANUBARAK_PUSH_WINDOW_MS) ||
        lost <= 0.0f)
        return;

    float shareSum = 0.0f;
    for (PushSample const& sample : state.pushSamples)
        shareSum += sample.share;

    float const share = std::max(shareSum / state.pushSamples.size(), ANUBARAK_PUSH_SHARE_FLOOR);
    float const allOnPerS = lost * IN_MILLISECONDS / span / share * ANUBARAK_PUSH_DISCOUNT;
    float const neededS = (health - ANUBARAK_PUSH_TARGET_PCT) / allOnPerS;
    float const leftS = static_cast<float>(leftMs) / IN_MILLISECONDS;
    if (neededS > leftS)
        return;

    state.push = true;
    state.pushMs = now;
    state.pushHpPct = health;
    state.pushPerS = allOnPerS;
}

// Null while he isn't engaged. Refreshes phase, latches and probes once per instance per ms.
AnubarakState* ReadPhase(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!InToCInstance(bot) || !AnubarakEngaged(botAI))
        return nullptr;

    Creature* boss = GetAnubarak(bot);
    AnubarakState* state = boss ? Sweep(bot) : nullptr;
    if (!state)
        return nullptr;

    uint32 const now = getMSTime();
    if (state->phaseMemoValid && state->phaseMemoMs == now)
        return state;

    // Gated triggers never read the phase while he's out of combat, so a wipe is only seen as this gap
    if (!state->engagedSeen || getMSTimeDiff(state->lastEngagedMs, now) > ANUBARAK_PULL_GAP_MS)
    {
        state->engagedSeen = true;
        state->pullMs = now;
        ClearPullLatches(*state);
    }

    state->lastEngagedMs = now;
    state->phaseMemoValid = true;
    state->phaseMemoMs = now;

    AnubarakPhase const before = state->phaseValue;
    if (boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || boss->HasAura(SPELL_SUBMERGE_ANUB))
        state->phaseValue = AnubarakPhase::Submerged;
    else
    {
        if (!state->swarm && SwarmStarted(bot, boss))
        {
            state->swarm = true;
            state->swarmMs = now;
            state->swarmHpPct = boss->GetHealthPct();
            for (auto const& seen : state->exits)
                if (Creature* burrower = ResolveCreature(bot, seen.first); burrower && burrower->IsAlive())
                    state->swarmBurrowers.insert(seen.first);
        }

        state->phaseValue = state->swarm ? AnubarakPhase::Swarm : AnubarakPhase::Surface;
    }

    if (state->phaseValue == AnubarakPhase::Surface &&
        (before == AnubarakPhase::None || before == AnubarakPhase::Submerged))
    {
        state->surfacedMs = now;
        state->surfacedHpPct = boss->GetHealthPct();
        state->submergeSpotValid = false;
        state->submergeDragStarted = false;
        state->kiteBudgetSet = false;
    }

    bool const submerging = state->phaseValue == AnubarakPhase::Submerged && before == AnubarakPhase::Surface;
    if (submerging)
    {
        ++state->submerges;
        state->kitePatchesSpent = 0;
        state->submergedMs = now;
        state->submergePosValid = true;
        state->submergePos = boss->GetPosition();
        // a phase 1 after this one walks the healers behind again, one at a time
        state->behindSet = false;
        state->behindStands.clear();
        state->behindGone.clear();
    }

    TrackBossStillness(*state, boss, now);
    if (state->phaseValue == AnubarakPhase::Submerged)
    {
        state->drag = DragLegLatch();
        state->dragFreeValid = false;
    }

    if (state->phaseValue == AnubarakPhase::Surface)
        ReadPush(bot, *state, boss, now);
    else
    {
        state->pushSamples.clear();
        state->push = false;
    }

    if (state->swarm && AnubarakHeroic(bot))
        ReadSwarmWave(bot, *state, boss, now);

    std::vector<Creature*> const spheres = ResolveSpheres(bot, *state);
    std::vector<Position> lostAt;
    for (uint8 side = 0; side < 2; ++side)
    {
        ObjectGuid const guid = side == 0 ? state->patch0 : state->patch1;
        if (guid.IsEmpty())
            continue;

        Creature* patch = FindSphere(spheres, guid);
        if (IsPermafrostPatch(patch))
        {
            state->tankPatchSeen[side] = true;
            state->tankPatchPos[side] = patch->GetPosition();
            continue;
        }

        if (state->tankPatchSeen[side])
            lostAt.push_back(state->tankPatchPos[side]);

        state->tankPatchSeen[side] = false;
    }

    RecordSphereHomes(*state, spheres);
    RefreshTankPatches(*state, spheres);
    if (AnubarakHeroic(bot))
        CountSpentKitePatches(*state, spheres);
    if (!state->patch0.IsEmpty() && !state->patch1.IsEmpty())
        state->firstPairDone = true;

    UpdateRefillSpots(bot, *state, spheres);

    bool placed[2] = {false, false};
    RefreshSideTanks(bot, *state, placed);
    FollowHumanSideTank(bot, *state, spheres, placed, now);

    ObjectGuid const held[2] = {state->patch0, state->patch1};
    Position const spots[2] = {SideSpot(*state, spheres, held, 0), SideSpot(*state, spheres, held, 1)};
    state->spot0 = spots[0];
    state->spot1 = spots[1];
    UpdateRaidPoint(bot, *state, spots[0], spots[1]);
    if (state->phaseValue == AnubarakPhase::Submerged && AnubarakHeroic(bot))
        for (Position const& patch : lostAt)
            StepGatherOff(bot, *state, patch);

    TrackWaves(bot, *state, now);
    TrackKiter(*state, GetSpikeTarget(bot));

    return state;
}

// A bot refreshes the latches through its own read. A player gets whatever the last bot read left.
AnubarakState const* LatchesFor(Player* player)
{
    if (!InToCInstance(player))
        return nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        return ReadPhase(botAI);

    return anubarakStates.Find(player->GetInstanceId());
}

Creature* LatchedTankPatch(Player* bot, AnubarakState const& state, uint8 side)
{
    Creature* patch = ResolveCreature(bot, side == 0 ? state.patch0 : state.patch1);
    return IsPermafrostPatch(patch) ? patch : nullptr;
}

std::vector<Position> LatchedTankSpots(Player* bot, AnubarakState const& state)
{
    std::vector<Position> tanks;
    for (uint8 side = 0; side < 2; ++side)
        if (Creature* patch = LatchedTankPatch(bot, state, side))
            tanks.push_back(patch->GetPosition());

    return tanks;
}

// Heroic never kites onto a stray patch this close to a tank patch: the spike's first test after a fail
// reaches 11.5 yd plus its 5.25 yd first walk
bool NearTankPatch(Position const& patch, std::vector<Position> const& tankSpots)
{
    return std::any_of(tankSpots.begin(), tankSpots.end(), [&patch](Position const& tank)
                       { return Dist2d(patch, tank) < ANUBARAK_SUBMERGE_PATCH_CLEARANCE_MIN; });
}

std::vector<Position> PatchPositions(Player* bot, AnubarakState const& state)
{
    std::vector<Position> patches;
    for (Creature* sphere : ResolveAlive(bot, state.spheres))
        if (IsPermafrostPatch(sphere))
            patches.push_back(sphere->GetPosition());

    return patches;
}

// Heroic phase 2: the raid point, where the healers already stand. He is dragged 25+ yd off it before he
// goes down, and the spike is summoned there.
bool GatherCentre(Player* bot, AnubarakState const& state, Position& out)
{
    if (state.phaseValue != AnubarakPhase::Submerged || !state.raidPointValid || !AnubarakHeroic(bot))
        return false;

    out = state.raidPoint;
    return true;
}

bool TankWalkClear(Position const& from, Position const& to, std::vector<Position> const& patches)
{
    return std::none_of(patches.begin(), patches.end(), [&](Position const& patch)
                        {
                            return Dist2d(to, patch) < ANUBARAK_TANK_KEEPOUT ||
                                   SegmentDistance2d(from, to, patch) < ANUBARAK_PERMAFROST_SLOW_REACH;
                        });
}

bool SpikeBehind(Player* kiter, Creature* spike, Position const& stand)
{
    return spike->GetExactDist2d(stand.GetPositionX(), stand.GetPositionY()) >
           kiter->GetExactDist2d(stand.GetPositionX(), stand.GetPositionY());
}

// Permafrost's slow drops a moment after the kiter steps off the patch, so it never decides the race
float KiterRunSpeed(Player* kiter)
{
    float const speed = kiter->GetSpeed(MOVE_RUN);
    return UnitOnPermafrost(kiter) ? std::max(speed, playerBaseMoveSpeed[MOVE_RUN]) : speed;
}

AnubarakPursuit::Point ToPoint(Position const& pos) { return {pos.GetPositionX(), pos.GetPositionY()}; }

// The speed step comes off the aura it carries: none at all means it idles after a patch and restarts
// from the base speed
AnubarakPursuit::Spike PursuitSpike(Creature* spike)
{
    AnubarakPursuit::Spike out;
    out.pos = ToPoint(spike->GetPosition());
    out.speed = spike->GetSpeed(MOVE_RUN);
    if (spike->HasAura(SPELL_SPIKE_SPEED3))
        out.step = AnubarakPursuit::LAST_STEP;
    else if (Aura const* aura = spike->GetAura(SPELL_SPIKE_SPEED2))
    {
        out.step = 1;
        out.nextStepS = static_cast<float>(std::max(aura->GetDuration(), 0)) / IN_MILLISECONDS;
    }
    else if (Aura const* aura = spike->GetAura(SPELL_SPIKE_SPEED1))
        out.nextStepS = static_cast<float>(std::max(aura->GetDuration(), 0)) / IN_MILLISECONDS;

    return out;
}

// How far off the spike and its lane a bot dodges, further while the spike outruns it
struct SpikeDodgeReach
{
    float danger;
    float clearance;
    float lane;
    float laneClearance;
};

SpikeDodgeReach DodgeReach(Player* bot, Creature* spike)
{
    if (spike->GetSpeed(MOVE_RUN) > bot->GetSpeed(MOVE_RUN))
        return {ANUBARAK_SPIKE_DANGER_RADIUS_FAST, ANUBARAK_SPIKE_CLEARANCE_FAST, ANUBARAK_SPIKE_LANE_HALF_WIDTH_FAST,
                ANUBARAK_SPIKE_LANE_CLEARANCE_FAST};

    return {ANUBARAK_SPIKE_DANGER_RADIUS, ANUBARAK_SPIKE_CLEARANCE, ANUBARAK_SPIKE_LANE_HALF_WIDTH,
            ANUBARAK_SPIKE_LANE_CLEARANCE};
}

// Walking from a to target cuts deeper into the patch's slow reach than either end already is
bool PathClips(Position const& a, Position const& target, Position const& patch)
{
    float const limit = std::min({ANUBARAK_PERMAFROST_SLOW_REACH + 1.0f, Dist2d(target, patch), Dist2d(a, patch)});
    return SegmentDistance2d(a, target, patch) < limit - ANUBARAK_PATH_CLIP_SLACK;
}

// Next corner of a square just outside the patch's slow reach: legs along a side never clip, while a point
// straight out to the side leaves a leg to the stand that cuts to 6 yd of the patch
Position DetourWaypoint(Player* kiter, Position const& patch, Position const& stand)
{
    Position const from = kiter->GetPosition();
    Vec2 u = Delta(patch, stand);
    float const length = Length(u);
    u = length > 0.0f ? Vec2{u.x / length, u.y / length} : Vec2{1.0f, 0.0f};
    Vec2 const perp = {-u.y, u.x};
    Vec2 const toKiter = Delta(patch, from);
    float const kiterSide = toKiter.x * perp.x + toKiter.y * perp.y >= 0.0f ? 1.0f : -1.0f;
    float const half = ANUBARAK_PERMAFROST_SLOW_REACH + 1.0f + ANUBARAK_DETOUR_MARGIN;

    for (float side : {kiterSide, -kiterSide})
    {
        Position const across = Offset(patch, perp, side * half);
        for (float along : {half, -half})
        {
            Position const corner = Offset(across, u, along);
            Position valid;
            if (!PathClips(from, corner, patch) && OnFloor(kiter, corner, valid))
                return valid;
        }
    }

    // No clean corner: step straight out from the patch first
    Vec2 out = toKiter;
    float const outLength = Length(out);
    out = outLength > 0.0f ? Vec2{out.x / outLength, out.y / outLength} : Vec2{-u.x, -u.y};
    return ValidateFloorPoint(kiter, Offset(patch, out, half));
}

// What the heroic spike's run keeps clear of: every patch, the phase 2 gather, and every living player
// outside it but the kiter, since Impale hits whoever the spike passes
struct LaneObstacle
{
    Position pos;
    float clearance;
    // A latched tank patch
    bool tank = false;
    bool gather = false;
    bool player = false;
};

std::vector<LaneObstacle> LaneObstacles(Player* kiter, AnubarakState const& state)
{
    std::vector<Position> tanks;
    for (uint8 side = 0; side < 2; ++side)
        if (Creature* patch = LatchedTankPatch(kiter, state, side))
            tanks.push_back(patch->GetPosition());

    std::vector<LaneObstacle> obstacles;
    for (Position const& patch : PatchPositions(kiter, state))
    {
        bool const tank = std::any_of(tanks.begin(), tanks.end(),
                                      [&patch](Position const& held) { return Dist2d(held, patch) < 0.5f; });
        obstacles.push_back({patch, ANUBARAK_SPIKE_ROUTE_CLEARANCE, tank, false});
    }

    Position gather;
    bool const hasGather = GatherCentre(kiter, state, gather);
    if (hasGather)
        obstacles.push_back({gather, ANUBARAK_KITE_GATHER_CLEARANCE, false, true});

    ForEachGroupMember(kiter, [&](Player* member)
    {
        if (member != kiter && member->IsAlive() && member->GetPositionZ() < ANUBARAK_PIT_CEILING_Z &&
            (!hasGather || Dist2d(member->GetPosition(), gather) > ANUBARAK_KITE_GATHER_CLEARANCE))
            obstacles.push_back({member->GetPosition(), ANUBARAK_KITE_PLAYER_CLEARANCE, false, false, true});

        return false;
    });

    return obstacles;
}

// Twice the signed area of a, b, p; its sign says which side of a to b p is on
float Cross(Position const& a, Position const& b, Position const& p)
{
    Vec2 const ab = Delta(a, b);
    Vec2 const ap = Delta(a, p);
    return ab.x * ap.y - ab.y * ap.x;
}

// A flat triangle reads as holding every point, so one under a square yard holds none
bool InsideTriangle(Position const& a, Position const& b, Position const& c, Position const& p)
{
    if (std::fabs(Cross(a, b, c)) < 2.0f)
        return false;

    float const d1 = Cross(a, b, p);
    float const d2 = Cross(b, c, p);
    float const d3 = Cross(c, a, p);
    bool const negative = d1 < 0.0f || d2 < 0.0f || d3 < 0.0f;
    bool const positive = d1 > 0.0f || d2 > 0.0f || d3 > 0.0f;
    return !(negative && positive);
}

// A leg that starts inside the clearance only has to head away
bool LegClears(Position const& from, Position const& to, LaneObstacle const& obstacle)
{
    float const need =
        std::min(obstacle.clearance, Dist2d(from, obstacle.pos) - ANUBARAK_KITE_LANE_START_SLACK);
    return SegmentDistance2d(from, to, obstacle.pos) >= need;
}

// The spike chases the kiter, so its run to the step stays inside the triangle spike, kiter, step.
// walked is false for a Blink, which leaves no trail for the spike to cut along.
bool LaneClears(Position const& spike, Position const& kiter, Position const& step,
                std::vector<LaneObstacle> const& obstacles, bool walked)
{
    return std::none_of(obstacles.begin(), obstacles.end(), [&](LaneObstacle const& obstacle)
                        {
                            return InsideTriangle(spike, kiter, step, obstacle.pos) ||
                                   !LegClears(spike, step, obstacle) || (walked && !LegClears(kiter, step, obstacle));
                        });
}

constexpr uint32 SPRINT_RANKS[] = {11305, 8696, 2983};
constexpr uint32 DASH_RANKS[] = {33357, 9821, 1850};
constexpr uint32 HAND_OF_PROTECTION_RANKS[] = {10278, 5599, 1022};
constexpr uint32 SPELL_BLINK = 1953;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
constexpr uint32 SPELL_CIRCLE_SUMMON = 48018;
constexpr uint32 SPELL_CIRCLE_TELEPORT = 48020;
// On the warlock while its circle is within the teleport's 40 yd
constexpr uint32 SPELL_CIRCLE_IN_RANGE = 62388;
constexpr uint32 SPELL_CAT_FORM = 768;
constexpr uint32 SPELL_FERAL_CHARGE_BEAR = 16979;
constexpr uint32 SPELL_FERAL_CHARGE_CAT = 49376;
// Feral Charge's 8-25 yd, and only onto a bug this much farther from the spike, still on the floor
constexpr float ANUBARAK_FERAL_CHARGE_MIN = 8.0f;
constexpr float ANUBARAK_FERAL_CHARGE_MAX = 25.0f;
constexpr float ANUBARAK_FERAL_CHARGE_GAIN = 15.0f;
constexpr float ANUBARAK_FERAL_CHARGE_FLOOR = 45.0f;

template <size_t N>
uint32 KnownRank(Player* player, uint32 const (&ranks)[N])
{
    for (uint32 rank : ranks)
        if (player->HasSpell(rank))
            return rank;

    return 0;
}

// Known, instant, off cooldown and affordable
bool ReadyInstant(Player* player, uint32 spellId)
{
    SpellInfo const* info = spellId && player->HasSpell(spellId) ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info || player->HasSpellCooldown(spellId) || info->CalcCastTime(player) ||
        info->PowerType >= static_cast<uint32>(MAX_POWERS))
        return false;

    return info->CalcPowerCost(player, info->GetSchoolMask()) <=
           static_cast<int32>(player->GetPower(Powers(info->PowerType)));
}

float SpeedBoostPct(Aura const* aura)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (AuraEffect const* effect = aura->GetEffect(i))
            if (effect->GetAuraType() == SPELL_AURA_MOD_INCREASE_SPEED)
                return static_cast<float>(effect->GetAmount());

    return 0.0f;
}

// Sprint, Dash and Nitro Boosts share one speed aura type, so only the fastest counts
Aura const* RunningSpeedBoost(Player* kiter)
{
    Aura const* best = nullptr;
    auto const take = [&](uint32 spell)
    {
        Aura const* aura = kiter->GetAura(spell);
        if (aura && (!best || SpeedBoostPct(aura) > SpeedBoostPct(best)))
            best = aura;
    };

    take(NITRO_BOOSTS_SPEED_SPELL);
    for (uint32 rank : SPRINT_RANKS)
        take(rank);

    for (uint32 rank : DASH_RANKS)
        take(rank);

    return best;
}

bool SpeedBoostRunning(Player* kiter) { return RunningSpeedBoost(kiter) != nullptr; }

bool IsSpeedBoostSpell(uint32 spell)
{
    return std::find(std::begin(SPRINT_RANKS), std::end(SPRINT_RANKS), spell) != std::end(SPRINT_RANKS) ||
           std::find(std::begin(DASH_RANKS), std::end(DASH_RANKS), spell) != std::end(DASH_RANKS);
}

// Sprint, Dash or Nitro Boosts ready with none running. Dash gives no speed outside Cat Form, so heroic
// shifts the druid into it first.
bool ReadySpeedBoost(Player* kiter, AnubarakKiteBoost& out)
{
    out = AnubarakKiteBoost();
    if (SpeedBoostRunning(kiter))
        return false;

    uint32 rank = 0;
    bool shift = false;
    if (kiter->getClass() == CLASS_ROGUE)
        rank = KnownRank(kiter, SPRINT_RANKS);
    else if (kiter->getClass() == CLASS_DRUID)
    {
        bool const cat = kiter->GetShapeshiftForm() == FORM_CAT;
        if (cat || (AnubarakHeroic(kiter) && ReadyInstant(kiter, SPELL_CAT_FORM)))
        {
            rank = KnownRank(kiter, DASH_RANKS);
            shift = !cat;
        }
    }

    if (rank && ReadyInstant(kiter, rank))
    {
        out.spellId = rank;
        out.catForm = shift;
        return true;
    }

    out.nitro = NitroBoostsReady(kiter);
    return out.nitro;
}

GameObject* WarlockCircle(Player* player)
{
    return player->getClass() == CLASS_WARLOCK ? player->GetGameObject(SPELL_CIRCLE_SUMMON) : nullptr;
}

// The warlock's circle while it can teleport there now: up, within the teleport's 40 yd, the spell ready
GameObject* TeleportCircle(Player* kiter)
{
    GameObject* circle = WarlockCircle(kiter);
    return circle && kiter->HasAura(SPELL_CIRCLE_IN_RANGE) && ReadyInstant(kiter, SPELL_CIRCLE_TELEPORT) ? circle
                                                                                                         : nullptr;
}

// The catch is due: the spike within Impale's reach plus what it closes over the own immunity's lead
bool KiteCatchDue(Player* kiter, Creature* spike)
{
    float const closing = std::max(spike->GetSpeed(MOVE_RUN) - KiterRunSpeed(kiter), 0.0f);
    return Dist2d(kiter->GetPosition(), spike->GetPosition()) <=
           ANUBARAK_IMPALE_REACH + closing * ANUBARAK_KITE_OWN_LEAD_S;
}

// Heroic druid's last tool once the catch is due: Feral Charge onto the burrower or scarab that gains most on the
// spike, landing on the floor off the tank patches with the spike's lane there clear
bool FeralChargeBoost(Player* kiter, Creature* spike, AnubarakState const& state,
                      std::vector<LaneObstacle> const& obstacles, AnubarakKiteBoost& out)
{
    ShapeshiftForm const form = kiter->GetShapeshiftForm();
    uint32 const spell = form == FORM_CAT                              ? SPELL_FERAL_CHARGE_CAT
                         : form == FORM_BEAR || form == FORM_DIREBEAR ? SPELL_FERAL_CHARGE_BEAR
                                                                       : 0;
    if (!spell || !ReadyInstant(kiter, spell) || !KiteCatchDue(kiter, spike))
        return false;

    float const fromSpike = Dist2d(kiter->GetPosition(), spike->GetPosition());
    std::vector<Position> const tanks = LatchedTankSpots(kiter, state);
    Creature* best = nullptr;
    float bestGain = 0.0f;
    for (std::vector<ObjectGuid> const* guids : {&state.burrowers, &state.scarabs})
        for (Creature* bug : ResolveAlive(kiter, *guids))
        {
            Position const& at = bug->GetPosition();
            float const reach = kiter->GetDistance(bug);
            float const gain = Dist2d(at, spike->GetPosition()) - fromSpike;
            if (reach < ANUBARAK_FERAL_CHARGE_MIN || reach > ANUBARAK_FERAL_CHARGE_MAX ||
                gain < ANUBARAK_FERAL_CHARGE_GAIN || (best && gain <= bestGain) ||
                Dist2d(at, ANUBARAK_ROOM_CENTER) > ANUBARAK_FERAL_CHARGE_FLOOR || NearTankPatch(at, tanks) ||
                !kiter->IsWithinLOSInMap(bug) ||
                !LaneClears(spike->GetPosition(), kiter->GetPosition(), at, obstacles, false))
                continue;

            best = bug;
            bestGain = gain;
        }

    if (!best)
        return false;

    out.spellId = spell;
    out.target = best->GetGUID();
    return true;
}

// A bot warlock's teleport goes at the catch: its circle ready in reach, no speed boost running, and the
// circle well behind the spike
bool TeleportCovers(Player* kiter, Creature* spike)
{
    GameObject* circle = GET_PLAYERBOT_AI(kiter) ? TeleportCircle(kiter) : nullptr;
    return circle && !SpeedBoostRunning(kiter) &&
           Dist2d(circle->GetPosition(), spike->GetPosition()) >=
               Dist2d(kiter->GetPosition(), spike->GetPosition()) + ANUBARAK_CIRCLE_TELEPORT_GAIN;
}

// Next boost in the chain. The teleport and Feral Charge go last, with no speed boost ready or running and the
// catch due: teleporting earlier runs the warlock's Nitro back across the circle and lands it next to the spike
bool NextKiteBoost(Player* kiter, Creature* spike, AnubarakState const& state,
                   std::vector<LaneObstacle> const& obstacles, AnubarakKiteBoost& out)
{
    if (ReadySpeedBoost(kiter, out))
        return true;

    bool const heroic = AnubarakHeroic(kiter);
    if (kiter->getClass() == CLASS_MAGE && ReadyInstant(kiter, SPELL_BLINK))
    {
        float const facing = kiter->GetOrientation();
        Position const landing =
            Offset(kiter->GetPosition(), {std::cos(facing), std::sin(facing)}, ANUBARAK_BLINK_DISTANCE);
        if (Dist2d(landing, ANUBARAK_ROOM_CENTER) <= ANUBARAK_PIT_RADIUS - ANUBARAK_BLINK_DISTANCE / 2.0f &&
            Dist2d(landing, spike->GetPosition()) > Dist2d(kiter->GetPosition(), spike->GetPosition()) &&
            LaneClears(spike->GetPosition(), kiter->GetPosition(), landing, obstacles, false))
        {
            out.spellId = SPELL_BLINK;
            out.face = true;
            out.facing = facing;
            return true;
        }
    }

    if (SpeedBoostRunning(kiter))
        return false;

    if (TeleportCovers(kiter, spike) && KiteCatchDue(kiter, spike))
    {
        out.spellId = SPELL_CIRCLE_TELEPORT;
        return true;
    }

    return heroic && kiter->getClass() == CLASS_DRUID && FeralChargeBoost(kiter, spike, state, obstacles, out);
}

// The kiter's run speed on the boost, 0 for Blink, the circle and Feral Charge, which jump
float BoostRunSpeed(Player* kiter, AnubarakKiteBoost const& boost)
{
    if (boost.nitro)
        return KiterRunSpeed(kiter) * NITRO_BOOSTS_SPEED_FACTOR;

    SpellInfo const* info = boost.spellId ? sSpellMgr->GetSpellInfo(boost.spellId) : nullptr;
    if (!info)
        return 0.0f;

    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.ApplyAuraName == SPELL_AURA_MOD_INCREASE_SPEED)
            return KiterRunSpeed(kiter) * (1.0f + static_cast<float>(effect.CalcValue()) / 100.0f);

    return 0.0f;
}

float BoostDurationS(AnubarakKiteBoost const& boost)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(boost.nitro ? NITRO_BOOSTS_SPEED_SPELL : boost.spellId);
    return info ? static_cast<float>(std::max(info->GetMaxDuration(), 0)) / IN_MILLISECONDS : 0.0f;
}

// A running speed boost counts for what's left of it, the run slowing once it ends
AnubarakPursuit::Pace KiterPace(Player* kiter)
{
    float const now = KiterRunSpeed(kiter);
    if (Aura const* aura = RunningSpeedBoost(kiter))
    {
        // Dash out of Cat Form gives nothing, so its end can't slow the kiter below what it runs now
        float const after =
            std::max(now / (1.0f + SpeedBoostPct(aura) / 100.0f), std::min(now, playerBaseMoveSpeed[MOVE_RUN]));
        return AnubarakPursuit::Pace(after, now, static_cast<float>(std::max(aura->GetDuration(), 0)) / IN_MILLISECONDS);
    }

    return AnubarakPursuit::Pace(now);
}

// The Hand time left at which the boost goes under a Hand
float BoostHandLeadS(Player* kiter, Creature* spike, AnubarakKiteBoost const& boost)
{
    float const speed = BoostRunSpeed(kiter, boost);
    if (speed <= 0.0f)
        return ANUBARAK_KITE_BOOST_HAND_S;

    return std::max(ANUBARAK_KITE_BOOST_HAND_S,
                    AnubarakPursuit::BoostLeadS(Dist2d(spike->GetPosition(), kiter->GetPosition()), speed,
                                                spike->GetSpeed(MOVE_RUN), ANUBARAK_IMPALE_REACH,
                                                ANUBARAK_KITE_BOOST_MARGIN, ANUBARAK_KITE_BOOST_HAND_MAX_S));
}

bool UnderHandOfProtection(Unit* unit)
{
    return std::any_of(std::begin(HAND_OF_PROTECTION_RANKS), std::end(HAND_OF_PROTECTION_RANKS),
                       [unit](uint32 rank) { return unit->HasAura(rank); });
}

// A kite's end patch rank, lower first, -1 never. Heroic takes only kite patches clear of the tank ones: its
// spheres never respawn, so a spent tank one leaves its side bare. Normal ranks a tank one last while a burrower lives
int8 KitePatchRank(Player* kiter, AnubarakState const& state, Creature* sphere, bool heroic, bool burrowers)
{
    if (!IsPermafrostPatch(sphere))
        return -1;

    int8 side = -1;
    std::vector<Position> tankSpots;
    for (uint8 s = 0; s < 2; ++s)
    {
        Creature* patch = LatchedTankPatch(kiter, state, s);
        if (!patch)
            continue;

        tankSpots.push_back(patch->GetPosition());
        if (patch == sphere)
            side = static_cast<int8>(s);
    }

    if (!heroic)
        return side >= 0 && burrowers ? 1 : 0;

    if (side >= 0)
        return -1;

    return NearTankPatch(sphere->GetPosition(), tankSpots) ? -1 : 0;
}

// The spike at its base speed tests only every 1.5 s, so a kiter marked inside Impale's reach that
// outruns it is gone before the first test: only a catch after that counts
float BaseTestGraceS(AnubarakPursuit::Spike const& chase, AnubarakPursuit::Pace const& pace, Player* kiter,
                     Creature* spike)
{
    return chase.step == 0 && pace.SpeedAt(0.0f) > chase.speed &&
                   Dist2d(spike->GetPosition(), kiter->GetPosition()) <= ANUBARAK_IMPALE_REACH
               ? ANUBARAK_SPIKE_BASE_TEST_S
               : 0.0f;
}

float HandLeftS(Unit* kiter)
{
    int32 left = 0;
    for (uint32 rank : HAND_OF_PROTECTION_RANKS)
        if (Aura const* aura = kiter->GetAura(rank))
            left = std::max(left, aura->GetDuration());

    return static_cast<float>(left) / IN_MILLISECONDS;
}

// Known, ready and affordable, the GCD aside: CanCastSpell fails on that, and a pick made on it flips
bool HandReady(Player* paladin)
{
    return paladin->getClass() == CLASS_PALADIN && ReadyInstant(paladin, KnownRank(paladin, HAND_OF_PROTECTION_RANKS));
}

constexpr uint32 SPELL_ICE_BLOCK = 45438;
constexpr uint32 SPELL_HYPOTHERMIA = 41425;
constexpr uint32 VANISH_RANKS[] = {26889, 1857, 1856};
constexpr uint32 SPELL_FEIGN_DEATH = 5384;

// The kiter's own Divine Shield, Ice Block, Vanish or Feign Death ready now, else 0
uint32 OwnImmunitySpell(Player* kiter)
{
    uint32 spell = 0;
    switch (kiter->getClass())
    {
        case CLASS_PALADIN:
            spell = kiter->HasAura(SPELL_FORBEARANCE) ? 0 : SPELL_DIVINE_SHIELD;
            break;
        case CLASS_MAGE:
            spell = kiter->HasAura(SPELL_HYPOTHERMIA) ? 0 : SPELL_ICE_BLOCK;
            break;
        case CLASS_ROGUE:
            spell = KnownRank(kiter, VANISH_RANKS);
            break;
        case CLASS_HUNTER:
            spell = SPELL_FEIGN_DEATH;
            break;
        default:
            break;
    }

    return spell && GET_PLAYERBOT_AI(kiter) && ReadyInstant(kiter, spell) ? spell : 0;
}

std::vector<Player*> ProtectorOrder(Player* bot);

// A side tank bot with a burrower of its side to hold or take
static bool HoldsSideBurrower(Player* player)
{
    PlayerbotAI* playerAI = GET_PLAYERBOT_AI(player);
    return playerAI && GetAnubarakBurrowerTankSide(player) >= 0 && GetAnubarakBurrowerPick(playerAI);
}

// Paladin bots with Hand of Protection ready for the kiter, never a side tank holding a burrower: the escort walk
// takes it off its patch and the burrower submerges back to full. The claim can still take its Hand where it stands
std::vector<Player*> ReadyHands(Player* kiter)
{
    std::vector<Player*> hands;
    if (kiter->HasAura(SPELL_FORBEARANCE))
        return hands;

    if (GET_PLAYERBOT_AI(kiter) && HandReady(kiter))
        hands.push_back(kiter);

    for (Player* paladin : ProtectorOrder(kiter))
        if (HandReady(paladin) && !HoldsSideBurrower(paladin))
            hands.push_back(paladin);

    return hands;
}

AnubarakKitePlanner::Rules const& KiteRules()
{
    static AnubarakKitePlanner::Rules const rules = []
    {
        AnubarakKitePlanner::Rules r;
        r.centre = ToPoint(ANUBARAK_ROOM_CENTER);
        r.ring = ANUBARAK_KITE_RING_RADIUS;
        r.patchReach = ANUBARAK_SPIKE_PATCH_REACH;
        r.impaleReach = ANUBARAK_IMPALE_REACH;
        r.slowReach = ANUBARAK_PERMAFROST_SLOW_REACH;
        r.edgeRadius = ANUBARAK_TANK_KEEPOUT;
        r.gatherClearance = ANUBARAK_KITE_GATHER_CLEARANCE;
        r.bystanderClearance = ANUBARAK_KITE_PLAYER_CLEARANCE;
        r.handS = ANUBARAK_HAND_DURATION_S;
        r.handLeadS = ANUBARAK_KITE_HAND_LEAD_S;
        r.ownLeadS = ANUBARAK_KITE_OWN_LEAD_S;
        r.handRange = ANUBARAK_HOP_RANGE;
        return r;
    }();
    return rules;
}

// The plan lands a Hand of Protection that hasn't landed yet
bool HandKite(KitePlan const& plan)
{
    return plan.valid && plan.outcome.handS >= 0.0f && !plan.handLanded && !plan.hand.IsEmpty();
}

float SinceS(uint32 ms) { return static_cast<float>(getMSTimeDiff(ms, getMSTime())) / IN_MILLISECONDS; }

// Seconds from now to the plan's catch, negative with none
float CatchLeftS(KitePlan const& plan)
{
    if (!plan.valid || plan.outcome.catchS < 0.0f)
        return -1.0f;

    return std::max(0.0f, plan.outcome.catchS - SinceS(plan.judgedMs));
}

// The tick on the plan's track nearest the kiter now
int PlanTick(KitePlan const& plan, Player* kiter)
{
    std::vector<AnubarakPursuit::Point> const& track = plan.candidate.track;
    if (track.empty())
        return 0;

    int const elapsed = static_cast<int>(SinceS(plan.judgedMs) / AnubarakPursuit::TICK_S);
    return AnubarakKitePlanner::NearestTick(track, ToPoint(kiter->GetPosition()), elapsed,
                                            ANUBARAK_KITE_TRACK_SLACK_TICKS);
}

// The plan's track from the kiter's spot now, for the chase helpers
struct PlanPath
{
    AnubarakPursuit::Point start;
    std::vector<AnubarakPursuit::Point> const* track = nullptr;
    int from = 0;

    AnubarakPursuit::Point At(float t) const
    {
        int const i = from + static_cast<int>(std::lround(t / AnubarakPursuit::TICK_S));
        return (*track)[static_cast<size_t>(std::clamp(i, 0, static_cast<int>(track->size()) - 1))];
    }
};

PlanPath PathFrom(KitePlan const& plan, Player* kiter)
{
    return {ToPoint(kiter->GetPosition()), &plan.candidate.track, PlanTick(plan, kiter)};
}

// A landed patch the kite may end on, tank patches left out unless tankToo
bool KitePatchAvailable(Player* kiter, AnubarakState const& state, bool tankToo)
{
    bool const heroic = AnubarakHeroic(kiter);
    bool const burrowers = AnyBurrower(kiter, state);
    for (Creature* sphere : ResolveAlive(kiter, state.spheres))
    {
        int8 const rank = KitePatchRank(kiter, state, sphere, heroic, burrowers);
        bool const tank = sphere == LatchedTankPatch(kiter, state, 0) || sphere == LatchedTankPatch(kiter, state, 1);
        if (rank >= 0 && (tankToo || !tank))
            return true;
    }

    return false;
}

// A Hand cast now is back before the second submerge's last 10 s. Its cooldown is 3 to 5 min by Guardian's
// Favor, and a submerge comes every 142 s.
bool HandBackInTime(Player* paladin, AnubarakState const& state, uint32 now)
{
    uint32 const rank = KnownRank(paladin, HAND_OF_PROTECTION_RANKS);
    SpellInfo const* info = rank ? sSpellMgr->GetSpellInfo(rank) : nullptr;
    if (!info)
        return true;

    int32 cooldown = static_cast<int32>(std::max(info->RecoveryTime, info->CategoryRecoveryTime));
    paladin->ApplySpellMod(rank, SPELLMOD_COOLDOWN, cooldown);
    int32 const cycle = static_cast<int32>(ANUBARAK_SURFACED_MS + ANUBARAK_SUBMERGED_MS);
    int32 const until = static_cast<int32>(ANUBARAK_HAND_KEEP_SUBMERGE - state.submerges) * cycle +
                        static_cast<int32>(ANUBARAK_SPIKE_LIFE_MS - ANUBARAK_HAND_SPARE_TAIL_MS) -
                        static_cast<int32>(getMSTimeDiff(state.submergedMs, now));
    return cooldown <= until;
}

// Heroic: every Hand from the second submerge on. In the first, one back in time for the second, or one that
// isn't while another bot paladin's that isn't is still ready, so the last of those waits for the second.
bool HandSpare(Player* paladin, AnubarakState const& state, uint32 now)
{
    if (state.submerges >= ANUBARAK_HAND_KEEP_SUBMERGE || HandBackInTime(paladin, state, now))
        return true;

    bool other = false;
    ForEachGroupMember(paladin, [&](Player* member)
    {
        other = member != paladin && member->IsAlive() && GET_PLAYERBOT_AI(member) && HandReady(member) &&
                !HandBackInTime(member, state, now);
        return other;
    });

    return other;
}

// What the planner sees: the patches by kind, the gather and the players off it, the Hands that can land and
// the kiter's own immunity.
AnubarakKitePlanner::KiteWorld KiteWorldFor(Player* kiter, Creature* spike, AnubarakState const& state,
                                            KitePlan const& plan)
{
    AnubarakKitePlanner::KiteWorld w;
    bool const heroic = AnubarakHeroic(kiter);
    bool const bot = GET_PLAYERBOT_AI(kiter) != nullptr;
    w.heroic = heroic;
    w.burrowers = AnyBurrower(kiter, state);
    w.kiter = ToPoint(kiter->GetPosition());
    w.pace = KiterPace(kiter);
    // A human boosts when it likes, so only a bot's ready boost is the plan's to time
    AnubarakKiteBoost ready;
    if (bot && ReadySpeedBoost(kiter, ready))
    {
        w.readyBoostSpeed = BoostRunSpeed(kiter, ready);
        w.readyBoostS = BoostDurationS(ready);
    }

    w.spike = PursuitSpike(spike);
    w.lifeS = std::max(AnubarakPursuit::TICK_S,
                       static_cast<float>(static_cast<int32>(state.submergedMs + ANUBARAK_SPIKE_LIFE_MS - getMSTime())) /
                           IN_MILLISECONDS);

    Creature* tanks[2] = {LatchedTankPatch(kiter, state, 0), LatchedTankPatch(kiter, state, 1)};
    std::vector<Position> const tankSpots = LatchedTankSpots(kiter, state);
    for (Creature* sphere : ResolveAlive(kiter, state.spheres))
    {
        bool const landed = IsPermafrostPatch(sphere);
        bool const falling = IsFrostSphereFalling(sphere);
        // The spike is on it, so it's already being used up
        if (Dist2d(sphere->GetPosition(), spike->GetPosition()) < 0.5f)
            continue;

        if (!landed && !falling)
        {
            // Normal's kite duty shoots another while none is down
            w.patchBudget += heroic ? 0 : 1;
            continue;
        }

        AnubarakKitePlanner::Patch patch;
        patch.pos = ToPoint(sphere->GetPosition());
        patch.falling = falling;
        patch.tank = landed && (sphere == tanks[0] || sphere == tanks[1]);
        patch.usable = landed && KitePatchRank(kiter, state, sphere, heroic, w.burrowers) >= 0;
        patch.id = sphere->GetGUID().GetRawValue();
        w.patches.push_back(patch);
        if (patch.usable || (falling && !(heroic && NearTankPatch(sphere->GetPosition(), tankSpots))))
            ++w.patchBudget;
    }

    Position gather;
    w.gather = GatherCentre(kiter, state, gather);
    if (w.gather)
        w.gatherPos = ToPoint(gather);

    ForEachGroupMember(kiter, [&](Player* member)
    {
        if (member != kiter && member->IsAlive() && member->GetPositionZ() < ANUBARAK_PIT_CEILING_Z &&
            (!w.gather || Dist2d(member->GetPosition(), gather) > ANUBARAK_KITE_GATHER_CLEARANCE))
            w.bystanders.push_back(ToPoint(member->GetPosition()));

        return false;
    });

    w.slowFactor = kiter->HasAura(ai::paladin::SPELL_HAND_OF_FREEDOM) ? 1.0f
                   : heroic                                           ? ANUBARAK_PERMAFROST_HEROIC_SLOW
                                                                      : ANUBARAK_PERMAFROST_NORMAL_SLOW;
    w.catchFromS = BaseTestGraceS(w.spike, w.pace, kiter, spike);
    w.handLeftS = HandLeftS(kiter);

    // Heroic: a bot warlock's circle wherever it is, so a run can come back into range of it
    GameObject* circle = bot && heroic ? WarlockCircle(kiter) : nullptr;
    if (circle && ReadyInstant(kiter, SPELL_CIRCLE_TELEPORT))
    {
        w.teleport = true;
        w.circle = ToPoint(circle->GetPosition());
    }

    // The held plan's paladin first, else the standby escort, so the escort doesn't swap
    ObjectGuid const first = !plan.hand.IsEmpty()                     ? plan.hand
                             : state.standbyKiter == kiter->GetGUID() ? state.standby
                                                                      : ObjectGuid::Empty;
    std::vector<Player*> hands = ReadyHands(kiter);
    std::stable_partition(hands.begin(), hands.end(),
                          [&first](Player* paladin) { return paladin->GetGUID() == first; });
    uint32 const now = getMSTime();
    for (Player* paladin : hands)
        if (paladin == kiter || (paladin->IsAlive() && !paladin->HasAura(SPELL_MARK)))
        {
            bool const spare = !heroic || HandSpare(paladin, state, now);
            w.hands.push_back(
                {paladin->GetGUID().GetRawValue(), ToPoint(paladin->GetPosition()), paladin == kiter, spare});
            w.spareHands += spare ? 1 : 0;
        }

    std::stable_partition(w.hands.begin(), w.hands.end(),
                          [](AnubarakKitePlanner::HandOffer const& offer) { return offer.spare; });
    // Heroic's four kite patches go two a submerge, so the first two spend one each unless nothing else ends
    // a kite, and the third gets theirs
    w.reservePatch = heroic && state.submerges < ANUBARAK_LAST_KITE_SUBMERGE && state.kitePatchesSpent >= 1;
    w.own = bot && OwnImmunitySpell(kiter) != 0;
    w.ownAfterHand = kiter->getClass() != CLASS_PALADIN;
    return w;
}

// A human's run, read off how far it moved since the last plan, else the way it faces while it moves
AnubarakPursuit::Point HumanVelocity(Player* kiter, KitePlan& plan, uint32 now)
{
    AnubarakPursuit::Point velocity;
    uint32 const ms = plan.lastPosMs ? getMSTimeDiff(plan.lastPosMs, now) : 0;
    if (ms >= ANUBARAK_KITE_MOTION_MIN_MS && ms <= ANUBARAK_KITE_MOTION_MAX_MS)
    {
        Vec2 const moved = Delta(plan.lastPos, kiter->GetPosition());
        float const seconds = static_cast<float>(ms) / IN_MILLISECONDS;
        velocity = {moved.x / seconds, moved.y / seconds};
    }
    else if (kiter->isMoving())
    {
        float const speed = KiterRunSpeed(kiter);
        velocity = {std::cos(kiter->GetOrientation()) * speed, std::sin(kiter->GetOrientation()) * speed};
    }

    plan.lastPos = kiter->GetPosition();
    plan.lastPosMs = now;
    return velocity;
}

// Judged at most every ANUBARAK_KITE_PLAN_JUDGE_MS. A bot's plan is the planner's pick, held unless beaten. A
// human's is its current motion, judged for the catch the Hand and the escort serve
KitePlan& PlanKite(Player* kiter, Creature* spike, AnubarakState& state)
{
    KitePlan& plan = state.kitePlan;
    uint32 const now = getMSTime();
    bool const same = plan.valid && plan.kiter == kiter->GetGUID();
    if (same && getMSTimeDiff(plan.judgedMs, now) < ANUBARAK_KITE_PLAN_JUDGE_MS)
        return plan;

    if (!same)
    {
        plan = KitePlan();
        plan.kiter = kiter->GetGUID();
    }

    plan.handLanded = plan.handLanded || (HandKite(plan) && UnderHandOfProtection(kiter));
    AnubarakKitePlanner::KiteWorld const w = KiteWorldFor(kiter, spike, state, plan);
    bool const bot = GET_PLAYERBOT_AI(kiter) != nullptr;
    AnubarakKitePlanner::Plan next;
    if (!bot)
        next = AnubarakKitePlanner::PlanMotion(w, HumanVelocity(kiter, plan, now), KiteRules());
    else
    {
        bool const held = same && plan.candidate.track.size() >= 2;
        next = AnubarakKitePlanner::PlanKite(w, held ? &plan.candidate : nullptr, held ? SinceS(plan.pickedMs) : 0.0f,
                                             held ? SinceS(plan.judgedMs) : 0.0f, KiteRules());
    }

    if (!next.valid)
        return plan;

    bool const picked = !same || !next.kept;
    plan.valid = true;
    plan.judgedMs = now;
    if (picked)
        plan.pickedMs = now;

    plan.candidate = std::move(next.candidate);
    plan.outcome = next.outcome;
    if (!plan.handLanded)
        plan.hand = plan.outcome.handId ? ObjectGuid(plan.outcome.handId) : ObjectGuid::Empty;

    int const patch = plan.candidate.patch;
    plan.patchGuid = patch >= 0 ? ObjectGuid(w.patches[static_cast<size_t>(patch)].id) : ObjectGuid::Empty;
    if (patch >= 0)
        plan.patchPos = Position(w.patches[static_cast<size_t>(patch)].pos.x, w.patches[static_cast<size_t>(patch)].pos.y,
                                 kiter->GetPositionZ());

    return plan;
}

// Heroic only. The patch stage holds once the walk onto the plan's patch began. A speed boost waits for the
// plan's boost tick or a due catch, and never goes on a spike no faster than the kiter
KiteStageRead const& ReadKiteStage(Player* kiter, Creature* spike, AnubarakState& state)
{
    KiteStageRead& read = state.kiteStage;
    uint32 const now = getMSTime();
    if (read.valid && read.ms == now && read.kiter == kiter->GetGUID())
        return read;

    read = KiteStageRead();
    read.valid = true;
    read.ms = now;
    read.kiter = kiter->GetGUID();

    bool const bot = GET_PLAYERBOT_AI(kiter) != nullptr;
    KitePlan const& plan = PlanKite(kiter, spike, state);
    float const dist = Dist2d(spike->GetPosition(), kiter->GetPosition());
    float const handLeft = HandLeftS(kiter);
    AnubarakKiteBoost boost;
    bool const hasBoost = bot && handLeft <= ANUBARAK_KITE_BOOST_HAND_MAX_S &&
                          NextKiteBoost(kiter, spike, state, LaneObstacles(kiter, state), boost) &&
                          (handLeft <= 0.0f || handLeft <= BoostHandLeadS(kiter, spike, boost));

    bool const patch = plan.candidate.patch >= 0;
    int const tick = PlanTick(plan, kiter);
    bool const walking = patch && tick >= plan.candidate.turnTick;
    if (patch && (walking || state.kitePatchKiter == kiter->GetGUID()))
        read.stage = AnubarakKiteStage::Patch;
    else if (AnubarakKiterImmune(kiter))
        read.stage = AnubarakKiteStage::Immune;
    else
        read.stage = AnubarakKiteStage::Run;

    bool const faster = spike->GetSpeed(MOVE_RUN) > KiterRunSpeed(kiter);
    bool const speedBoost = boost.nitro || IsSpeedBoostSpell(boost.spellId);
    bool fire = (faster && dist <= ANUBARAK_KITE_BOOST_RANGE) || dist <= ANUBARAK_KITE_BOOST_CLOSE;
    if (speedBoost && handLeft <= 0.0f)
        fire = faster && ((plan.candidate.boostTick >= 0 && tick >= plan.candidate.boostTick) ||
                          (plan.outcome.caught && KiteCatchDue(kiter, spike)));
    else if (speedBoost)
        fire = fire && faster;

    // The plan's teleport goes at its tick, ahead of any boost due then
    bool const teleport = bot && plan.candidate.teleportTick >= 0 && tick >= plan.candidate.teleportTick &&
                          TeleportCircle(kiter);
    if (teleport)
    {
        read.boost = AnubarakKiteBoost();
        read.boost.spellId = SPELL_CIRCLE_TELEPORT;
    }
    else if (hasBoost && fire)
        read.boost = boost;

    if ((teleport || (hasBoost && fire)) && read.stage == AnubarakKiteStage::Run)
        read.stage = AnubarakKiteStage::Boost;

    if (walking)
        state.kitePatchKiter = kiter->GetGUID();

    return read;
}

// Prefers a sphere whose patch would put the kiter ahead of the spike at its stand
Creature* KiteSphere(std::vector<Creature*> const& flying, Player* target, Creature* spike)
{
    Creature* best = nullptr;
    bool bestAhead = false;
    float bestDist = 0.0f;
    for (Creature* sphere : flying)
    {
        Vec2 away = Delta(spike->GetPosition(), sphere->GetPosition());
        float const length = Length(away);
        bool ahead = false;
        if (length > 0.5f)
        {
            away = {away.x / length, away.y / length};
            ahead = SpikeBehind(target, spike, Offset(sphere->GetPosition(), away, ANUBARAK_KITE_STAND_OFFSET));
        }

        float const dist = target->GetExactDist2d(sphere->GetPositionX(), sphere->GetPositionY());
        if (!best || (ahead && !bestAhead) || (ahead == bestAhead && dist < bestDist))
        {
            best = sphere;
            bestAhead = ahead;
            bestDist = dist;
        }
    }

    return best;
}

bool TankPairSphere(AnubarakState const& state, Creature* sphere)
{
    auto const home = state.sphereHomes.find(sphere->GetGUID());
    return home != state.sphereHomes.end() &&
           (home->second == ANUBARAK_TANK_HOME_NORTH || home->second == ANUBARAK_TANK_HOME_SOUTH);
}

// Heroic's first pair comes off the east homes while one of them still flies: any other sphere pulled in
// for a side is a kite sphere gone
std::vector<Creature*> SideCandidates(AnubarakState const& state, std::vector<Creature*> const& flying, bool heroic)
{
    if (!heroic || state.firstPairDone)
        return flying;

    std::vector<Creature*> pair;
    for (Creature* sphere : flying)
        if (TankPairSphere(state, sphere))
            pair.push_back(sphere);

    return pair.empty() ? flying : pair;
}

float SurfacedLeftS(AnubarakState const& state, uint32 now)
{
    return static_cast<float>(static_cast<int32>(state.surfacedMs + ANUBARAK_SURFACED_MS - now)) / IN_MILLISECONDS;
}

// A stray patch heroic's kite may end on: off the tank patches by the spike's restart reach
bool UsableKitePatch(Creature* sphere, AnubarakState const& state, std::vector<Position> const& tankSpots, bool heroic)
{
    ObjectGuid const guid = sphere->GetGUID();
    return IsPermafrostPatch(sphere) && guid != state.patch0 && guid != state.patch1 &&
           !(heroic && NearTankPatch(sphere->GetPosition(), tankSpots));
}

// Heroic opens a submerge's kite budget 30 s before he goes down, or as he does if no bot read it then.
// Kite patches already down count toward the two.
void OpenKiteBudget(AnubarakState& state, uint32 flying, uint32 kitePatches, uint32 now)
{
    if (state.kiteBudgetSet)
        return;

    bool const due = state.phaseValue == AnubarakPhase::Submerged ||
                     (state.phaseValue == AnubarakPhase::Surface && state.firstPairDone &&
                      SurfacedLeftS(state, now) <= ANUBARAK_KITE_PRESHOT_LEFT_S);
    if (!due)
        return;

    state.kiteBudgetSet = true;
    state.kiteBudgetFlying = flying;
    state.kiteLeftover = std::min(kitePatches, ANUBARAK_KITE_SPHERES_PER_SUBMERGE);
}

uint32 KiteSpheresDowned(AnubarakState const& state, uint32 flying)
{
    uint32 const shot = state.kiteBudgetFlying > flying ? state.kiteBudgetFlying - flying : 0;
    return shot + state.kiteLeftover;
}

Creature* NearestCenterSphere(std::vector<Creature*> const& spheres)
{
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* sphere : spheres)
    {
        float const dist = Dist2d(sphere->GetPosition(), ANUBARAK_ROOM_CENTER);
        if (!best || dist < bestDist)
        {
            best = sphere;
            bestDist = dist;
        }
    }

    return best;
}

// A preferred sphere first, the held one kept unless it isn't preferred and another is
template <typename Qualifies, typename Prefers>
Creature* HeldOrPick(std::vector<Creature*> const& flying, std::vector<SphereDuty> const& previous,
                     Qualifies const& qualifies, Prefers const& prefers, Player* target, Creature* spike)
{
    // Spheres drift, so the pick swaps back and forth and its shooters with it
    Creature* held = nullptr;
    for (SphereDuty const& duty : previous)
        if (std::strcmp(duty.label, "stock") == 0)
            if (Creature* sphere = FindSphere(flying, duty.sphere); !held && sphere && qualifies(sphere))
                held = sphere;

    if (held && prefers(held))
        return held;

    std::vector<Creature*> open;
    std::vector<Creature*> preferred;
    for (Creature* sphere : flying)
    {
        if (!qualifies(sphere))
            continue;

        open.push_back(sphere);
        if (prefers(sphere))
            preferred.push_back(sphere);
    }

    if (!preferred.empty())
        open = std::move(preferred);
    else if (held)
        return held;

    if (open.empty())
        return nullptr;

    return target && spike ? KiteSphere(open, target, spike) : NearestCenterSphere(open);
}

// After a fail the spike walks on about 14 yd before its next mark, so a kite patch this far off every tank
// patch can't spend one on the restart
bool ClearOfTankPatches(Position const& at, std::vector<Position> const& tanks)
{
    return std::none_of(tanks.begin(), tanks.end(), [&at](Position const& tank)
                        { return Dist2d(at, tank) < ANUBARAK_KITE_SPHERE_TANK_CLEARANCE; });
}

// Heroic's kite spheres both go before the submerge: shot one at a time, kiters spend tank patches while the
// next is shot. Clear of the spot by the clearance it was found with, or the spot is searched again mid-drag
template <typename Taken>
Creature* PreSubmergeKiteSphere(Player* bot, AnubarakState const& state, std::vector<Creature*> const& flying,
                                std::vector<SphereDuty> const& previous, Taken const& taken)
{
    Creature* boss = GetAnubarak(bot);
    if (!state.submergeSpotValid && !boss)
        return nullptr;

    Position const spot = state.submergeSpotValid ? state.submergeSpot : boss->GetPosition();
    float const clearance = std::max(state.submergeSpotValid ? state.submergeSpotClearance : 0.0f,
                                     ANUBARAK_STOCK_SPIKE_CLEARANCE);
    std::vector<Position> const tanks = LatchedTankSpots(bot, state);
    auto const qualifies = [&](Creature* sphere)
    {
        Position const& at = sphere->GetPosition();
        return !taken(sphere) && Dist2d(at, ANUBARAK_ROOM_CENTER) <= ANUBARAK_KITE_SPHERE_FLOOR &&
               Dist2d(at, spot) >= clearance &&
               (!state.raidPointValid ||
                SegmentDistance2d(spot, state.raidPoint, at) > ANUBARAK_SPIKE_ROUTE_CLEARANCE) &&
               !NearTankPatch(at, tanks);
    };

    auto const prefers = [&tanks](Creature* sphere) { return ClearOfTankPatches(sphere->GetPosition(), tanks); };
    return HeldOrPick(flying, previous, qualifies, prefers, nullptr, nullptr);
}

// Heroic phase 2's kite sphere while fewer than two patches are down and the submerge's two aren't shot
// yet. A flying sphere hovers past the spike's test, so none waits on the floor to be spent.
template <typename Taken>
Creature* HeroicKiteSphere(Player* bot, AnubarakState const& state, std::vector<Creature*> const& flying,
                           std::vector<SphereDuty> const& previous, Creature* spike, Player* target,
                           Taken const& taken)
{
    bool const spikeUp = spike && spike->IsAlive();
    if (!spikeUp && !state.submergePosValid)
        return nullptr;

    Position const from = spikeUp ? spike->GetPosition() : state.submergePos;
    std::vector<Position> const tanks = LatchedTankSpots(bot, state);
    auto const qualifies = [&](Creature* sphere)
    {
        Position const& at = sphere->GetPosition();
        return !taken(sphere) && (state.firstPairDone || !TankPairSphere(state, sphere)) &&
               Dist2d(at, ANUBARAK_ROOM_CENTER) <= ANUBARAK_KITE_SPHERE_FLOOR &&
               Dist2d(at, from) > ANUBARAK_STOCK_SPIKE_CLEARANCE &&
               (!spikeUp || !target ||
                SegmentDistance2d(spike->GetPosition(), target->GetPosition(), at) > ANUBARAK_SPIKE_LANE_CLEARANCE) &&
               !NearTankPatch(at, tanks);
    };

    auto const prefers = [&tanks](Creature* sphere) { return ClearOfTankPatches(sphere->GetPosition(), tanks); };
    return HeldOrPick(flying, previous, qualifies, prefers, spikeUp ? target : nullptr, spikeUp ? spike : nullptr);
}

struct SphereShooter
{
    Player* player;
    // On its dodge spot: it shoots only what it reaches from there
    bool parked;
};

template <typename Taken>
Creature* StockSphere(std::vector<Creature*> const& flying, std::vector<SphereDuty> const& previous,
                      Position const& spikeFrom, Creature* spike, Player* target, Taken const& taken)
{
    auto const qualifies = [&](Creature* sphere)
    {
        if (taken(sphere) || Dist2d(sphere->GetPosition(), spikeFrom) <= ANUBARAK_STOCK_SPIKE_CLEARANCE)
            return false;

        return !spike || !target ||
               SegmentDistance2d(spike->GetPosition(), target->GetPosition(), sphere->GetPosition()) >
                   ANUBARAK_SPIKE_LANE_CLEARANCE;
    };

    // Spheres drift, so the nearest one swaps back and forth and its shooters with it
    for (SphereDuty const& duty : previous)
        if (std::strcmp(duty.label, "stock") == 0)
            if (Creature* held = FindSphere(flying, duty.sphere))
                if (qualifies(held))
                    return held;

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* sphere : flying)
    {
        if (!qualifies(sphere))
            continue;

        float const dist = Dist2d(sphere->GetPosition(), ANUBARAK_ROOM_CENTER);
        if (!best || dist < bestDist)
        {
            best = sphere;
            bestDist = dist;
        }
    }

    return best;
}

char const* const SIDE_LABELS[2] = {"0", "1"};
char const* const KITE_LABEL = "kite";

// A living bot side tank, side 0 first, else whichever side is held
ObjectGuid FirstBotSideTank(Player* bot, ObjectGuid const sides[2])
{
    for (uint8 side = 0; side < 2; ++side)
        if (Player* tank = sides[side].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, sides[side]))
            if (tank->IsAlive() && GET_PLAYERBOT_AI(tank))
                return sides[side];

    return sides[0].IsEmpty() ? sides[1] : sides[0];
}

// Who holds him: the main tank while alive. Else the side tank he's on, so a human who taunted him keeps
// him and no bot taunts him back, else the first bot side tank.
ObjectGuid PickupGuid(Player* bot, AnubarakState const& state)
{
    Player* main = state.mainTank.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state.mainTank);
    if (main && main->IsAlive())
        return state.mainTank;

    Creature* boss = GetAnubarak(bot);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    if (victim && (victim->GetGUID() == state.sideTanks[0] || victim->GetGUID() == state.sideTanks[1]))
        return victim->GetGUID();

    return FirstBotSideTank(bot, state.sideTanks);
}

bool GripperReady(Player* member, ObjectGuid const& pickup, std::vector<ObjectGuid> const& busy)
{
    return member && member->IsAlive() && GET_PLAYERBOT_AI(member) && member->getClass() == CLASS_DEATH_KNIGHT &&
           member->HasSpell(SPELL_DEATH_GRIP) && !member->HasAura(SPELL_MARK) && InAnubarakPit(member) &&
           member->GetGUID() != pickup && std::find(busy.begin(), busy.end(), member->GetGUID()) == busy.end() &&
           member->GetSpellCooldownDelay(SPELL_DEATH_GRIP) <= ANUBARAK_GRIP_COOLDOWN_WAIT_MS;
}

// The side's own DK tank, else the lowest-guid DK bot with no tank duty, else the other side's DK tank before
// any burrower is up. A far grip walks the DK well off its side, so it takes the free DK first
Player* PickGripper(Player* bot, AnubarakState const& state, uint8 side, std::vector<ObjectGuid> const& busy,
                    bool farGrip = false)
{
    ObjectGuid const pickup = PickupGuid(bot, state);
    auto const eligible = [&](Player* member) { return GripperReady(member, pickup, busy); };

    Player* own = state.sideTanks[side].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state.sideTanks[side]);
    if (eligible(own) && !farGrip)
        return own;

    Player* best = nullptr;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (eligible(member) && member->GetGUID() != state.sideTanks[0] && member->GetGUID() != state.sideTanks[1] &&
            (!best || member->GetGUID() < best->GetGUID()))
            best = member;

        return false;
    });

    if (best)
        return best;

    if (farGrip)
        return eligible(own) ? own : nullptr;

    uint8 const otherSide = 1 - side;
    Player* other =
        state.sideTanks[otherSide].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state.sideTanks[otherSide]);
    return eligible(other) && !AnyBurrower(bot, state) ? other : nullptr;
}

// Nearest flying sphere within reach of the spot, the last pick while it still is
template <typename Taken>
Creature* GripSphere(std::vector<GripDuty> const& previous, std::vector<Creature*> const& flying, Position const& spot,
                     uint8 side, float within, Taken const& taken)
{
    auto const reach = [&](Creature* sphere)
    {
        return !taken(sphere) &&
               sphere->GetExactDist(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ()) <= within;
    };

    for (GripDuty const& duty : previous)
        if (duty.side == side)
            if (Creature* held = FindSphere(flying, duty.sphere))
                if (reach(held))
                    return held;

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* sphere : flying)
    {
        if (!reach(sphere))
            continue;

        float const dist = sphere->GetExactDist(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
        if (!best || dist < bestDist)
        {
            best = sphere;
            bestDist = dist;
        }
    }

    return best;
}

// The spot, else up to ANUBARAK_GRIP_STEP_OUT toward a sphere out of reach of it, never nearer the other
// side's spot than the spot is, so the two patches keep their gap
Position GripStand(Player* bot, Position const& spot, Position const& otherSpot, Creature* sphere)
{
    if (sphere->GetExactDist(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ()) <= ANUBARAK_GRIP_RANGE)
        return spot;

    Vec2 toward = Delta(spot, sphere->GetPosition());
    float const length = Length(toward);
    if (length < 1.0f)
        return spot;

    toward = {toward.x / length, toward.y / length};
    Vec2 away = Delta(otherSpot, spot);
    float const awayLength = Length(away);
    if (awayLength >= 1.0f)
    {
        away = {away.x / awayLength, away.y / awayLength};
        float const along = toward.x * away.x + toward.y * away.y;
        if (along < 0.0f)
        {
            toward = {toward.x - along * away.x, toward.y - along * away.y};
            float const sideways = Length(toward);
            if (sideways < 0.1f)
                return spot;

            toward = {toward.x / sideways, toward.y / sideways};
        }
    }

    // Only as far as the sphere needs: it flies about 14 yd up
    float const height = sphere->GetPositionZ() - spot.GetPositionZ();
    float const flat = std::sqrt(std::max(ANUBARAK_GRIP_RANGE * ANUBARAK_GRIP_RANGE - height * height, 0.0f));
    float const step = std::clamp(length - flat + ANUBARAK_GRIP_REACH_MARGIN, 0.0f, ANUBARAK_GRIP_STEP_OUT);
    return ValidateFloorPoint(bot, Offset(spot, toward, step));
}

constexpr float ANUBARAK_FAR_GRIP_TURNS_DEG[] = {0.0f, 20.0f, -20.0f, 40.0f, -40.0f};

// For a sphere hovering past the step-out: on the floor just inside Death Grip's reach of it, on its side
// nearest the spot, turned round it until on the floor and off the spike and its lane
bool FarGripStand(Player* dk, Creature* sphere, Position const& spot, Position& out)
{
    Vec2 toward = Delta(sphere->GetPosition(), spot);
    float const length = Length(toward);
    float const height = sphere->GetPositionZ() - spot.GetPositionZ();
    float const flat =
        std::sqrt(std::max(ANUBARAK_GRIP_RANGE * ANUBARAK_GRIP_RANGE - height * height, 0.0f)) - ANUBARAK_GRIP_REACH_MARGIN;
    if (length < 1.0f || flat < 1.0f)
        return false;

    toward = {toward.x / length, toward.y / length};
    Creature* spike = GetPursuingSpike(dk);
    Player* kiter = spike ? GetSpikeTarget(dk) : nullptr;
    Position const base(sphere->GetPositionX(), sphere->GetPositionY(), spot.GetPositionZ());
    for (float degrees : ANUBARAK_FAR_GRIP_TURNS_DEG)
    {
        Position const raw = Offset(base, Rotate(toward, degrees * ANUBARAK_DEG_TO_RAD), flat);
        if (spike && (Dist2d(raw, spike->GetPosition()) < ANUBARAK_SPIKE_DANGER_RADIUS_FAST ||
                      (kiter && SegmentDistance2d(spike->GetPosition(), kiter->GetPosition(), raw) <
                                    ANUBARAK_SPIKE_LANE_CLEARANCE_FAST)))
            continue;

        Position valid;
        if (OnFloor(dk, raw, valid) && sphere->GetExactDist(valid.GetPositionX(), valid.GetPositionY(),
                                                            valid.GetPositionZ()) <= ANUBARAK_GRIP_RANGE)
        {
            out = valid;
            return true;
        }
    }

    return false;
}

// Pulled down by the grip: a flying sphere hovers about 13 yd up
bool GrippedDown(Creature* sphere, GripCast const& cast)
{
    return sphere->GetPositionZ() - cast.from.GetPositionZ() <= ANUBARAK_GRIP_LANDED_HEIGHT;
}

// The grip lands within 3 yd of the DK, and a gripped patch under 20 yd off the other side's never latches
constexpr float ANUBARAK_GRIP_CLEAR = ANUBARAK_TANK_PATCH_GAP + ANUBARAK_GRIP_LANDED;

// Out to the clearance plus the arrive slack off the other side's patch or the sphere about to make it,
// while the sphere stays in reach of it
Position GripStandClear(Player* bot, Position const& stand, Creature* otherPatch, Creature* sphere)
{
    float const want = ANUBARAK_GRIP_CLEAR + ANUBARAK_TANK_PATCH_ARRIVE;
    float const dist = otherPatch ? Dist2d(stand, otherPatch->GetPosition()) : want;
    if (dist >= want || dist < 1.0f)
        return stand;

    Vec2 const away = Delta(otherPatch->GetPosition(), stand);
    Position const moved = ValidateFloorPoint(bot, Offset(stand, {away.x / dist, away.y / dist}, want - dist));
    return sphere->GetExactDist(moved.GetPositionX(), moved.GetPositionY(), moved.GetPositionZ()) <= ANUBARAK_GRIP_RANGE
               ? moved
               : stand;
}

// A grip landing this far off the spot gets another link, posted Death Grip's reach less the margin
// from the landing: the landed sphere sits low. It rises back above reach about 4 s after the landing.
constexpr float ANUBARAK_GRIP_RELAY_FROM = ANUBARAK_SIDE_SHOOT_FAR;
constexpr float ANUBARAK_GRIP_RELAY_REACH = ANUBARAK_GRIP_RANGE - ANUBARAK_GRIP_REACH_MARGIN;
constexpr uint32 ANUBARAK_GRIP_RELAY_WAIT_MS = 3000;

// A grip that takes jumps the sphere about 20 yd inside 120 ms. Death Grip 49576 is magic-class, so it
// can miss the hit roll and leave the sphere where it was
constexpr uint32 ANUBARAK_GRIP_PULL_MS = 600;
constexpr float ANUBARAK_GRIP_PULL_GAIN = 3.0f;

bool GripTookNoPull(Creature* sphere, GripCast const& cast, uint32 now)
{
    return getMSTimeDiff(cast.castMs, now) >= ANUBARAK_GRIP_PULL_MS &&
           sphere->GetExactDist(cast.from.GetPositionX(), cast.from.GetPositionY(), cast.from.GetPositionZ()) >
               cast.startDist - ANUBARAK_GRIP_PULL_GAIN;
}

// The side's own DK tank first, its burrowers walking along with it, then the lowest-guid DK bot with no
// tank duty. Never the pickup tank or the other side's tank.
Player* PickRelayLink(Player* bot, AnubarakState const& state, uint8 side, std::vector<ObjectGuid> const& busy)
{
    ObjectGuid const pickup = PickupGuid(bot, state);
    Player* own = state.sideTanks[side].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state.sideTanks[side]);
    if (GripperReady(own, pickup, busy))
        return own;

    Player* best = nullptr;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (GripperReady(member, pickup, busy) && member->GetGUID() != state.sideTanks[0] &&
            member->GetGUID() != state.sideTanks[1] && (!best || member->GetGUID() < best->GetGUID()))
            best = member;

        return false;
    });

    return best;
}

// The next link for a grip bringing the sphere down at from, on the first RelayPosts point that's on the
// floor, out of every patch's slow and off the spike and its lane. None closes the relay.
void PlanRelay(Player* bot, AnubarakState const& state, uint8 side, Position const& from, Position const& spot,
               std::vector<ObjectGuid> const& busy, GripRelay& relay)
{
    relay.dk.Clear();
    relay.from = from;
    relay.closed = true;
    Player* dk = PickRelayLink(bot, state, side, busy);
    if (!dk)
        return;

    std::vector<Position> const patches = PatchPositions(bot, state);
    Creature* spike = GetPursuingSpike(dk);
    Player* kiter = spike ? GetSpikeTarget(dk) : nullptr;
    for (AnubarakPursuit::Point const& point :
         AnubarakPursuit::RelayPosts(ToPoint(from), ToPoint(spot), ANUBARAK_GRIP_RELAY_REACH))
    {
        Position const raw(point.x, point.y, spot.GetPositionZ());
        if (std::any_of(patches.begin(), patches.end(),
                        [&raw](Position const& patch) { return Dist2d(raw, patch) < ANUBARAK_TANK_KEEPOUT; }))
            continue;

        if (spike && (Dist2d(raw, spike->GetPosition()) < ANUBARAK_SPIKE_DANGER_RADIUS_FAST ||
                      (kiter && SegmentDistance2d(spike->GetPosition(), kiter->GetPosition(), raw) <
                                    ANUBARAK_SPIKE_LANE_CLEARANCE_FAST)))
            continue;

        Position valid;
        if (OnFloor(dk, raw, valid))
        {
            relay.dk = dk->GetGUID();
            relay.post = valid;
            relay.closed = false;
            return;
        }
    }
}

// True while a grip has a sphere for the side, kept from the shooters until it's down. A landing under
// ANUBARAK_TANK_PATCH_GAP off the other side's patch never latches, so that grip counts as failed
template <typename Taken>
bool ServeSideByGrip(Player* bot, AnubarakState& state, uint8 side, Position const& spot, Position const& otherSpot,
                     Creature* otherPatch, std::vector<Creature*> const& spheres, std::vector<Creature*> const& flying,
                     std::vector<GripDuty> const& previousGrips, Taken const& taken,
                     std::vector<std::pair<char const*, Creature*>>& needs, std::vector<Creature*>& reserved,
                     uint32 now)
{
    GripCast& cast = state.grips[side];
    GripRelay& relay = state.relays[side];
    std::vector<ObjectGuid> busy;
    for (GripDuty const& duty : state.gripDuties)
        busy.push_back(duty.dk);

    if (cast.valid)
    {
        Creature* sphere = FindSphere(spheres, cast.sphere);

        // A relay link pulling nothing leaves the sphere low where the grip before it landed it, so that
        // landing counts again: the next link grips from there, or the side kills it there
        if (sphere && cast.relayLink && !cast.landed && GripTookNoPull(sphere, cast, now))
        {
            cast.dk = cast.prevDk;
            cast.from = cast.prevFrom;
            cast.landed = true;
            cast.relayLink = false;
            relay = GripRelay();
        }

        Player* dk = ObjectAccessor::GetPlayer(*bot, cast.dk);
        if (IsFrostSphereFlying(sphere) && dk && dk->IsAlive())
        {
            Position const other = otherPatch ? otherPatch->GetPosition() : otherSpot;
            Position const at = sphere->GetPosition();
            if (GrippedDown(sphere, cast) &&
                (Dist2d(at, cast.from) <= ANUBARAK_GRIP_LANDED ||
                 (Dist2d(at, spot) <= ANUBARAK_SIDE_SHOOT_FAR && Dist2d(at, other) >= ANUBARAK_TANK_PATCH_GAP)))
                cast.landed = true;

            // A first grip pulling nothing goes back to picking a DK at once, not after the judge window
            bool const live = cast.landed || (getMSTimeDiff(cast.castMs, now) < ANUBARAK_GRIP_JUDGE_MS &&
                                              !GripTookNoPull(sphere, cast, now));
            if (live && !relay.closed && Dist2d(cast.from, spot) > ANUBARAK_GRIP_RELAY_FROM)
            {
                if (relay.dk.IsEmpty() || Dist2d(relay.from, cast.from) > ANUBARAK_GRIP_LANDED)
                {
                    std::vector<ObjectGuid> others = busy;
                    others.push_back(cast.dk);
                    PlanRelay(bot, state, side, cast.from, spot, others, relay);
                }

                if (cast.landed && !relay.landedMs)
                    relay.landedMs = now;

                Player* next = relay.dk.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, relay.dk);
                if (next && GripperReady(next, PickupGuid(bot, state), busy) &&
                    (!relay.landedMs || getMSTimeDiff(relay.landedMs, now) < ANUBARAK_GRIP_RELAY_WAIT_MS))
                {
                    state.gripDuties.push_back({cast.dk, sphere->GetGUID(), side, cast.from});
                    state.gripDuties.push_back({relay.dk, sphere->GetGUID(), side, relay.post, true});
                    reserved.push_back(sphere);
                    return true;
                }

                relay.dk.Clear();
                relay.closed = true;
            }

            if (live && !(cast.landed && otherPatch && Dist2d(at, other) < ANUBARAK_TANK_PATCH_GAP))
            {
                state.gripDuties.push_back({cast.dk, sphere->GetGUID(), side, cast.from});
                if (cast.landed)
                {
                    state.grippedFor[sphere->GetGUID()] = side;
                    if (Dist2d(cast.from, spot) > ANUBARAK_GRIP_STEP_OUT + ANUBARAK_TANK_PATCH_ARRIVE)
                        state.farGripped.insert(sphere->GetGUID());

                    needs.emplace_back(SIDE_LABELS[side], sphere);
                }
                else
                    reserved.push_back(sphere);

                return true;
            }

            state.grippedFor.erase(sphere->GetGUID());
            state.farGripped.erase(sphere->GetGUID());
        }

        cast = GripCast();
    }

    relay = GripRelay();
    Player* dk = PickGripper(bot, state, side, busy);
    if (!dk)
        return false;

    auto const blocked = [&](Creature* sphere)
    {
        return taken(sphere) || Dist2d(sphere->GetPosition(), otherSpot) <= ANUBARAK_SIDE_SHOOT_NEAR ||
               std::any_of(previousGrips.begin(), previousGrips.end(), [&](GripDuty const& duty)
                           { return duty.side != side && duty.sphere == sphere->GetGUID(); });
    };

    Position stand = spot;
    Creature* sphere = GripSphere(previousGrips, flying, spot, side, ANUBARAK_GRIP_RANGE, blocked);
    if (!sphere)
    {
        sphere = GripSphere(previousGrips, flying, spot, side, ANUBARAK_GRIP_RANGE + ANUBARAK_GRIP_STEP_OUT, blocked);
        if (sphere)
            stand = GripStand(bot, spot, otherSpot, sphere);
    }

    if (sphere)
        stand = GripStandClear(bot, stand, otherPatch, sphere);

    // Every sphere left hovers past the step-out, where a shot one lands too far off to serve the side:
    // the DK walks out to the nearest and its patch becomes the side's
    if (!sphere)
    {
        Creature* distant = GripSphere(previousGrips, flying, spot, side, ANUBARAK_ROOM_RADIUS, blocked);
        Player* farDk = distant ? PickGripper(bot, state, side, busy, true) : nullptr;
        Position farStand;
        if (farDk && FarGripStand(farDk, distant, spot, farStand))
        {
            dk = farDk;
            sphere = distant;
            stand = farStand;
        }
    }

    state.gripDuties.push_back({dk->GetGUID(), sphere ? sphere->GetGUID() : ObjectGuid::Empty, side, stand});
    if (!sphere)
        return false;

    // Posted while the first link walks out, so it's in reach when the sphere comes down
    if (Dist2d(stand, spot) > ANUBARAK_GRIP_RELAY_FROM)
    {
        busy.push_back(dk->GetGUID());
        PlanRelay(bot, state, side, stand, spot, busy, relay);
        if (!relay.dk.IsEmpty() && ObjectAccessor::GetPlayer(*bot, relay.dk))
            state.gripDuties.push_back({relay.dk, sphere->GetGUID(), side, relay.post, true});

        // None now, the cast looks again from where it lands
        relay.closed = false;
    }

    reserved.push_back(sphere);
    return true;
}

// Shooting a tank patch: a sphere near the spot at once, one within reach after the wait unless nearOnly,
// never one close enough to the other side's patch to pull both sides into one Spider Frenzy
template <typename Taken>
Creature* SideSphere(AnubarakState const& state, uint8 side, Position const& spot, Position const* other,
                     std::vector<Creature*> const& flying, std::vector<SphereDuty> const& previous, Taken const& taken,
                     uint32 now, bool nearOnly)
{
    auto const fits = [&](Creature* sphere, float within)
    {
        return !taken(sphere) && Dist2d(sphere->GetPosition(), spot) <= within &&
               (!other || Dist2d(sphere->GetPosition(), *other) >= ANUBARAK_TANK_PATCH_GAP);
    };

    // Spheres drift, so the nearest one swaps back and forth and a side's two shooters split on two
    float const keep = nearOnly ? ANUBARAK_SIDE_SHOOT_NEAR : ANUBARAK_SIDE_SHOOT_FAR;
    for (SphereDuty const& duty : previous)
        if (std::strcmp(duty.label, SIDE_LABELS[side]) == 0)
            if (Creature* held = FindSphere(flying, duty.sphere))
                if (fits(held, keep))
                    return held;

    float const within = !nearOnly && getMSTimeDiff(state.needs[side].sinceMs, now) >= ANUBARAK_SIDE_SHOOT_WAIT_MS
                             ? ANUBARAK_SIDE_SHOOT_FAR
                             : ANUBARAK_SIDE_SHOOT_NEAR;
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* sphere : flying)
    {
        if (!fits(sphere, within))
            continue;

        float const dist = Dist2d(sphere->GetPosition(), spot);
        if (!best || dist < bestDist)
        {
            best = sphere;
            bestDist = dist;
        }
    }

    return best;
}

void ComputeDuties(Player* bot, AnubarakState& state)
{
    std::vector<SphereDuty> const previous = std::move(state.duties);
    state.duties.clear();
    std::vector<GripDuty> const previousGrips = std::move(state.gripDuties);
    state.gripDuties.clear();

    // Bots only: a player never reads the assignment, and would leave a need unserved. One walking a
    // dodge would spend the duty walking. No warlock: its pet follows it onto the sphere.
    std::vector<SphereShooter> shooters;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (!member->IsAlive() || !GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRangedDps(member) ||
            member->getClass() == CLASS_WARLOCK || member->HasAura(SPELL_MARK) || !InAnubarakPit(member))
            return false;

        Position spot;
        bool const held = state.dodgeHolds.count(member->GetGUID()) && GetAnubarakDodgeHold(member, spot);
        if (held && Dist2d(member->GetPosition(), spot) > ANUBARAK_DODGE_HOLD_TRIGGER)
            return false;

        shooters.push_back({member, held});
        return false;
    });

    std::sort(shooters.begin(), shooters.end(), [](SphereShooter const& a, SphereShooter const& b)
              { return a.player->GetGUID() < b.player->GetGUID(); });

    std::vector<Creature*> const spheres = ResolveSpheres(bot, state);
    std::vector<Creature*> flying;
    for (Creature* sphere : spheres)
        if (IsFrostSphereFlying(sphere))
            flying.push_back(sphere);

    std::vector<std::pair<char const*, Creature*>> needs;
    std::vector<Creature*> reserved;
    auto const taken = [&needs, &reserved](Creature* sphere)
    {
        return std::find(reserved.begin(), reserved.end(), sphere) != reserved.end() ||
               std::any_of(needs.begin(), needs.end(),
                           [sphere](std::pair<char const*, Creature*> const& need) { return need.second == sphere; });
    };

    bool const heroic = AnubarakHeroic(bot);
    ObjectGuid const tankPatches[2] = {state.patch0, state.patch1};
    uint32 const now = getMSTime();

    // Tank patches first: a lost one is refilled ahead of any kite
    if (state.phaseValue == AnubarakPhase::Surface || state.phaseValue == AnubarakPhase::Swarm)
    {
        bool const tenManHeroic = heroic && !bot->GetMap()->Is25ManRaid();

        // A grip in flight or landed stays its side's, whichever side picks first
        for (GripCast const& cast : state.grips)
            if (cast.valid)
                if (Creature* sphere = FindSphere(flying, cast.sphere))
                    reserved.push_back(sphere);

        Position const spots[2] = {SideSpot(state, spheres, tankPatches, 0),
                                   SideSpot(state, spheres, tankPatches, 1)};
        std::vector<Creature*> const sideFlying = SideCandidates(state, flying, heroic);
        bool open[2] = {false, false};
        for (uint8 side = 0; side < 2; ++side)
        {
            SideNeed& need = state.needs[side];
            if (LatchedTankPatch(bot, state, side) || FallingFor(state, spheres, side, spots[side], spots[1 - side]) ||
                (tenManHeroic && state.sideTanks[side].IsEmpty()))
            {
                need = SideNeed();
                continue;
            }

            if (!need.open)
            {
                need.open = true;
                need.sinceMs = now;
            }

            open[side] = true;
        }

        // Where the other side's patch is, or is about to come down
        auto const otherOf = [&](uint8 side, Position& other)
        {
            if (Creature* patch = PatchToBe(state, spheres, tankPatches, 1 - side))
            {
                other = patch->GetPosition();
                return true;
            }

            if (std::any_of(state.gripDuties.begin(), state.gripDuties.end(),
                            [side](GripDuty const& duty) { return duty.side != side; }))
            {
                other = spots[1 - side];
                return true;
            }

            bool found = false;
            for (auto const& [label, sphere] : needs)
            {
                if (std::strcmp(label, SIDE_LABELS[1 - side]) == 0)
                {
                    other = sphere->GetPosition();
                    found = true;
                }
            }

            return found;
        };

        // A sphere already near a side's spot is shot there and spends no grip. At the pull one always
        // hovers over the south spot, and gripping it to the north one drags it back across the middle.
        bool served[2] = {false, false};
        for (uint8 side = 0; side < 2; ++side)
        {
            if (!open[side] || state.grips[side].valid)
                continue;

            Position other;
            bool const hasOther = otherOf(side, other);
            if (Creature* sphere =
                    SideSphere(state, side, spots[side], hasOther ? &other : nullptr, sideFlying, previous, taken, now, true))
            {
                needs.emplace_back(SIDE_LABELS[side], sphere);
                served[side] = true;
            }
        }

        for (uint8 side = 0; side < 2; ++side)
        {
            if (!open[side] || served[side])
                continue;

            // Read again for side 1: side 0's grip may have just failed
            Creature* const otherPatch = PatchToBe(state, spheres, tankPatches, 1 - side);
            if (ServeSideByGrip(bot, state, side, spots[side], otherPatch ? otherPatch->GetPosition() : spots[1 - side],
                                otherPatch, spheres, sideFlying, previousGrips, taken, needs, reserved, now))
                continue;

            Position other;
            bool const hasOther = otherOf(side, other);
            if (Creature* sphere =
                    SideSphere(state, side, spots[side], hasOther ? &other : nullptr, sideFlying, previous, taken, now, false))
                needs.emplace_back(SIDE_LABELS[side], sphere);
        }

    }
    else
    {
        state.needs[0] = SideNeed();
        state.needs[1] = SideNeed();
        state.grips[0] = GripCast();
        state.grips[1] = GripCast();
        state.relays[0] = GripRelay();
        state.relays[1] = GripRelay();
    }

    // Normal drops a sphere for a kiter with no patch; heroic's come from its own stock below. A sphere
    // already falling near the kiter lands as a patch in 1.5 s, so don't spend another.
    Player* target = GetSpikeTarget(bot);
    Creature* spike = ResolveCreature(bot, state.spikeGuid);
    if (!heroic && target && spike && spike->IsAlive() && !FallingNear(spheres, target->GetPosition()) &&
        !KitePatchAvailable(target, state, false))
    {
        std::vector<Creature*> open;
        for (Creature* sphere : flying)
            if (!taken(sphere))
                open.push_back(sphere);

        if (Creature* sphere = KiteSphere(open, target, spike))
            needs.emplace_back(KITE_LABEL, sphere);
    }

    // A kite comes every 5.5 s or so, so patches are made ahead of it. Normal stocks in phase 2 only: a
    // stock patch in phase 1 would crowd the submerge spot. Heroic's first lands clear of it.
    bool const surfaced = state.phaseValue == AnubarakPhase::Surface;
    if (state.phaseValue == AnubarakPhase::Submerged || (heroic && surfaced))
    {
        std::vector<Position> const tankSpots = LatchedTankSpots(bot, state);
        uint32 ready = 0;
        uint32 kitePatches = 0;
        for (Creature* sphere : spheres)
        {
            bool const stray = UsableKitePatch(sphere, state, tankSpots, heroic);
            kitePatches += stray ? 1 : 0;
            if (IsFrostSphereFalling(sphere) || stray)
                ++ready;
        }

        if (heroic)
        {
            uint32 const left = static_cast<uint32>(flying.size());
            OpenKiteBudget(state, left, kitePatches, now);
            uint32 const downed = KiteSpheresDowned(state, left);
            uint32 const missing = ready < ANUBARAK_P2_PATCH_STOCK_HEROIC ? ANUBARAK_P2_PATCH_STOCK_HEROIC - ready : 0;
            uint32 const budget =
                downed < ANUBARAK_KITE_SPHERES_PER_SUBMERGE ? ANUBARAK_KITE_SPHERES_PER_SUBMERGE - downed : 0;
            uint32 const want = state.kiteBudgetSet ? std::min(missing, budget) : 0;
            for (uint32 i = 0; i < want; ++i)
            {
                Creature* sphere = surfaced ? PreSubmergeKiteSphere(bot, state, flying, previous, taken)
                                            : HeroicKiteSphere(bot, state, flying, previous, spike, target, taken);
                if (!sphere)
                    break;

                needs.emplace_back("stock", sphere);
            }
        }
        else
        {
            bool const spikeUp = spike && spike->IsAlive();
            Creature* boss = GetAnubarak(bot);
            if (ready < ANUBARAK_P2_PATCH_STOCK && (spikeUp || boss))
            {
                Position const from = spikeUp ? spike->GetPosition() : boss->GetPosition();
                if (Creature* sphere = StockSphere(flying, previous, from, spikeUp ? spike : nullptr, target, taken))
                    needs.emplace_back("stock", sphere);
            }
        }
    }

    if (shooters.empty())
        return;

    auto const assigned = [&state](ObjectGuid guid)
    {
        return std::any_of(state.duties.begin(), state.duties.end(),
                           [guid](SphereDuty const& duty) { return duty.shooter == guid; });
    };
    auto const eligible = [](SphereShooter const& shooter, Creature* sphere)
    { return !shooter.parked || AnubarakSphereInReach(shooter.player, sphere); };
    auto const staffed = [&state](char const* label, ObjectGuid sphere)
    {
        return std::count_if(state.duties.begin(), state.duties.end(), [label, sphere](SphereDuty const& duty)
                             { return duty.sphere == sphere && std::strcmp(duty.label, label) == 0; });
    };
    auto const wanted = [surfaced](char const* label)
    {
        return surfaced && std::strcmp(label, "stock") == 0 ? ANUBARAK_SHOOTERS_PER_STOCK_UP
                                                            : ANUBARAK_SHOOTERS_PER_NEED;
    };

    // Latched shooters first, every need, so a vacancy further up never takes one from a need below
    for (auto const& [label, sphere] : needs)
    {
        ObjectGuid const sphereGuid = sphere->GetGUID();
        for (SphereDuty const& duty : previous)
        {
            if (staffed(label, sphereGuid) >= wanted(label))
                break;

            if (duty.sphere != sphereGuid || std::strcmp(duty.label, label) != 0 || assigned(duty.shooter))
                continue;

            auto const shooter = std::find_if(shooters.begin(), shooters.end(), [&duty](SphereShooter const& s)
                                              { return s.player->GetGUID() == duty.shooter; });
            if (shooter != shooters.end() && eligible(*shooter, sphere))
                state.duties.push_back({duty.shooter, sphereGuid, label});
        }
    }

    for (auto const& [label, sphere] : needs)
    {
        ObjectGuid const sphereGuid = sphere->GetGUID();
        while (staffed(label, sphereGuid) < wanted(label))
        {
            // Sorted by guid, so a strict comparison leaves a tie on the lower guid
            SphereShooter const* nearest = nullptr;
            float nearestDist = 0.0f;
            for (SphereShooter const& shooter : shooters)
            {
                if (assigned(shooter.player->GetGUID()) || !eligible(shooter, sphere))
                    continue;

                float const dist = shooter.player->GetExactDist2d(sphere->GetPositionX(), sphere->GetPositionY());
                if (!nearest || dist < nearestDist)
                {
                    nearest = &shooter;
                    nearestDist = dist;
                }
            }

            if (!nearest)
                break;

            state.duties.push_back({nearest->player->GetGUID(), sphereGuid, label});
        }
    }
}

// One interrupter per casting burrower, so overlapping casts never wait on one bot. The victim first,
// it's in melee already, then the lowest guid with one ready in range. Never a marked bot: it's kiting.
void ComputeStrikeDuties(Player* bot, AnubarakState& state)
{
    state.strikes.clear();

    std::vector<Creature*> casters;
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
        if (burrower->FindCurrentSpellBySpellId(SPELL_SHADOW_STRIKE))
            casters.push_back(burrower);

    if (casters.empty())
        return;

    std::sort(casters.begin(), casters.end(),
              [](Creature* a, Creature* b) { return a->GetGUID() < b->GetGUID(); });

    auto const picked = [&state](ObjectGuid guid)
    {
        return std::any_of(state.strikes.begin(), state.strikes.end(),
                           [guid](StrikeDuty const& duty) { return duty.interrupter == guid; });
    };

    std::vector<Creature*> open;
    for (Creature* caster : casters)
    {
        Unit* victim = caster->GetVictim();
        Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
        if (victimPlayer && victimPlayer->IsAlive() && !victimPlayer->HasAura(SPELL_MARK) &&
            !picked(victimPlayer->GetGUID()) && AnubarakReadyInterrupt(victimPlayer, caster))
            state.strikes.push_back({caster->GetGUID(), victimPlayer->GetGUID()});
        else
            open.push_back(caster);
    }

    if (open.empty())
        return;

    std::vector<Player*> members;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (member->IsAlive() && !member->HasAura(SPELL_MARK))
            members.push_back(member);

        return false;
    });

    std::sort(members.begin(), members.end(), [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    for (Creature* caster : open)
    {
        for (Player* member : members)
        {
            if (picked(member->GetGUID()) || !member->IsWithinDistInMap(caster, ANUBARAK_INTERRUPT_RANGE) ||
                !AnubarakReadyInterrupt(member, caster))
                continue;

            state.strikes.push_back({caster->GetGUID(), member->GetGUID()});
            break;
        }
    }
}

// Living unmarked paladin bots in Hand of Protection order: neither tank nor healer, a side tank with no
// burrower to hold or take, a healer, any other tank, lowest guid first within each
std::vector<Player*> ProtectorOrder(Player* bot)
{
    std::vector<std::pair<uint8, Player*>> ranked;
    ForEachGroupMember(bot, [&](Player* member)
    {
        PlayerbotAI* paladinAI =
            member->IsAlive() && member->getClass() == CLASS_PALADIN ? GET_PLAYERBOT_AI(member) : nullptr;
        if (!paladinAI || member->HasAura(SPELL_MARK))
            return false;

        bool const tank = IsAnubarakTankPlayer(member);
        uint8 rank = 3;
        if (!tank)
            rank = PlayerbotAI::IsHeal(member) ? 2 : 0;
        else if (GetAnubarakBurrowerTankSide(member) >= 0 && !GetAnubarakBurrowerPick(paladinAI))
            rank = 1;

        ranked.emplace_back(rank, member);
        return false;
    });

    std::sort(ranked.begin(), ranked.end(), [](auto const& a, auto const& b)
              { return a.first != b.first ? a.first < b.first : a.second->GetGUID() < b.second->GetGUID(); });

    std::vector<Player*> order;
    for (auto const& entry : ranked)
        order.push_back(entry.second);

    return order;
}

// Once per instance per ms. One paladin claims the kiter for the mark window, passing it on if its Hand
// hasn't landed in ANUBARAK_PROTECT_CLAIM_MS: the planned one, the standby escort, else any in reach
ProtectClaim const* DeriveProtection(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return nullptr;

    ProtectClaim& claim = state->protect;
    uint32 const now = getMSTime();
    if (claim.memoValid && claim.memoMs == now)
        return &claim;

    claim.memoValid = true;
    claim.memoMs = now;
    claim.need = false;
    claim.spikeInImmunityRange = false;
    claim.spikeFaster = false;
    claim.standReachable = false;
    claim.heroic = AnubarakHeroic(bot);
    claim.selfDue = false;

    Creature* spike = GetPursuingSpike(bot);
    Player* kiter = spike ? GetSpikeTarget(bot) : nullptr;
    ObjectGuid const kiterGuid = kiter ? kiter->GetGUID() : ObjectGuid::Empty;
    if (kiterGuid != claim.kiter)
    {
        claim.kiter = kiterGuid;
        claim.protector.Clear();
        claim.passed.clear();
    }

    if (!kiter)
        return &claim;

    float const spikeDist = Dist2d(spike->GetPosition(), kiter->GetPosition());
    bool const immune = AnubarakKiterImmune(kiter);
    bool const forbearance = kiter->HasAura(SPELL_FORBEARANCE);
    KitePlan const* plan = &PlanKite(kiter, spike, *state);
    float const catchLeft = CatchLeftS(*plan);
    bool const caught = catchLeft >= 0.0f;
    float const impaleMs = (caught ? catchLeft : ANUBARAK_KITE_CATCH_HORIZON_S) * IN_MILLISECONDS;

    // A kiter off the run its catch was read from isn't caught then, so the Hand also waits for the spike to
    // close. Never on a kiter outrunning it (a human on Dash too), and a heroic bot's planned boost goes first
    AnubarakKiteBoost boost;
    int const boostTick = plan->candidate.boostTick;
    bool const boostFirst = GET_PLAYERBOT_AI(kiter) && claim.heroic && boostTick >= 0 &&
                            (plan->outcome.handS < 0.0f ||
                             static_cast<float>(boostTick) * AnubarakPursuit::TICK_S <= plan->outcome.handS) &&
                            ReadySpeedBoost(kiter, boost);
    bool const handDue =
        caught && impaleMs <= ANUBARAK_KITE_HAND_LEAD_S * IN_MILLISECONDS &&
        spikeDist <= ANUBARAK_IMPALE_REACH +
                         std::max(0.0f, spike->GetSpeed(MOVE_RUN) - KiterRunSpeed(kiter)) * ANUBARAK_KITE_HAND_CLOSE_S &&
        KiterRunSpeed(kiter) <= spike->GetSpeed(MOVE_RUN) && !boostFirst;
    claim.spikeInImmunityRange = spikeDist <= ANUBARAK_KITE_IMMUNITY_RANGE;
    claim.spikeFaster = caught;

    using AnubarakKitePlanner::Ending;
    bool const handKite = HandKite(*plan);
    bool const selfHand = handKite && plan->hand == kiterGuid;
    bool const ownPlanned = plan->outcome.ownS >= 0.0f;
    // A spike within the immunity range has beaten whatever walk the plan gave the kiter
    claim.standReachable =
        (plan->outcome.ending == Ending::Patch || plan->outcome.ending == Ending::PatchHand) && !claim.spikeInImmunityRange;
    // No Hand off the plan while it still teleports first
    bool const teleportPlanned = plan->outcome.teleportS >= 0.0f;
    claim.need = !immune && !forbearance && !selfHand && handDue &&
                 (handKite || (!claim.standReachable && !ownPlanned && !teleportPlanned));
    ObjectGuid const planned = handKite && !selfHand ? plan->hand : ObjectGuid::Empty;

    if (!claim.protector.IsEmpty() && !immune)
    {
        Player* protector = ObjectAccessor::GetPlayer(*bot, claim.protector);
        // The spike fell back: the claim lapses, and the same paladin takes it again next time
        if (!claim.need)
            claim.protector.Clear();
        else if (!protector || !protector->IsAlive() ||
                 getMSTimeDiff(claim.claimMs, now) >= ANUBARAK_PROTECT_CLAIM_MS)
        {
            claim.passed.push_back(claim.protector);
            claim.protector.Clear();
        }
    }

    if (claim.need && claim.protector.IsEmpty())
    {
        // Range and sight asked here, the cast itself waits out the GCD
        auto const serves = [&](Player* paladin)
        {
            return paladin && paladin != kiter && paladin->IsAlive() && GET_PLAYERBOT_AI(paladin) &&
                   std::find(claim.passed.begin(), claim.passed.end(), paladin->GetGUID()) == claim.passed.end() &&
                   paladin->IsWithinDistInMap(kiter, ANUBARAK_HOP_RANGE) && paladin->IsWithinLOSInMap(kiter) &&
                   HandReady(paladin);
        };

        Player* pick = planned.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, planned);
        if (!serves(pick) && state->standbyKiter == kiterGuid && !state->standby.IsEmpty())
            pick = ObjectAccessor::GetPlayer(*bot, state->standby);

        if (!serves(pick))
        {
            pick = nullptr;
            for (Player* paladin : ProtectorOrder(bot))
                if (serves(paladin))
                {
                    pick = paladin;
                    break;
                }
        }

        if (pick)
        {
            claim.protector = pick->GetGUID();
            claim.claimMs = now;
        }
    }

    if (!immune)
    {
        bool const ownDue = caught && impaleMs <= ANUBARAK_KITE_OWN_LEAD_S * IN_MILLISECONDS;
        bool const low = kiter->GetHealthPct() <= ANUBARAK_KITE_OWN_LOW_HEALTH_PCT;
        bool const ownNow = ownPlanned && low;
        // Unplanned, the immunity waits for a spike the kiter can't outrun, unless one more Impale could kill
        bool const ownForced = claim.protector.IsEmpty() && !claim.standReachable &&
                               (low || KiterRunSpeed(kiter) <= spike->GetSpeed(MOVE_RUN));
        claim.selfDue = (selfHand && handDue && !forbearance) || ownNow || (ownDue && (ownPlanned || ownForced));
    }
    // A Hand kite whose own immunity waits behind its Hand casts it before the Hand ends: cast after,
    // Impale's next tick could land first
    else if (plan->handLanded && plan->candidate.own && kiter->getClass() != CLASS_PALADIN && OwnImmunitySpell(kiter))
    {
        float const left = HandLeftS(kiter);
        claim.selfDue = left > 0.0f && left <= ANUBARAK_KITE_OWN_AFTER_HAND_S;
    }

    return &claim;
}

char const* ProtectRole(Player* bot, ProtectClaim const& claim)
{
    if (claim.need && claim.protector == bot->GetGUID())
        return "hop";

    if (claim.kiter != bot->GetGUID())
        return "none";

    // The own immunity after a Hand goes out while the Hand still holds
    return claim.selfDue ? "self" : "none";
}

// Alive, unmarked, and not holding him while he's up, else null
Player* UsableSideTank(Player* bot, AnubarakState const& state, uint8 side)
{
    Player* tank = state.sideTanks[side].IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state.sideTanks[side]);
    if (!tank || !tank->IsAlive() || tank->HasAura(SPELL_MARK))
        return nullptr;

    bool const holdsHim = state.phaseValue != AnubarakPhase::Submerged && tank->GetGUID() == PickupGuid(bot, state);
    return holdsHim ? nullptr : tank;
}

// Once per instance per ms. Up to two on a usable side tank stay with it, so side tanks never taunt them off
// each other. The rest go by exit, two a side, and a side tank that can't hold its own hands them over
std::unordered_map<ObjectGuid, uint8> const& BurrowerOwners(Player* bot, AnubarakState& state)
{
    uint32 const now = getMSTime();
    if (state.ownerMemoValid && state.ownerMemoMs == now)
        return state.owners;

    state.ownerMemoValid = true;
    state.ownerMemoMs = now;
    state.owners.clear();

    bool usable[2];
    bool human[2];
    for (uint8 side = 0; side < 2; ++side)
    {
        Player* tank = UsableSideTank(bot, state, side);
        usable[side] = tank != nullptr;
        human[side] = tank && !GET_PLAYERBOT_AI(tank);
    }

    AliveCreatures burrowers = ResolveAlive(bot, state.burrowers);
    std::sort(burrowers.begin(), burrowers.end(), [](Creature* a, Creature* b) { return a->GetGUID() < b->GetGUID(); });

    uint8 count[2] = {0, 0};
    std::vector<Creature*> free;
    std::vector<Creature*> onTank[2];
    for (Creature* burrower : burrowers)
    {
        Unit* victim = burrower->GetVictim();
        ObjectGuid const victimGuid = victim ? victim->GetGUID() : ObjectGuid::Empty;
        int8 const onSide = victimGuid.IsEmpty()            ? -1
                            : victimGuid == state.sideTanks[0] ? 0
                            : victimGuid == state.sideTanks[1] ? 1
                                                               : -1;
        if (onSide >= 0 && usable[onSide])
            onTank[onSide].push_back(burrower);
        else
            free.push_back(burrower);
    }

    // A side tank keeps two of those on it while the other can take more, its own exit's first: a third
    // on one tank kills it
    std::vector<Creature*> extra[2];
    for (uint8 side = 0; side < 2; ++side)
    {
        std::vector<Creature*>& held = onTank[side];
        if (usable[1 - side] && held.size() > ANUBARAK_BURROWERS_PER_SIDE)
        {
            auto const ownExit = [&](Creature* burrower)
            {
                auto const exit = state.exits.find(burrower->GetGUID());
                return exit != state.exits.end() && exit->second.side == side;
            };

            std::stable_partition(held.begin(), held.end(), ownExit);
            extra[side].assign(held.begin() + ANUBARAK_BURROWERS_PER_SIDE, held.end());
            held.resize(ANUBARAK_BURROWERS_PER_SIDE);
        }

        for (Creature* burrower : held)
            state.owners[burrower->GetGUID()] = side;

        count[side] = static_cast<uint8>(held.size());
    }

    for (uint8 side = 0; side < 2; ++side)
        for (Creature* burrower : extra[side])
        {
            uint8 const owner = count[1 - side] < ANUBARAK_BURROWERS_PER_SIDE ? 1 - side : side;
            state.owners[burrower->GetGUID()] = owner;
            ++count[owner];
        }

    for (Creature* burrower : free)
    {
        auto const exit = state.exits.find(burrower->GetGUID());
        bool const known = exit != state.exits.end();
        uint8 owner = known ? exit->second.side : 0;
        if (!usable[owner] && usable[1 - owner])
            owner = 1 - owner;
        else if (usable[0] && usable[1] && count[owner] >= ANUBARAK_BURROWERS_PER_SIDE &&
                 count[1 - owner] < ANUBARAK_BURROWERS_PER_SIDE)
            owner = 1 - owner;

        if (human[owner] && usable[1 - owner] && !human[1 - owner] && known &&
            getMSTimeDiff(exit->second.seenMs, now) >= ANUBARAK_HUMAN_PICKUP_GRACE_MS)
            owner = 1 - owner;

        state.owners[burrower->GetGUID()] = owner;
        ++count[owner];
    }

    return state.owners;
}

constexpr uint32 REBIRTH_RANKS[] = {48477, 26994, 20748, 20747, 20742, 20739, 20484};

// Known and off cooldown. Not CanCastSpell: in cat or bear form that says no, and the action shifts out
// first.
bool RebirthReady(Player* player)
{
    if (player->getClass() != CLASS_DRUID)
        return false;

    for (uint32 rank : REBIRTH_RANKS)
        if (player->HasSpell(rank))
            return !player->HasSpellCooldown(rank);

    return false;
}

// Up and off Permafrost inside its free time: the ms of it left, else -1
int32 FreeMsLeft(AnubarakState const& state, Creature* burrower, uint32 now)
{
    auto const exit = state.exits.find(burrower->GetGUID());
    if (exit == state.exits.end() || exit->second.down || burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        UnitOnPermafrost(burrower))
        return -1;

    uint32 const up = getMSTimeDiff(exit->second.upMs, now);
    return up < ANUBARAK_BURROWER_FREE_MS ? static_cast<int32>(ANUBARAK_BURROWER_FREE_MS - up) : -1;
}

// Tanks right by it have held it ANUBARAK_FOCUS_SETTLE_MS, or no side tank will: the threat is too thin
// before that. The clock runs on across a tank swap, and a tank deep in Expose Weakness can't wait for it
bool BurrowerSettled(Player* bot, AnubarakState& state, Creature* burrower)
{
    Unit* victim = burrower->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    bool const held = holder && IsAnubarakTankPlayer(holder) && burrower->GetExactDist2d(holder) <= ANUBARAK_FOCUS_SETTLE;
    if (held &&
        holder->GetAuraCount(sSpellMgr->GetSpellIdForDifficulty(SPELL_EXPOSE_WEAKNESS, holder)) >= ANUBARAK_FOCUS_STACKS)
        return true;

    if (held)
    {
        uint32 const now = getMSTime();
        auto const [watch, fresh] = state.settles.try_emplace(burrower->GetGUID());
        if (fresh)
            watch->second.sinceMs = now;

        if (getMSTimeDiff(watch->second.sinceMs, now) >= ANUBARAK_FOCUS_SETTLE_MS)
            return true;
    }
    else
        state.settles.erase(burrower->GetGUID());

    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, state);
    auto const owner = owners.find(burrower->GetGUID());
    return owner != owners.end() && !UsableSideTank(bot, state, owner->second);
}

bool IsFocusableBurrower(Player* bot, AnubarakState& state, Creature* burrower, uint32 now)
{
    if (burrower && burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        state.settles.erase(burrower->GetGUID());

    if (!burrower || !burrower->IsAlive() || burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (burrower->GetHealthPct() < ANUBARAK_FOCUS_OFF_FROST_PCT)
        return true;

    return BurrowerSettled(bot, state, burrower) &&
           (UnitOnPermafrost(burrower) || FreeMsLeft(state, burrower, now) >= 0);
}

// The living DPS bots of each role by guid, side 0 and 1 in turn, split again at each wave so a death
// rebalances at the next one. -1 for anyone outside the split: a human, a bot that joined since.
int8 DpsSide(Player* bot, AnubarakState& state)
{
    uint32 const wave = state.wave;
    if (!state.dpsSideValid || state.dpsSideWave != wave)
    {
        state.dpsSideValid = true;
        state.dpsSideWave = wave;

        std::vector<Player*> melee;
        std::vector<Player*> ranged;
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (member->IsAlive() && GET_PLAYERBOT_AI(member) && IsToCDpsBot(member))
                (PlayerbotAI::IsMelee(member) ? melee : ranged).push_back(member);

            return false;
        });

        std::unordered_map<ObjectGuid, uint32> sides;
        for (std::vector<Player*>* role : {&melee, &ranged})
        {
            std::sort(role->begin(), role->end(), [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });
            for (size_t i = 0; i < role->size(); ++i)
                sides[(*role)[i]->GetGUID()] = static_cast<uint32>(i % 2);
        }

        for (auto it = state.dpsSide.begin(); it != state.dpsSide.end();)
            it = sides.count(it->first) ? std::next(it) : state.dpsSide.erase(it);

        for (auto const& [guid, side] : sides)
            state.dpsSide[guid] = side;
    }

    auto const it = state.dpsSide.find(bot->GetGUID());
    return it == state.dpsSide.end() ? -1 : static_cast<int8>(it->second);
}

// Heroic phase 3: one that came up after the swarm is killed like phase 1's, unless he dies within
// ANUBARAK_SWARM_IGNORE_S anyway, then it stays on its side tank and the DPS stay on him (guide)
bool LeftToSideTank(Player* bot, AnubarakState const& state, Creature* burrower)
{
    return state.swarm && state.waveLeave && AnubarakHeroic(bot) && !state.swarmBurrowers.count(burrower->GetGUID());
}

// The side's burrower DPS focus: one off Permafrost in its free time first, the soonest to end first, then the
// lowest health. The latched one stays while it qualifies, unless it's on Permafrost and another runs free
Creature* SideFocus(Player* bot, AnubarakState& state, uint8 side)
{
    uint32 const now = getMSTime();
    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, state);
    auto const onSide = [&](Creature* burrower)
    {
        auto const owner = burrower ? owners.find(burrower->GetGUID()) : owners.end();
        return owner != owners.end() && owner->second == side;
    };

    Creature* best = nullptr;
    int32 bestFree = -1;
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
    {
        if (!onSide(burrower) || LeftToSideTank(bot, state, burrower) ||
            !IsFocusableBurrower(bot, state, burrower, now))
            continue;

        // Guid breaks a tie, so the pick never hangs on the sweep's order
        int32 const free = FreeMsLeft(state, burrower, now);
        float const pct = burrower->GetHealthPct();
        bool better = !best;
        if (best && (free >= 0) != (bestFree >= 0))
            better = free >= 0;
        else if (best && free >= 0 && free != bestFree)
            better = free < bestFree;
        else if (best)
            better = pct < best->GetHealthPct() ||
                     (!(best->GetHealthPct() < pct) && burrower->GetGUID() < best->GetGUID());

        if (better)
        {
            best = burrower;
            bestFree = free;
        }
    }

    ObjectGuid& latch = side == 0 ? state.focus0 : state.focus1;
    Creature* latched = ResolveCreature(bot, latch);
    if (onSide(latched) && IsFocusableBurrower(bot, state, latched, now) &&
        (bestFree < 0 || FreeMsLeft(state, latched, now) >= 0))
        return latched;

    latch = best ? best->GetGUID() : ObjectGuid::Empty;
    return best;
}

// A burrower on someone other than a tank within this of a melee bot is its target, whatever its side, while
// that side has no side tank to taunt it back
constexpr float ANUBARAK_MELEE_LOOSE_BURROWER = 10.0f;
// A burrower stand counts as in his melee reach this far inside it, since he turns and steps
constexpr float ANUBARAK_MELEE_BOSS_REACH_SLACK = 1.0f;
// A melee bot this far off its held burrower stand walks back to it
constexpr float ANUBARAK_MELEE_STAND_ARRIVE = 1.5f;
// A held stand is picked again once its burrower moved this far from where it stood at the pick
constexpr float ANUBARAK_MELEE_STAND_DRIFT = 1.5f;

Creature* LooseBurrowerNear(Player* bot, AnubarakState& state)
{
    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, state);
    Creature* nearest = nullptr;
    float nearestDist = ANUBARAK_MELEE_LOOSE_BURROWER;
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
    {
        Unit* victim = burrower->GetVictim();
        Player* player = victim ? victim->ToPlayer() : nullptr;
        auto const owner = owners.find(burrower->GetGUID());
        if (!player || IsAnubarakTankPlayer(player) || burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
            LeftToSideTank(bot, state, burrower) ||
            (owner != owners.end() && UsableSideTank(bot, state, owner->second)))
            continue;

        float const dist = bot->GetExactDist2d(burrower);
        if (dist <= nearestDist)
        {
            nearest = burrower;
            nearestDist = dist;
        }
    }

    return nearest;
}

// No frost spheres: heroic has six for the pull, and only sphere duty spends them
bool IsAnubarakSkullTarget(Unit* unit)
{
    uint32 const entry = unit->GetEntry();
    return unit->IsCreature() && (entry == static_cast<uint32>(ToCNpcs::NPC_ANUBARAK) ||
                                  entry == static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER) ||
                                  entry == static_cast<uint32>(ToCNpcs::NPC_SWARM_SCARAB));
}

// Each scarab puts 20000 threat on a random player, so there's no tank to gather on: per bot, the
// nearest one in reach, the tanks' pickup reach by default
Creature* NearestDpsScarab(Player* bot, AnubarakState const& state, float reach = ANUBARAK_SCARAB_PICKUP_RANGE)
{
    Creature* nearest = nullptr;
    float nearestDist = reach;
    for (Creature* scarab : ResolveAlive(bot, state.scarabs))
    {
        if (scarab->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || scarab->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
            !scarab->IsInCombat())
            continue;

        float const dist = bot->GetExactDist2d(scarab);
        if (dist <= nearestDist)
        {
            nearest = scarab;
            nearestDist = dist;
        }
    }

    return nearest;
}

struct MeleeRing
{
    float reach;
    float clearance;
};

// Phases 1 and 2 keep the wider ring, since melee on him there decide the push
MeleeRing MeleeBurrowerRing(Player* bot, AnubarakState const& state, Unit const* burrower)
{
    if (state.phaseValue == AnubarakPhase::Swarm && AnubarakHeroic(bot))
        return {bot->GetMeleeRange(burrower) - ANUBARAK_SWARM_MELEE_REACH_MARGIN,
                ANUBARAK_SWARM_MELEE_PATCH_CLEARANCE};

    return {ANUBARAK_MELEE_BURROWER_REACH, ANUBARAK_MELEE_PATCH_CLEARANCE};
}

// A melee bot anywhere in reach of a burrower this far from every patch is out of the slow
bool BurrowerNearPatch(Creature* burrower, std::vector<Position> const& patches, MeleeRing const& ring)
{
    return std::any_of(patches.begin(), patches.end(), [burrower, &ring](Position const& patch)
                       { return Dist2d(burrower->GetPosition(), patch) < ring.clearance + ring.reach; });
}

constexpr float ANUBARAK_MELEE_STAND_TURNS_DEG[] = {0.0f,   20.0f, -20.0f, 40.0f,  -40.0f, 60.0f,
                                                    -60.0f, 80.0f, -80.0f, 100.0f, -100.0f};

// Points ring.reach off the burrower away from its nearest patch, turned by ANUBARAK_MELEE_STAND_TURNS_DEG. In
// his melee reach first, behind him before in front, so a switch to him needs no walk; else the one nearest him
bool MeleeBurrowerStand(Player* bot, Creature* burrower, std::vector<Position> const& patches, MeleeRing const& ring,
                        bool floorChecked, Position& out)
{
    Creature* boss = GetAnubarak(bot);
    if (boss && boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        boss = nullptr;

    Position const at = burrower->GetPosition();
    Position const* nearest = nullptr;
    for (Position const& patch : patches)
        if (!nearest || Dist2d(at, patch) < Dist2d(at, *nearest))
            nearest = &patch;

    if (!nearest)
        return false;

    Vec2 away = Delta(*nearest, at);
    float const length = Length(away);
    away = length > 0.1f ? Vec2{away.x / length, away.y / length} : Vec2{1.0f, 0.0f};

    // the floor check may shift a point, never out of melee range
    float const outer = std::min(ring.reach + 0.5f, bot->GetMeleeRange(burrower));
    auto const clear = [&](Position const& point)
    {
        return Dist2d(point, at) <= outer &&
               std::none_of(patches.begin(), patches.end(), [&point, &ring](Position const& patch)
                            { return Dist2d(point, patch) < ring.clearance; });
    };

    float const bossReach = boss ? bot->GetMeleeRange(boss) - ANUBARAK_MELEE_BOSS_REACH_SLACK : 0.0f;
    struct Candidate
    {
        uint8 tier;
        float dist;
        Position pos;
    };

    std::vector<Candidate> candidates;
    for (float degrees : ANUBARAK_MELEE_STAND_TURNS_DEG)
    {
        Position const raw = Offset(at, Rotate(away, degrees * ANUBARAK_DEG_TO_RAD), ring.reach);
        if (!clear(raw))
            continue;

        uint8 tier = 2;
        float dist = boss ? Dist2d(boss->GetPosition(), raw) : Dist2d(bot->GetPosition(), raw);
        if (boss && dist <= bossReach)
        {
            tier = boss->HasInArc(static_cast<float>(M_PI), &raw) ? 1 : 0;
            dist = Dist2d(bot->GetPosition(), raw);
        }

        candidates.push_back({tier, dist, raw});
    }

    std::sort(candidates.begin(), candidates.end(), [](Candidate const& a, Candidate const& b)
              { return a.tier != b.tier ? a.tier < b.tier : a.dist < b.dist; });

    for (Candidate const& candidate : candidates)
    {
        Position valid = candidate.pos;
        if (floorChecked && (!OnFloor(bot, candidate.pos, valid) || !clear(valid)))
            continue;

        out = valid;
        return true;
    }

    return false;
}

bool MeleeReachesBurrower(Player* bot, AnubarakState const& state, Creature* burrower)
{
    std::vector<Position> const patches = PatchPositions(bot, state);
    MeleeRing const ring = MeleeBurrowerRing(bot, state, burrower);
    Position stand;
    return !BurrowerNearPatch(burrower, patches, ring) ||
           MeleeBurrowerStand(bot, burrower, patches, ring, false, stand);
}

// The two sides' spots: a latched patch, else the side's spot off the other one. False with neither
// patch down, so he isn't dragged toward a spot no patch has come to yet.
bool AnubarakSideSpots(Player* bot, Position& spot0, Position& spot1)
{
    Creature* side0 = GetAnubarakTankPatch(bot, 0);
    Creature* side1 = GetAnubarakTankPatch(bot, 1);
    if (!side0 && !side1)
        return false;

    spot0 = side0 ? side0->GetPosition() : GetAnubarakTankSpot(bot, 0);
    spot1 = side1 ? side1->GetPosition() : GetAnubarakTankSpot(bot, 1);
    return true;
}
}

namespace TrialOfTheCrusaderHelpers
{

Creature* GetAnubarak(Player* bot)
{
    if (!InToCInstance(bot))
        return nullptr;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return nullptr;

    Creature* anubarak = ResolveCreature(bot, instance->GetGuidData(TOC_DATA_ANUBARAK));
    return anubarak && anubarak->IsAlive() ? anubarak : nullptr;
}

bool AnubarakEngaged(PlayerbotAI* botAI) { return ToCEncounterIsLive(botAI, ToCEncounter::Anubarak); }

AnubarakPhase GetAnubarakPhase(PlayerbotAI* botAI)
{
    AnubarakState const* state = ReadPhase(botAI);
    return state ? state->phaseValue : AnubarakPhase::None;
}

bool AnubarakLeechingSwarmActive(PlayerbotAI* botAI) { return GetAnubarakPhase(botAI) == AnubarakPhase::Swarm; }

bool AnubarakSwarmHealing(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && AnubarakHeroic(bot) && AnubarakEngaged(botAI) && GetAnubarakPhase(botAI) == AnubarakPhase::Swarm;
}

// The burrowers up when the swarm began are dead, or the wait ran out
static bool SwarmBurstOpen(Player* bot, AnubarakState const& state, uint32 now)
{
    if (getMSTimeDiff(state.swarmMs, now) >= ANUBARAK_SWARM_BURST_WAIT_MS)
        return true;

    return std::none_of(state.swarmBurrowers.begin(), state.swarmBurrowers.end(), [&](ObjectGuid const& guid)
    {
        Creature* burrower = ResolveCreature(bot, guid);
        return burrower && burrower->IsAlive();
    });
}

// At the pace he has lost health since he came up, he'd still be above 30% this close to the submerge
static bool BurstBehind(AnubarakState const& state, Creature* boss, uint32 now)
{
    float const elapsedS = static_cast<float>(getMSTimeDiff(state.surfacedMs, now)) / IN_MILLISECONDS;
    float const lost = state.surfacedHpPct - boss->GetHealthPct();
    if (elapsedS <= 0.0f || lost <= 0.0f)
        return true;

    float const toThirtyS = (boss->GetHealthPct() - ANUBARAK_SWARM_HEALTH_PCT) * elapsedS / lost;
    return toThirtyS > SurfacedLeftS(state, now) - ANUBARAK_BURST_BEHIND_MARGIN_S;
}

bool AnubarakBurstHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !AnubarakHeroic(bot) || !AnubarakEngaged(botAI))
        return false;

    AnubarakState* state = ReadPhase(botAI);
    Creature* boss = state ? GetAnubarak(bot) : nullptr;
    if (!boss)
        return false;

    uint32 const now = getMSTime();
    bool held = false;
    switch (state->phaseValue)
    {
        case AnubarakPhase::Swarm:
            held = !SwarmBurstOpen(bot, *state, now);
            break;
        case AnubarakPhase::Surface:
            if (boss->GetHealthPct() <= ANUBARAK_BURST_HOLD_PCT)
                held = !BurstBehind(*state, boss, now);
            break;
        case AnubarakPhase::Submerged:
            held = boss->GetHealthPct() <= ANUBARAK_BURST_HOLD_PCT;
            break;
        default:
            break;
    }

    return held;
}

bool AnubarakLustOpen(PlayerbotAI* botAI)
{
    AnubarakState* state = ReadPhase(botAI);
    if (!state || state->phaseValue != AnubarakPhase::Swarm)
        return false;

    Player* bot = botAI->GetBot();
    return !AnubarakHeroic(bot) || SwarmBurstOpen(bot, *state, getMSTime());
}

Creature* GetPursuingSpike(Player* bot)
{
    AnubarakState const* state = Sweep(bot);
    Creature* spike = state ? ResolveCreature(bot, state->spikeGuid) : nullptr;
    return spike && spike->IsAlive() ? spike : nullptr;
}

// Null off a ToC instance, where nobody carries his mark
Player* GetSpikeTarget(Player* bot)
{
    AnubarakState const* state = AnubarakStateOf(bot);
    if (!state)
        return nullptr;

    ObjectGuid const guid = state->marked.Get(bot, [bot](ObjectGuid& marked)
    {
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (member->IsAlive() && member->HasAura(SPELL_MARK))
                marked = member->GetGUID();

            return !marked.IsEmpty();
        });
    });

    return guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, guid);
}

bool AnubarakSpikeLaneCrosses(Player* bot, Position const& spot)
{
    Creature* spike = bot ? GetPursuingSpike(bot) : nullptr;
    Player* target = spike ? GetSpikeTarget(bot) : nullptr;
    return target &&
           SegmentDistance2d(spike->GetPosition(), target->GetPosition(), spot) < ANUBARAK_SPIKE_LANE_CLEARANCE;
}

bool AnubarakSpikeReachesFirst(Player* bot, Position const& spot)
{
    Creature* spike = bot ? GetPursuingSpike(bot) : nullptr;
    float const spikeSpeed = spike ? spike->GetSpeed(MOVE_RUN) : 0.0f;
    float const botSpeed = bot ? bot->GetSpeed(MOVE_RUN) : 0.0f;
    if (spikeSpeed <= 0.0f || botSpeed <= 0.0f)
        return false;

    float const walkS = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) / botSpeed;
    return Dist2d(spike->GetPosition(), spot) / spikeSpeed <= walkS + ANUBARAK_SPIKE_WALK_MARGIN_S;
}

bool IsFrostSphereFlying(Unit* sphere)
{
    return sphere && sphere->IsAlive() && sphere->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_FROST_SPHERE) &&
           sphere->HasAura(SPELL_FROST_SPHERE) && !sphere->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool IsFrostSphereFalling(Unit* sphere)
{
    return sphere && sphere->IsAlive() && sphere->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_FROST_SPHERE) &&
           sphere->HasAura(SPELL_FROST_SPHERE) && sphere->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool IsPermafrostPatch(Unit* sphere)
{
    return sphere && sphere->IsAlive() && sphere->GetEntry() == static_cast<uint32>(ToCNpcs::NPC_FROST_SPHERE) &&
           !sphere->HasAura(SPELL_FROST_SPHERE);
}

std::vector<Creature*> GetFrostSpheres(Player* bot)
{
    AnubarakState const* state = Sweep(bot);
    return state ? ResolveSpheres(bot, *state) : std::vector<Creature*>();
}

bool UnitOnPermafrost(Unit* unit)
{
    return unit && unit->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PERMAFROST, unit));
}

Position GetAnubarakTankSidePoint(uint8 side)
{
    float const offset = side == 0 ? ANUBARAK_TANK_PATCH_OFFSET : -ANUBARAK_TANK_PATCH_OFFSET;
    return Position(ANUBARAK_ROOM_CENTER.GetPositionX(), ANUBARAK_ROOM_CENTER.GetPositionY() + offset,
                    ANUBARAK_ROOM_CENTER.GetPositionZ());
}

Creature* GetAnubarakTankPatch(Player* bot, uint8 side)
{
    if (!bot || side > 1)
        return nullptr;

    AnubarakState const* state = ReadPhase(GET_PLAYERBOT_AI(bot));
    return state ? LatchedTankPatch(bot, *state, side) : nullptr;
}

Position GetAnubarakTankSpot(Player* bot, uint8 side)
{
    AnubarakState const* state = bot && side < 2 ? LatchesFor(bot) : nullptr;
    if (!state)
        return GetAnubarakTankSidePoint(side > 1 ? 0 : side);

    ObjectGuid const held[2] = {state->patch0, state->patch1};
    return SideSpot(*state, ResolveSpheres(bot, *state), held, side);
}

Position GetAnubarakSideTankHold(Player* bot, uint8 side)
{
    // Linked into its side's grip: where it grips from, so its own walks and the grip's agree
    if (AnubarakState const* state = ReadPhase(GET_PLAYERBOT_AI(bot)))
        for (GripDuty const& duty : state->gripDuties)
            if (duty.dk == bot->GetGUID() && duty.side == side)
                return duty.stand;

    Creature* patch = GetAnubarakTankPatch(bot, side);
    if (!patch)
        return GetAnubarakTankSpot(bot, side);

    Position const centre = patch->GetPosition();
    for (Creature* burrower : GetAnubarakBurrowers(bot))
        if (burrower->GetVictim() == bot && !burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
            !UnitOnPermafrost(burrower))
            return centre;

    // Not floor-checked: read every tick through the burrower pick, and it lies between the patch and the
    // anchor, both on the open floor
    Position const anchor = GetAnubarakBossAnchor(bot);
    Vec2 const toward = Delta(centre, anchor);
    float const length = Length(toward);
    if (length < ANUBARAK_SIDE_TANK_RIM)
        return centre;

    Vec2 const dir = {toward.x / length, toward.y / length};
    Creature* other = GetAnubarakTankPatch(bot, 1 - side);
    if (!other)
        return Offset(centre, dir, ANUBARAK_SIDE_TANK_RIM);

    // Patches landed close together put both holds' burrowers inside Spider Frenzy, so the rims turn apart.
    // Both side tanks read the same pair, so they turn the same way.
    Position const otherCentre = other->GetPosition();
    Vec2 const otherToward = Delta(otherCentre, anchor);
    float const otherLength = Length(otherToward);
    Vec2 const otherDir = otherLength > 0.1f ? Vec2{otherToward.x / otherLength, otherToward.y / otherLength}
                                             : Vec2{-dir.x, -dir.y};
    Creature* boss = GetAnubarak(bot);
    AnubarakPursuit::HoldDirs const dirs =
        AnubarakPursuit::SideHoldDirs(ToPoint(centre), ToPoint(otherCentre), {dir.x, dir.y}, {otherDir.x, otherDir.y},
                                      ToPoint(boss ? boss->GetPosition() : anchor), ANUBARAK_SIDE_HOLD_INSET,
                                      ANUBARAK_FRENZY_APART);
    if (!dirs.turned)
        return Offset(centre, dir, ANUBARAK_SIDE_TANK_RIM);

    // Off the line to the anchor it can leave the open floor
    Position valid;
    return OnFloor(bot, Offset(centre, {dirs.a.x, dirs.a.y}, ANUBARAK_SIDE_TANK_RIM), valid) ? valid : centre;
}

Position GetAnubarakBossAnchor(Player* bot)
{
    Position anchor;
    return GetAnubarakDragAnchor(bot, anchor) ? anchor : ANUBARAK_ROOM_CENTER;
}

Position GetAnubarakPickupStand(Player* bot)
{
    // Heroic tanks him by the east pair, next to where he waits, so he isn't run across the room and dragged
    // back. (778.0, 129.3) is walkable for bots
    Position anchor;
    if (!GetAnubarakDragAnchor(bot, anchor))
    {
        Position const spot0 = GetAnubarakPrepullSideStand(bot, 0);
        Position const spot1 = GetAnubarakPrepullSideStand(bot, 1);
        anchor = Position((spot0.GetPositionX() + spot1.GetPositionX()) / 2.0f,
                          (spot0.GetPositionY() + spot1.GetPositionY()) / 2.0f, ANUBARAK_ROOM_CENTER.GetPositionZ());
    }

    return Position(anchor.GetPositionX() - ANUBARAK_PICKUP_STAND_OFFSET, anchor.GetPositionY(),
                    anchor.GetPositionZ());
}

Position GetAnubarakPrepullSideStand(Player* bot, uint8 side)
{
    if (!bot || !AnubarakHeroic(bot))
        return GetAnubarakTankSidePoint(side > 1 ? 0 : side);

    std::array<AnubarakPursuit::Point, 2> const spots = HeroicPairSpots();
    AnubarakPursuit::Point const& spot = spots[side > 1 ? 0 : side];
    return Position(spot.x, spot.y, ANUBARAK_ROOM_CENTER.GetPositionZ());
}

bool GetAnubarakDragAnchor(Player* bot, Position& out)
{
    Position spot0;
    Position spot1;
    if (!AnubarakSideSpots(bot, spot0, spot1))
        return false;

    out = Position((spot0.GetPositionX() + spot1.GetPositionX()) / 2.0f,
                   (spot0.GetPositionY() + spot1.GetPositionY()) / 2.0f,
                   (spot0.GetPositionZ() + spot1.GetPositionZ()) / 2.0f);
    return true;
}

std::vector<Position> GetAnubarakDragPoints(Player* bot, Creature* boss)
{
    std::vector<Position> points;
    Position anchor;
    Position spot0;
    Position spot1;
    AnubarakState* state = boss ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state || !GetAnubarakDragAnchor(bot, anchor) || !AnubarakSideSpots(bot, spot0, spot1))
        return points;

    points.push_back(anchor);

    // Patches that landed close together leave no gap on the line between them, so a leg refused at the
    // anchor can still end short of it on his side
    Vec2 along = Delta(spot0, spot1);
    float const length = Length(along);
    if (length >= 1.0f)
    {
        Vec2 normal = {-along.y / length, along.x / length};
        Vec2 const toBoss = Delta(anchor, boss->GetPosition());
        if (normal.x * toBoss.x + normal.y * toBoss.y < 0.0f)
            normal = {-normal.x, -normal.y};

        for (float offset : {4.0f, 7.5f})
            points.push_back(Offset(anchor, normal, offset));
    }

    // He comes up where the spike was, which can be past a patch on the line through both, and every leg
    // from there walks the tank through it
    std::vector<AnubarakPursuit::Point> patches;
    for (Position const& patch : PatchPositions(bot, *state))
        patches.push_back(ToPoint(patch));

    auto const add = [&](std::vector<AnubarakPursuit::Point> const& extra)
    {
        for (AnubarakPursuit::Point const& point : extra)
        {
            Position const pos(point.x, point.y, anchor.GetPositionZ());
            if (Dist2d(pos, ANUBARAK_ROOM_CENTER) <= ANUBARAK_SUBMERGE_SPOT_FLOOR)
                points.push_back(pos);
        }
    };

    AnubarakPursuit::Point const from = ToPoint(boss->GetPosition());
    add(AnubarakPursuit::DragDetours(from, ToPoint(anchor), patches, ANUBARAK_DRAG_DETOUR_TRIGGER,
                                     ANUBARAK_DRAG_DETOUR_STANDOFF, ANUBARAK_TANK_KEEPOUT));

    uint32 const now = getMSTime();
    if (!state->dragFreeValid || state->drag.valid || Dist2d(boss->GetPosition(), anchor) <= ANUBARAK_DRAG_STALL_RANGE)
    {
        state->dragFreeValid = true;
        state->dragFreeMs = now;
    }
    else if (getMSTimeDiff(state->dragFreeMs, now) >= ANUBARAK_DRAG_STALL_MS)
        add(AnubarakPursuit::RingPoints(ToPoint(anchor), ANUBARAK_DRAG_STALL_RANGE, from, patches,
                                        ANUBARAK_TANK_KEEPOUT));

    return points;
}

std::vector<Position> GetAnubarakSubmergeDragPoints(Player* bot, Creature* boss, Position const& spot)
{
    std::vector<Position> points = {spot};
    AnubarakState* state = boss ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return points;

    // A patch on the way gives no straight leg, and he'd go down wherever he stands, maybe next to it
    std::vector<AnubarakPursuit::Point> patches;
    for (Position const& patch : PatchPositions(bot, *state))
        patches.push_back(ToPoint(patch));

    // Only ones nearer the spot than he is, so arriving at one never offers the way back
    float const left = Dist2d(boss->GetPosition(), spot);
    for (AnubarakPursuit::Point const& point :
         AnubarakPursuit::DragDetours(ToPoint(boss->GetPosition()), ToPoint(spot), patches,
                                      ANUBARAK_DRAG_DETOUR_TRIGGER, ANUBARAK_DRAG_DETOUR_STANDOFF,
                                      ANUBARAK_TANK_KEEPOUT))
    {
        Position const pos(point.x, point.y, spot.GetPositionZ());
        if (Dist2d(pos, ANUBARAK_ROOM_CENTER) <= ANUBARAK_SUBMERGE_SPOT_FLOOR && Dist2d(pos, spot) < left)
            points.push_back(pos);
    }

    return points;
}

// Heroic: the ring point in drag reach farthest from the raid point, by bands, nearest the tank on a tie. The
// spike is summoned there, so the first chase starts that far from the raid
static bool FarSubmergeSpot(Player* bot, Creature* boss, AnubarakState const& state, std::vector<Position> const& patches,
                            Position& out, float& clearanceOut)
{
    if (!state.raidPointValid || !boss)
        return false;

    struct Candidate
    {
        Position pos;
        float fromRaid;
        float fromTank;
    };

    Position const& raid = state.raidPoint;
    std::vector<Candidate> ring;
    for (float radius : ANUBARAK_SUBMERGE_FAR_RINGS)
        for (uint32 i = 0; i < ANUBARAK_SUBMERGE_FAR_HEADINGS; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) /
                                static_cast<float>(ANUBARAK_SUBMERGE_FAR_HEADINGS);
            Position const pos = Offset(ANUBARAK_ROOM_CENTER, {std::cos(angle), std::sin(angle)}, radius);
            if (Dist2d(pos, boss->GetPosition()) <= ANUBARAK_SUBMERGE_DRAG_REACH)
                ring.push_back({pos, Dist2d(pos, raid), Dist2d(pos, bot->GetPosition())});
        }

    std::sort(ring.begin(), ring.end(), [](Candidate const& a, Candidate const& b)
              {
                  int32 const bandA = static_cast<int32>(a.fromRaid / ANUBARAK_SUBMERGE_FAR_BAND);
                  int32 const bandB = static_cast<int32>(b.fromRaid / ANUBARAK_SUBMERGE_FAR_BAND);
                  return bandA != bandB ? bandA > bandB : a.fromTank < b.fromTank;
              });

    for (bool const farFirst : {true, false})
        for (float clearance : ANUBARAK_SUBMERGE_CLEARANCES)
            for (Candidate const& candidate : ring)
            {
                if (farFirst && candidate.fromRaid < ANUBARAK_SUBMERGE_FAR_MIN)
                    break;

                bool const blocked =
                    std::any_of(patches.begin(), patches.end(), [&](Position const& patch)
                                {
                                    return Dist2d(candidate.pos, patch) < clearance ||
                                           SegmentDistance2d(candidate.pos, raid, patch) < ANUBARAK_SPIKE_ROUTE_CLEARANCE;
                                });
                Position valid;
                if (blocked || !OnFloor(bot, candidate.pos, valid))
                    continue;

                out = valid;
                clearanceOut = clearance;
                return true;
            }

    return false;
}

// Phase 3 starts on the anchor if the push gets him under 30% before he submerges, so no drag while it
// still does. Read every tick, so a push that stalls still drags, just late.
static bool PushHoldsDrag(AnubarakState const& state, Creature* boss, uint32 now)
{
    if (!state.push || !boss)
        return false;

    float const health = boss->GetHealthPct();
    uint32 const since = getMSTimeDiff(state.pushMs, now);
    float const perS =
        since >= ANUBARAK_PUSH_LIVE_MS ? (state.pushHpPct - health) * IN_MILLISECONDS / since : state.pushPerS;
    float const neededS = perS > 0.0f ? (health - ANUBARAK_PUSH_TARGET_PCT) / perS : -1.0f;
    float const leftS = SurfacedLeftS(state, now);
    return health <= ANUBARAK_PUSH_TARGET_PCT || (neededS >= 0.0f && neededS + ANUBARAK_PUSH_DRAG_MARGIN_S <= leftS);
}

bool GetAnubarakSubmergeSpot(Player* bot, Position& out)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    bool const heroic = state && AnubarakHeroic(bot);
    uint32 const now = getMSTime();
    if (!state || state->phaseValue != AnubarakPhase::Surface ||
        getMSTimeDiff(state->surfacedMs, now) < (heroic ? ANUBARAK_SUBMERGE_FAR_SEARCH_MS : ANUBARAK_SUBMERGE_PREP_MS))
        return false;

    std::vector<Position> patches;
    for (Creature* sphere : ResolveAlive(bot, state->spheres))
        if (IsPermafrostPatch(sphere))
            patches.push_back(sphere->GetPosition());

    if (patches.empty())
        return false;

    auto const clear = [&patches](Position const& spot, float clearance)
    {
        return std::none_of(patches.begin(), patches.end(),
                            [&spot, clearance](Position const& patch) { return Dist2d(spot, patch) < clearance; });
    };

    // Latched for the window, the sweep raycasts every candidate. Redone once a patch lands closer than
    // the clearance the spot was found with.
    if (!state->submergeSpotValid || !clear(state->submergeSpot, state->submergeSpotClearance))
    {
        // The floor is walkable out to 45 yd, so he never ends up against a wall
        auto const onFloor = [](float x, float y)
        { return ANUBARAK_ROOM_CENTER.GetExactDist2d(x, y) <= ANUBARAK_SUBMERGE_SPOT_FLOOR; };

        // The spike is summoned on him, so it starts away from the raid and its run to the raid spends no
        // patch. Dropped when nothing passes.
        Position const raid = state->raidPoint;
        auto const offRaid = [&](float x, float y)
        {
            Position const spot(x, y, raid.GetPositionZ());
            return onFloor(x, y) && Dist2d(spot, raid) >= ANUBARAK_SUBMERGE_RAID_CLEARANCE &&
                   std::none_of(patches.begin(), patches.end(), [&](Position const& patch)
                                { return SegmentDistance2d(spot, raid, patch) < ANUBARAK_SPIKE_ROUTE_CLEARANCE; });
        };

        // Within the nearest clear ring, the spot toward a point 40 yd past the tank, away from the tank
        // patches, so the spike starts on the far side from both
        Position prefer = ANUBARAK_ROOM_CENTER;
        std::vector<Position> tankPatches;
        for (uint8 side = 0; side < 2; ++side)
            if (Creature* patch = LatchedTankPatch(bot, *state, side))
                tankPatches.push_back(patch->GetPosition());

        if (!tankPatches.empty())
        {
            Position centroid(0.0f, 0.0f, 0.0f);
            for (Position const& patch : tankPatches)
                centroid.Relocate(centroid.GetPositionX() + patch.GetPositionX() / tankPatches.size(),
                                  centroid.GetPositionY() + patch.GetPositionY() / tankPatches.size());

            Vec2 const away = Delta(centroid, bot->GetPosition());
            float const length = Length(away);
            if (length > 1.0f)
                prefer = Offset(bot->GetPosition(), {away.x / length, away.y / length},
                                ANUBARAK_SUBMERGE_SPOT_SEARCH);
        }

        state->submergeSpotValid = false;
        Position farSpot;
        float farClearance = 0.0f;
        if (heroic && FarSubmergeSpot(bot, GetAnubarak(bot), *state, patches, farSpot, farClearance))
        {
            state->submergeSpotValid = true;
            state->submergeSpot = farSpot;
            state->submergeSpotClearance = farClearance;
        }

        HazardSweepCache cache;
        for (bool const awayFromRaid : {true, false})
        {
            if (state->submergeSpotValid || (awayFromRaid && !state->raidPointValid))
                continue;

            for (float clearance : ANUBARAK_SUBMERGE_CLEARANCES)
            {
                std::vector<HazardCircle> hazards;
                for (Position const& patch : patches)
                    hazards.emplace_back(patch, clearance);

                std::function<bool(float, float)> const accept =
                    awayFromRaid ? std::function<bool(float, float)>(offRaid) : std::function<bool(float, float)>(onFloor);
                Position const spot = FindNearestPositionClearOfHazards(bot, hazards, ANUBARAK_SUBMERGE_SPOT_SEARCH,
                                                                        2.0f, static_cast<float>(M_PI) / 8.0f, &prefer,
                                                                        accept, &cache);
                if (spot == Position())
                    continue;

                state->submergeSpotValid = true;
                state->submergeSpot = spot;
                state->submergeSpotClearance = clearance;
                break;
            }
        }
    }

    if (!state->submergeSpotValid)
        return false;

    // Heroic's drag leaves the anchor with under 18 s left, later still once the walk there takes less
    if (heroic && !state->submergeDragStarted)
    {
        Creature* boss = GetAnubarak(bot);
        float const walkS = boss ? Dist2d(boss->GetPosition(), state->submergeSpot) / ANUBARAK_SUBMERGE_DRAG_SPEED +
                                       ANUBARAK_SUBMERGE_DRAG_MARGIN_S + ANUBARAK_SUBMERGE_DRAG_STUN_S
                                 : 0.0f;
        if (SurfacedLeftS(*state, now) >= std::min(ANUBARAK_SUBMERGE_DRAG_LEAD_S, walkS) ||
            PushHoldsDrag(*state, boss, now))
            return false;

        state->submergeDragStarted = true;
    }

    out = state->submergeSpot;
    return true;
}

// Pre-steps sit this far inside his chase start, so he stays put while the tank walks round him
constexpr float ANUBARAK_DRAG_PRESTEP_INSIDE[] = {3.0f, 2.0f, 1.0f};

// The first DragPreSteps point on the floor that's still inside his chase start
static bool DragPreStep(Player* bot, Creature* boss, Position const& spot, std::vector<Position> const& patches,
                        float chaseStart, Position& out)
{
    std::vector<AnubarakPursuit::Point> points;
    for (Position const& patch : patches)
        points.push_back(ToPoint(patch));

    std::vector<float> radii;
    for (float inside : ANUBARAK_DRAG_PRESTEP_INSIDE)
        radii.push_back(chaseStart - inside);

    for (AnubarakPursuit::Point const& point :
         AnubarakPursuit::DragPreSteps(ToPoint(boss->GetPosition()), ToPoint(bot->GetPosition()), ToPoint(spot), points,
                                       radii, ANUBARAK_TANK_KEEPOUT, ANUBARAK_PERMAFROST_SLOW_REACH))
    {
        Position valid;
        if (OnFloor(bot, Position(point.x, point.y, bot->GetPositionZ()), valid) &&
            Dist2d(valid, boss->GetPosition()) < chaseStart && TankWalkClear(bot->GetPosition(), valid, patches) &&
            TankWalkClear(valid, spot, patches))
        {
            out = valid;
            return true;
        }
    }

    return false;
}

AnubarakDragLeg GetAnubarakDragLeg(Player* bot, Creature* boss, Position const& point, bool submerge)
{
    AnubarakDragLeg leg;
    AnubarakState* state = bot && boss ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return leg;

    DragLegLatch& latch = state->drag;
    if (latch.valid && latch.tank != bot->GetGUID())
        latch = DragLegLatch();

    if (boss->GetVictim() != bot)
    {
        latch = DragLegLatch();
        return leg;
    }

    uint32 const now = getMSTime();
    if (latch.valid && Dist2d(boss->GetPosition(), latch.point) <= ANUBARAK_DRAG_DONE)
    {
        latch = DragLegLatch();
        return leg;
    }

    // Followed on the pre-step, the leg planned off where he stood is stale
    if (latch.valid && latch.preStep)
    {
        if (latch.bossMoved || getMSTimeDiff(latch.plannedMs, now) >= ANUBARAK_DRAG_LEG_MS)
            latch = DragLegLatch();
        else if (bot->GetExactDist2d(latch.preStepSpot.GetPositionX(), latch.preStepSpot.GetPositionY()) >
                 ANUBARAK_DRAG_SPOT_ARRIVE)
        {
            leg.step = AnubarakDragStep::Walk;
            leg.tankSpot = latch.preStepSpot;
            leg.preStep = true;
            return leg;
        }
        else
        {
            latch.preStep = false;
            latch.plannedMs = now;
        }
    }

    if (latch.valid)
    {
        bool const onSpot = bot->GetExactDist2d(latch.tankSpot.GetPositionX(), latch.tankSpot.GetPositionY()) <=
                            ANUBARAK_DRAG_SPOT_ARRIVE;
        if (onSpot && !latch.arrived)
        {
            latch.arrived = true;
            latch.arrivedMs = now;
        }

        // He can still stay put (a cast, a stun), so a tank waiting on its spot judges after the settle time
        bool const followed =
            latch.bossMoved || (latch.arrived && getMSTimeDiff(latch.arrivedMs, now) >= ANUBARAK_DRAG_SETTLE_MS);
        bool const settled = followed && BossStillFor(*state, now, ANUBARAK_DRAG_SETTLE_MS);
        if (!settled && getMSTimeDiff(latch.plannedMs, now) < ANUBARAK_DRAG_LEG_MS)
        {
            leg.step = onSpot ? AnubarakDragStep::Settle : AnubarakDragStep::Walk;
            leg.tankSpot = latch.tankSpot;
            return leg;
        }

        // Judged or dropped. A moved point is judged here too, never mid walk.
        latch = DragLegLatch();
    }

    float const start = submerge ? ANUBARAK_DRAG_DONE : ANUBARAK_DRAG_START;
    float const toPoint = Dist2d(boss->GetPosition(), point);
    if (toPoint <= start || !BossStillFor(*state, now, ANUBARAK_DRAG_SETTLE_MS))
        return leg;

    // He chases from past both reaches plus contact and runs on to his next recheck once the bot stops, so
    // the bot stands far enough out that he stops on the point. The hold action walks it backwards
    float const bossSpeed = boss->GetSpeed(MOVE_RUN);
    float const botSpeed = bot->GetSpeed(MOVE_RUN_BACK);
    if (bossSpeed <= 0.0f || botSpeed <= 0.0f)
        return leg;

    float const chaseStart = boss->GetCombatReach() + bot->GetCombatReach() + CONTACT_DISTANCE;
    float const speedRatio = bossSpeed / botSpeed;
    float const stopLag = bossSpeed * ANUBARAK_DRAG_STOP_LAG_S;
    float const lead = chaseStart + std::max(toPoint - stopLag, 0.0f) / speedRatio;

    Vec2 dir = Delta(boss->GetPosition(), point);
    dir = {dir.x / toPoint, dir.y / toPoint};
    Position const spot = ValidateFloorPoint(bot, Offset(boss->GetPosition(), dir, lead));
    // Floor check pulled it inside his reach, so he'd never follow and the leg would replan forever
    if (Dist2d(spot, boss->GetPosition()) <= chaseStart)
        return leg;

    // Slowed 80% the tank crawls and he stops short, so no leg ends in a patch or walks through one. A
    // tank cut off from its spot by one first steps round him.
    std::vector<Position> const patches = PatchPositions(bot, *state);
    bool const spotClear = std::none_of(patches.begin(), patches.end(), [&spot](Position const& patch)
                                        { return Dist2d(spot, patch) < ANUBARAK_TANK_KEEPOUT; });
    Position preStep;
    bool const cutOff = !TankWalkClear(bot->GetPosition(), spot, patches);
    if (!spotClear || (cutOff && !DragPreStep(bot, boss, spot, patches, chaseStart, preStep)))
        return leg;

    latch.valid = true;
    latch.tank = bot->GetGUID();
    latch.point = point;
    latch.tankSpot = spot;
    latch.preStep = cutOff;
    latch.preStepSpot = preStep;
    latch.plannedMs = now;
    if (cutOff)
    {
        leg.step = AnubarakDragStep::Walk;
        leg.tankSpot = preStep;
        leg.preStep = true;
        return leg;
    }

    bool const onSpot = bot->GetExactDist2d(latch.tankSpot.GetPositionX(), latch.tankSpot.GetPositionY()) <=
                        ANUBARAK_DRAG_SPOT_ARRIVE;
    if (onSpot)
    {
        latch.arrived = true;
        latch.arrivedMs = now;
    }

    leg.step = onSpot ? AnubarakDragStep::Settle : AnubarakDragStep::Walk;
    leg.tankSpot = latch.tankSpot;
    return leg;
}

void ClearAnubarakDrag(Player* bot)
{
    if (!InToCInstance(bot))
        return;

    AnubarakState* state = anubarakStates.Find(bot->GetInstanceId());
    if (state && state->drag.valid && state->drag.tank == bot->GetGUID())
        state->drag = DragLegLatch();
}

bool IsAnubarakDragInFlight(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!InToCInstance(bot))
        return false;

    AnubarakState const* state = anubarakStates.Find(bot->GetInstanceId());
    if (!state || !state->drag.valid || state->drag.tank != bot->GetGUID() ||
        getMSTimeDiff(state->drag.plannedMs, getMSTime()) >= ANUBARAK_DRAG_LEG_MS)
        return false;

    Creature* boss = GetAnubarak(bot);
    return boss && boss->GetVictim() == bot && Dist2d(boss->GetPosition(), state->drag.point) > ANUBARAK_DRAG_DONE;
}

bool GetAnubarakTankStepOut(Player* bot, Creature* boss, Position& out)
{
    AnubarakState const* state = bot && boss ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    std::vector<Position> const patches = PatchPositions(bot, *state);
    bool const inside = UnitOnPermafrost(bot) ||
                        std::any_of(patches.begin(), patches.end(), [bot](Position const& patch)
                                    { return Dist2d(bot->GetPosition(), patch) < ANUBARAK_PERMAFROST_SLOW_REACH; });
    if (!inside)
        return false;

    std::vector<HazardCircle> hazards;
    for (Position const& patch : patches)
        hazards.emplace_back(patch, ANUBARAK_TANK_KEEPOUT);

    // He follows past 12 yd, so the step stays inside that when it can and he keeps his place
    Position const bossPos = boss->GetPosition();
    float const reach = boss->GetCombatReach() + bot->GetCombatReach();
    auto const nearHim = [&bossPos, reach](float x, float y) { return bossPos.GetExactDist2d(x, y) <= reach; };

    HazardSweepCache cache;
    Position spot = FindNearestPositionClearOfHazards(bot, hazards, ANUBARAK_TANK_STEP_SEARCH, 1.5f,
                                                      static_cast<float>(M_PI) / 8.0f, &bossPos, nearHim, &cache);
    if (spot == Position())
        spot = FindNearestPositionClearOfHazards(bot, hazards, ANUBARAK_TANK_STEP_SEARCH, 1.5f,
                                                 static_cast<float>(M_PI) / 8.0f, &bossPos, {}, &cache);

    if (spot == Position())
        return false;

    out = spot;
    return true;
}

bool AnubarakTankPatchLatched(Player* bot)
{
    return GetAnubarakTankPatch(bot, 0) || GetAnubarakTankPatch(bot, 1);
}

bool AnubarakTauntWouldSteal(PlayerbotAI* botAI)
{
    if (!botAI || !AnubarakEngaged(botAI))
        return false;

    Player* bot = botAI->GetBot();
    Creature* boss = GetAnubarak(bot);
    if (!boss || botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != boss)
        return false;

    Unit* victim = boss->GetVictim();
    if (!victim || victim == bot)
        return false;

    Player* holder = victim->ToPlayer();
    return holder && IsAnubarakTankPlayer(holder);
}

bool AnubarakThreatCapped(Player* bot)
{
    if (!IsToCDpsBot(bot))
        return false;

    AnubarakState* state = ReadPhase(GET_PLAYERBOT_AI(bot));
    if (!state || (state->phaseValue != AnubarakPhase::Surface && state->phaseValue != AnubarakPhase::Swarm))
        return false;

    ThreatCapLatch& cap = state->threatCaps[bot->GetGUID()];
    uint32 const now = getMSTime();
    if (cap.memoValid && cap.memoMs == now)
        return cap.value;

    cap.memoValid = true;
    cap.memoMs = now;
    cap.value = false;

    Creature* boss = GetAnubarak(bot);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    Player* tank = victim ? victim->ToPlayer() : nullptr;
    if (!tank || tank == bot || !IsAnubarakTankPlayer(tank))
        return false;

    // The "threat" AI value is a share in a uint8 and wraps past 255%
    float const own = boss->GetThreatMgr().GetThreat(bot);
    float const held = boss->GetThreatMgr().GetThreat(tank);
    if (!cap.capped && own > 0.0f && own >= ANUBARAK_THREAT_CAP * held)
        cap.capped = true;
    else if (cap.capped && own < ANUBARAK_THREAT_RELEASE * held)
        cap.capped = false;

    cap.value = cap.capped;
    return cap.value;
}

bool AnubarakBurrowerThreatCapped(Player* bot, Unit* burrower)
{
    if (!IsToCDpsBot(bot) || !burrower || burrower->GetEntry() != static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER))
        return false;

    AnubarakState* state = ReadPhase(GET_PLAYERBOT_AI(bot));
    Unit* victim = state ? burrower->GetVictim() : nullptr;
    Player* tank = victim ? victim->ToPlayer() : nullptr;
    if (!tank || tank == bot || !IsAnubarakTankPlayer(tank))
        return false;

    float const own = burrower->GetThreatMgr().GetThreat(bot);
    float const held = burrower->GetThreatMgr().GetThreat(tank);
    bool& capped = state->burrowerCaps[{bot->GetGUID(), burrower->GetGUID()}];
    if (!capped && own > 0.0f && own >= ANUBARAK_THREAT_CAP * held)
        capped = true;
    else if (capped && own < ANUBARAK_THREAT_RELEASE * held)
        capped = false;

    return capped;
}

bool AnubarakBurrowerAreaCapped(Player* bot, Unit* burrower)
{
    if (!IsToCDpsBot(bot) || !PlayerbotAI::IsMelee(bot) || !burrower ||
        burrower->GetEntry() != static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER))
        return false;

    AnubarakState* state = ReadPhase(GET_PLAYERBOT_AI(bot));
    Unit* victim = state ? burrower->GetVictim() : nullptr;
    Player* tank = victim ? victim->ToPlayer() : nullptr;
    if (!tank || tank == bot || !IsAnubarakTankPlayer(tank))
        return false;

    float const own = burrower->GetThreatMgr().GetThreat(bot);
    float const held = burrower->GetThreatMgr().GetThreat(tank);
    bool& capped = state->burrowerAreaCaps[{bot->GetGUID(), burrower->GetGUID()}];
    if (!capped && own > 0.0f && own >= ANUBARAK_MELEE_AREA_CAP * held)
        capped = true;
    else if (capped && own < ANUBARAK_MELEE_AREA_RELEASE * held)
        capped = false;

    return capped;
}

bool AnubarakBurrowerAreaCappedNear(Player* bot, Position const& at, float range)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
        if (!burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
            Dist2d(burrower->GetPosition(), at) <= range + burrower->GetCombatReach() &&
            AnubarakBurrowerAreaCapped(bot, burrower))
            return true;

    return false;
}

char const* GetAnubarakCleaveBuffToDrop(PlayerbotAI* botAI)
{
    static char const* const buffs[] = {"blade flurry", "sweeping strikes"};
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !IsToCDpsBot(bot) || !PlayerbotAI::IsMelee(bot) || !AnubarakEngaged(botAI))
        return nullptr;

    for (char const* buff : buffs)
        if (botAI->HasAura(buff, bot) &&
            AnubarakBurrowerAreaCappedNear(bot, bot->GetPosition(), NOMINAL_MELEE_RANGE + bot->GetCombatReach()))
            return buff;

    return nullptr;
}

bool AnubarakBurrowerThreatCappedNear(Player* bot, Position const& at, float range)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
        if (!burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && Dist2d(burrower->GetPosition(), at) <= range &&
            AnubarakBurrowerThreatCapped(bot, burrower))
            return true;

    return false;
}

int8 GetAnubarakBurrowerTankSide(Player* player)
{
    AnubarakState const* state = LatchesFor(player);
    if (!state)
        return -1;

    for (uint8 side = 0; side < 2; ++side)
        if (state->sideTanks[side] == player->GetGUID())
            return side;

    return -1;
}

bool IsAnubarakPickupTank(Player* bot)
{
    AnubarakState const* state = bot && bot->IsAlive() ? LatchesFor(bot) : nullptr;
    return state && PickupGuid(bot, *state) == bot->GetGUID();
}

bool IsAnubarakTankPlayer(Player* player)
{
    return player && (PlayerbotAI::IsTank(player) || PlayerbotAI::IsTank(player, true));
}

AnubarakPrepullTanks GetAnubarakPrepullTanks(Player* bot)
{
    AnubarakPrepullTanks tanks;
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group)
        return tanks;

    bool walked = false;
    ObjectGuid const mainTank = StableMainTankGuid(group, walked);
    std::vector<ObjectGuid> const sides = RankSideTanks(bot, group, mainTank, walked);
    for (size_t side = 0; side < 2 && side < sides.size(); ++side)
        tanks.side[side] = sides[side];

    Position const points[2] = {GetAnubarakTankSidePoint(0), GetAnubarakTankSidePoint(1)};
    if (HumanOnOtherSpot(bot, tanks.side, points) >= 0)
        std::swap(tanks.side[0], tanks.side[1]);

    Player* main = mainTank.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, mainTank);
    tanks.pickup = main && main->IsAlive() ? mainTank : FirstBotSideTank(bot, tanks.side);
    return tanks;
}

std::vector<Creature*> GetAnubarakBurrowers(Player* bot)
{
    AnubarakState const* state = Sweep(bot);
    return state ? ResolveAlive<std::vector<Creature*>>(bot, state->burrowers) : std::vector<Creature*>();
}

bool AnubarakSideTankNeedsDefensive(Player* bot)
{
    return bot && bot->IsAlive() && GetAnubarakBurrowerTankSide(bot) >= 0 &&
           (bot->GetAuraCount(sSpellMgr->GetSpellIdForDifficulty(SPELL_EXPOSE_WEAKNESS, bot)) >=
                ANUBARAK_SIDE_DEFENSIVE_STACKS ||
            bot->GetHealthPct() < ANUBARAK_SIDE_DEFENSIVE_PCT);
}

bool AnubarakSideTankDefensiveKept(PlayerbotAI* botAI, uint32 spellId)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Creature* boss = bot ? GetAnubarak(bot) : nullptr;
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!boss || !info || !AnubarakHeroic(bot) || bot->GetHealthPct() < ANUBARAK_SIDE_DEFENSIVE_PCT ||
        boss->GetHealthPct() > ANUBARAK_SIDE_DEFENSIVE_KEEP_PCT ||
        std::max(info->RecoveryTime, info->CategoryRecoveryTime) < ANUBARAK_SIDE_DEFENSIVE_KEEP_COOLDOWN_MS)
        return false;

    AnubarakPhase const phase = GetAnubarakPhase(botAI);
    return phase == AnubarakPhase::Surface || phase == AnubarakPhase::Submerged;
}

Player* GetAnubarakBattleResTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AnubarakState const* state = bot ? ReadPhase(botAI) : nullptr;
    if (!state || !RebirthReady(bot) || bot->HasAura(SPELL_MARK))
        return nullptr;

    // One druid per corpse: the lowest guid with Rebirth ready, a tank only when no other druid can. Never
    // the one holding him: the cast takes it out of bear form.
    bool const surfaced = state->phaseValue == AnubarakPhase::Surface || state->phaseValue == AnubarakPhase::Swarm;
    ObjectGuid const pickup = surfaced ? PickupGuid(bot, *state) : ObjectGuid::Empty;
    ObjectGuid const druid = state->resDruid.Get(
        bot,
        [bot, &pickup](ObjectGuid& first)
        {
            bool firstTank = false;
            ForEachGroupMember(bot, [&](Player* member)
            {
                if (!member->IsAlive() || !GET_PLAYERBOT_AI(member) || member->HasAura(SPELL_MARK) ||
                    !RebirthReady(member) || member->GetGUID() == pickup)
                    return false;

                bool const tank = IsAnubarakTankPlayer(member);
                if (first.IsEmpty() || (firstTank && !tank) || (firstTank == tank && member->GetGUID() < first))
                {
                    first = member->GetGUID();
                    firstTank = tank;
                }

                return false;
            });
        },
        pickup);

    if (druid != bot->GetGUID())
        return nullptr;

    // A pending Soulstone or Reincarnation brings it back without the Rebirth
    auto const resable = [](Player* dead)
    {
        return dead && !dead->IsAlive() && dead->getDeathState() == DeathState::Corpse &&
               !dead->isResurrectRequested() &&
               (dead->HasPlayerFlag(PLAYER_FLAGS_GHOST) || !dead->GetUInt32Value(PLAYER_SELF_RES_SPELL));
    };

    for (ObjectGuid const& guid : state->deadSideTanks)
    {
        Player* tank = ObjectAccessor::GetPlayer(*bot, guid);
        if (resable(tank))
            return tank;
    }

    ObjectGuid const healer = state->resHealer.Get(bot, [bot, &resable](ObjectGuid& lowest)
    {
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsHeal(member) && resable(member) &&
                (lowest.IsEmpty() || member->GetGUID() < lowest))
                lowest = member->GetGUID();

            return false;
        });
    });

    return healer.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, healer);
}

bool AnubarakWaveImminent(PlayerbotAI* botAI)
{
    AnubarakState const* state = ReadPhase(botAI);
    if (!state || state->phaseValue == AnubarakPhase::Submerged)
        return false;

    int32 const until = MsUntilWave(*state, getMSTime());
    return until <= static_cast<int32>(ANUBARAK_WAVE_READY_MS);
}

// Any taunt aura the tank put on it: its class taunt, Righteous Defense's, Death Grip's
static bool TauntedBy(Unit* unit, Player* tank)
{
    for (AuraEffect const* effect : unit->GetAuraEffectsByType(SPELL_AURA_MOD_TAUNT))
        if (effect->GetCasterGUID() == tank->GetGUID())
            return true;

    return false;
}

AnubarakBurrowerPick GetAnubarakBurrowerPick(PlayerbotAI* botAI)
{
    AnubarakBurrowerPick pick;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    int8 const side = GetAnubarakBurrowerTankSide(bot);
    AnubarakState* state = side >= 0 ? ReadPhase(botAI) : nullptr;
    if (!state)
        return pick;

    pick.side = static_cast<uint8>(side);
    pick.hold = GetAnubarakSideTankHold(bot, pick.side);

    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, *state);
    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    std::unordered_map<Player*, uint8> onMember;
    std::unordered_map<Player*, uint8> allOnMember;
    std::unordered_map<Player*, float> farOnMember;
    uint8 heldCount = 0;
    bool looseOnHealer = false;
    float looseDist = 0.0f;
    for (Creature* burrower : GetAnubarakBurrowers(bot))
    {
        // Submerged: it comes back up where it went down, 10 s later
        if (burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (burrower->GetVictim() == bot)
        {
            ++heldCount;
            bool const taunted = TauntedBy(burrower, bot);
            if (!pick.held || (taunted && !pick.heldTaunted) || (taunted == pick.heldTaunted && burrower == current))
            {
                pick.held = burrower;
                pick.heldTaunted = taunted;
            }

            continue;
        }

        Player* victim = burrower->GetVictim() ? burrower->GetVictim()->ToPlayer() : nullptr;
        if (victim)
            ++allOnMember[victim];

        auto const owner = owners.find(burrower->GetGUID());
        if (owner == owners.end() || owner->second != pick.side)
        {
            if (bot->GetExactDist2d(burrower) <= ANUBARAK_SIDE_SHOUT_RADIUS)
                pick.otherNear = true;

            continue;
        }

        if (victim)
        {
            ++onMember[victim];
            float& farthest = farOnMember[victim];
            farthest = std::max(farthest, bot->GetExactDist2d(burrower));
        }

        if (bot->GetExactDist2d(burrower) <= ANUBARAK_SIDE_SHOUT_RADIUS)
            ++pick.looseNear;

        bool const onHealer = victim && PlayerbotAI::IsHeal(victim);
        float const dist = Dist2d(burrower->GetPosition(), pick.hold);
        if (!pick.loose || (onHealer && !looseOnHealer) || (onHealer == looseOnHealer && dist < looseDist))
        {
            pick.loose = burrower;
            looseOnHealer = onHealer;
            looseDist = dist;
        }
    }

    // Righteous Defense taunts all three attackers on the member, so never a side tank or one that puts this tank
    // past its two, the other side's counted too, except under RESCUE_PCT outside the swarm so a dying member gets it
    bool const swarm = state->phaseValue == AnubarakPhase::Swarm;
    uint8 most = 0;
    for (auto const& [member, count] : onMember)
    {
        bool const rescue = !swarm && member->GetHealthPct() < ANUBARAK_TAUNT_RESCUE_PCT;
        uint8 const taken = rescue ? count : allOnMember[member];
        if (GetAnubarakBurrowerTankSide(member) >= 0 || heldCount + taken > ANUBARAK_BURROWERS_PER_SIDE)
            continue;

        if (count > most || (count == most && pick.rescue && member->GetGUID() < pick.rescue->GetGUID()))
        {
            pick.rescue = member;
            pick.rescueFar = farOnMember[member];
            most = count;
        }
    }

    return pick;
}

bool GetAnubarakBurrowerLead(PlayerbotAI* botAI, AnubarakBurrowerLead& out)
{
    out = AnubarakBurrowerLead();
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !bot->IsAlive() || IsAnubarakTankPlayer(bot) ||
        (!PlayerbotAI::IsHeal(bot) && !PlayerbotAI::IsRangedDps(bot)) || !AnubarakEngaged(botAI))
        return false;

    AnubarakState* state = ReadPhase(botAI);
    // the gather keeps everyone off the spike's run, and a bot walking off it is a bystander the kite must miss
    if (!state || bot->HasAura(SPELL_MARK) || IsAnubarakEscort(botAI) || AnubarakGathers(botAI) ||
        GetAnubarakDutySphere(bot) || GetAnubarakBattleResTarget(botAI))
        return false;

    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, *state);
    float best = 0.0f;
    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
    {
        if (burrower->GetVictim() != bot || burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        auto const owner = owners.find(burrower->GetGUID());
        Player* tank = owner != owners.end() ? UsableSideTank(bot, *state, owner->second) : nullptr;
        float const dist = tank ? bot->GetExactDist2d(tank) : 0.0f;
        if (tank && (!out.tank || dist < best))
        {
            out.tank = tank;
            best = dist;
        }
    }

    if (!out.tank)
        return false;

    out.walk = best > ANUBARAK_LEAD_REACH;
    float const share = ANUBARAK_LEAD_STOP / std::max(best, 0.1f);
    out.to = Position(out.tank->GetPositionX() + (bot->GetPositionX() - out.tank->GetPositionX()) * share,
                      out.tank->GetPositionY() + (bot->GetPositionY() - out.tank->GetPositionY()) * share,
                      bot->GetPositionZ());
    return true;
}

bool AnubarakSideTankBossCapped(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state || GetAnubarakBurrowerTankSide(bot) < 0)
        return false;

    ThreatCapLatch& cap = state->sideBossCaps[bot->GetGUID()];
    uint32 const now = getMSTime();
    if (cap.memoValid && cap.memoMs == now)
        return cap.value;

    cap.memoValid = true;
    cap.memoMs = now;
    cap.value = false;
    Creature* boss = GetAnubarak(bot);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    if (!victim || victim == bot)
        return false;

    float const own = boss->GetThreatMgr().GetThreat(bot);
    float const held = boss->GetThreatMgr().GetThreat(victim);
    if (!cap.capped && own > 0.0f && own >= ANUBARAK_SIDE_BOSS_CAP * held)
        cap.capped = true;
    else if (cap.capped && own < ANUBARAK_SIDE_BOSS_RELEASE * held)
        cap.capped = false;

    cap.value = cap.capped;
    return cap.value;
}

enum class SideAoeShape : uint8
{
    // Round the caster
    Caster,
    // Round a ground point
    Point,
    // Round the target, Pestilence spreading the diseases on it
    Disease,
    // Bounces on from the last hit to the closest within the jump, any direction
    Bounce,
    // The closest to the caster within the jump, in front of it
    CasterChain,
};

struct SideAoeSpell
{
    char const* name;
    SideAoeShape shape;
    // The area's radius when the spell gives none, else the chain's jump
    float radius;
    uint8 targets;
    // Lasts 8-10 s, so what walks in later counts too
    bool ground;
};

// Ground areas first, the most threat. Melee chains jump 10 yd, ranged ones 7.5.
static constexpr SideAoeSpell SIDE_AOE_SPELLS[] = {
    {"death and decay", SideAoeShape::Point, 10.0f, 0, true},
    {"consecration", SideAoeShape::Caster, 8.0f, 0, true},
    {"blood boil", SideAoeShape::Caster, 10.0f, 0, false},
    {"thunder clap", SideAoeShape::Caster, 8.0f, 0, false},
    {"swipe (bear)", SideAoeShape::Caster, 8.0f, 0, false},
    {"holy wrath", SideAoeShape::Caster, 10.0f, 0, false},
    {"hammer of the righteous", SideAoeShape::Bounce, 10.0f, 3, false},
    {"heart strike", SideAoeShape::CasterChain, 10.0f, 2, false},
    {"cleave", SideAoeShape::CasterChain, 10.0f, 2, false},
    {"avenger's shield", SideAoeShape::Bounce, 7.5f, 3, false},
    {"pestilence", SideAoeShape::Disease, 10.0f, 0, false},
};

// What a ground area may still take in over its 8-10 s
constexpr float ANUBARAK_SIDE_AOE_GROUND_MARGIN = 3.0f;
// Own burrowers this close count toward the two the cast needs
constexpr float ANUBARAK_SIDE_AOE_NEAR = 15.0f;
// Death and Decay goes down this far off the own pair's midpoint, away from him
constexpr float ANUBARAK_SIDE_AOE_SHIFTS[] = {0.0f, 2.0f, 4.0f, 6.0f};
constexpr float ANUBARAK_SIDE_AOE_RANGED = 30.0f;
// Any other area spell holds with one of the other side's burrowers this close to the bot or its target
constexpr float ANUBARAK_SIDE_AOE_OTHER_REACH = 10.0f;
// A ground area only with the side tank this close to its hold. Cast at the pickup, half of Death and
// Decay's ticks hit nothing while the burrowers followed the tank 17-23 yd to its patch.
constexpr float ANUBARAK_SIDE_AOE_HOLD_REACH = 3.0f;

bool AtSideTankHold(Player* bot, uint8 side)
{
    return Dist2d(bot->GetPosition(), GetAnubarakSideTankHold(bot, side)) <= ANUBARAK_SIDE_AOE_HOLD_REACH;
}

enum class SideAoeKind : uint8
{
    Own,
    Other,
    Boss,
    Scarab,
};

struct SideAoeUnit
{
    Unit* unit;
    SideAoeKind kind;
};

struct SideAoeHits
{
    uint8 own = 0;
    bool other = false;
    bool boss = false;
};

enum class SideAoeVerdict : uint8
{
    Pass,
    // Fewer than two of its own
    Own,
    Other,
    Cap,
};

// Every selectable burrower, scarab and him, sorted by whose they are
static std::vector<SideAoeUnit> SideAoeUnits(Player* bot, AnubarakState& state, uint8 side)
{
    std::vector<SideAoeUnit> units;
    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, state);
    for (Creature* burrower : ResolveAlive(bot, state.burrowers))
    {
        if (burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        auto const owner = owners.find(burrower->GetGUID());
        units.push_back(
            {burrower, owner != owners.end() && owner->second == side ? SideAoeKind::Own : SideAoeKind::Other});
    }

    for (Creature* scarab : ResolveAlive(bot, state.scarabs))
        units.push_back({scarab, SideAoeKind::Scarab});

    Creature* boss = GetAnubarak(bot);
    if (boss && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        units.push_back({boss, SideAoeKind::Boss});

    return units;
}

// The core takes a creature into a player's area once its centre is within the radius plus its combat
// reach, so he is inside any area centred within 20 yd of him
static bool InsideArea(Position const& centre, Unit* unit, float radius)
{
    return Dist2d(centre, unit->GetPosition()) <= radius + unit->GetCombatReach();
}

static void Tally(SideAoeHits& hits, SideAoeKind kind)
{
    hits.own += kind == SideAoeKind::Own ? 1 : 0;
    hits.other = hits.other || kind == SideAoeKind::Other;
    hits.boss = hits.boss || kind == SideAoeKind::Boss;
}

static SideAoeHits AreaHits(std::vector<SideAoeUnit> const& units, Position const& centre, float radius, bool ground)
{
    float const margin = ground ? ANUBARAK_SIDE_AOE_GROUND_MARGIN : 0.0f;
    SideAoeHits hits;
    for (SideAoeUnit const& unit : units)
        if (InsideArea(centre, unit.unit, unit.kind == SideAoeKind::Own ? radius : radius + margin))
            Tally(hits, unit.kind);

    return hits;
}

static SideAoeHits ChainHits(Player* bot, std::vector<SideAoeUnit> const& units, Unit* first, float jump,
                             uint8 targets, bool fromCaster)
{
    std::vector<Unit*> hit = {first};
    SideAoeHits hits;
    for (SideAoeUnit const& unit : units)
        if (unit.unit == first)
            Tally(hits, unit.kind);

    while (hit.size() < targets)
    {
        Unit* from = fromCaster ? static_cast<Unit*>(bot) : hit.back();
        SideAoeUnit const* next = nullptr;
        float best = 0.0f;
        for (SideAoeUnit const& unit : units)
        {
            if (std::find(hit.begin(), hit.end(), unit.unit) != hit.end() ||
                (fromCaster && !bot->HasInArc(static_cast<float>(M_PI), unit.unit)))
                continue;

            float const dist = Dist2d(from->GetPosition(), unit.unit->GetPosition());
            if (dist > jump + from->GetCombatReach() + unit.unit->GetCombatReach() || (next && dist >= best))
                continue;

            next = &unit;
            best = dist;
        }

        if (!next)
            break;

        hit.push_back(next->unit);
        Tally(hits, next->kind);
    }

    return hits;
}

static SideAoeVerdict JudgeHits(SideAoeHits const& hits, bool capped, bool needOwn)
{
    if (hits.other)
        return SideAoeVerdict::Other;

    if (needOwn && hits.own < ANUBARAK_BURROWERS_PER_SIDE)
        return SideAoeVerdict::Own;

    return hits.boss && capped ? SideAoeVerdict::Cap : SideAoeVerdict::Pass;
}

static SideAoeSpell const* FindSideAoeSpell(std::string const& name)
{
    for (SideAoeSpell const& spell : SIDE_AOE_SPELLS)
        if (name == spell.name)
            return &spell;

    return nullptr;
}

// The spell's own radius where it has one, so a glyph's reach counts
static float SideAoeRadius(Player* bot, uint32 spellId, SideAoeSpell const& spell)
{
    if (spell.shape == SideAoeShape::Bounce || spell.shape == SideAoeShape::CasterChain)
        return spell.radius;

    float radius = 0.0f;
    if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        for (SpellEffectInfo const& effect : info->Effects)
            if (effect.HasRadius())
                radius = std::max(radius, effect.CalcRadius(bot));

    return radius > 0.0f ? radius : spell.radius;
}

static bool Diseased(PlayerbotAI* botAI, Unit* target)
{
    return botAI->GetAura("blood plague", target, true) || botAI->GetAura("frost fever", target, true);
}

// What the cast hits aimed at target, Death and Decay at centre
static SideAoeHits SideAoeCastHits(PlayerbotAI* botAI, SideAoeSpell const& spell, uint32 spellId, Unit* target,
                                   Position const& centre, std::vector<SideAoeUnit> const& units)
{
    Player* bot = botAI->GetBot();
    float const radius = SideAoeRadius(bot, spellId, spell);
    switch (spell.shape)
    {
        case SideAoeShape::Caster:
            return AreaHits(units, bot->GetPosition(), radius, spell.ground);
        case SideAoeShape::Point:
            return AreaHits(units, centre, radius, spell.ground);
        case SideAoeShape::Disease:
            return AreaHits(units, target->GetPosition(), radius, false);
        case SideAoeShape::Bounce:
            return ChainHits(bot, units, target, radius, spell.targets, false);
        default:
            return ChainHits(bot, units, target, radius, spell.targets, true);
    }
}

static bool SideAoeInReach(Player* bot, SideAoeSpell const& spell, Unit* target, Position const& centre)
{
    switch (spell.shape)
    {
        case SideAoeShape::Caster:
            return true;
        case SideAoeShape::Point:
            return Dist2d(bot->GetPosition(), centre) <= ANUBARAK_SIDE_AOE_RANGED;
        default:
            return spell.radius < 10.0f ? bot->IsWithinCombatRange(target, ANUBARAK_SIDE_AOE_RANGED) &&
                                              bot->IsWithinLOSInMap(target)
                                        : bot->IsWithinMeleeRange(target);
    }
}

// CanCastSpell's probe skips power, so a spell short a rune passes there and fails at the cast. Death runes
// cover any type.
static bool SideAoeRunesReady(Player* bot, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    SpellRuneCostEntry const* cost =
        info && info->PowerType == POWER_RUNE ? sSpellRuneCostStore.LookupEntry(info->RuneCostID) : nullptr;
    if (!cost || cost->NoRuneCost())
        return true;

    // indexed by RuneType, the way Spell::CheckRuneCost reads it
    int32 need[RUNE_DEATH];
    for (uint8 type = 0; type < RUNE_DEATH; ++type)
        need[type] = static_cast<int32>(cost->RuneCost[type]);

    int32 death = 0;
    for (uint8 i = 0; i < MAX_RUNES; ++i)
    {
        if (bot->GetRuneCooldown(i))
            continue;

        RuneType const rune = bot->GetCurrentRune(i);
        if (rune == RUNE_DEATH)
            ++death;
        else if (rune < RUNE_DEATH && need[rune] > 0)
            --need[rune];
    }

    int32 shortBy = 0;
    for (int32 left : need)
        shortBy += std::max(left, 0);

    return shortBy <= death;
}

bool GetAnubarakSideTankAoe(PlayerbotAI* botAI, AnubarakSideTankAoe& out)
{
    out = AnubarakSideTankAoe();
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    int8 const side = bot ? GetAnubarakBurrowerTankSide(bot) : -1;
    AnubarakState* state = side >= 0 ? ReadPhase(botAI) : nullptr;
    if (!state || (state->phaseValue != AnubarakPhase::Surface && state->phaseValue != AnubarakPhase::Swarm) ||
        bot->HasAura(SPELL_MARK))
        return false;

    std::vector<SideAoeUnit> const units = SideAoeUnits(bot, *state, static_cast<uint8>(side));
    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    std::vector<Unit*> own;
    for (SideAoeUnit const& unit : units)
        if (unit.kind == SideAoeKind::Own && bot->GetExactDist2d(unit.unit) <= ANUBARAK_SIDE_AOE_NEAR)
            own.push_back(unit.unit);

    std::sort(own.begin(), own.end(), [bot, current](Unit* a, Unit* b)
              {
                  if ((a == current) != (b == current))
                      return a == current;
                  return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
              });

    if (own.size() >= ANUBARAK_BURROWERS_PER_SIDE)
    {
        bool const capped = AnubarakSideTankBossCapped(bot);
        Creature* boss = GetAnubarak(bot);
        Position const mid((own[0]->GetPositionX() + own[1]->GetPositionX()) / 2.0f,
                           (own[0]->GetPositionY() + own[1]->GetPositionY()) / 2.0f, own[0]->GetPositionZ());
        Vec2 away = boss ? Delta(boss->GetPosition(), mid) : Vec2{0.0f, 0.0f};
        float const length = Length(away);
        away = length > 0.01f ? Vec2{away.x / length, away.y / length} : Vec2{0.0f, 0.0f};

        bool const atHold = AtSideTankHold(bot, static_cast<uint8>(side));
        for (SideAoeSpell const& spell : SIDE_AOE_SPELLS)
        {
            uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell.name)->Get();
            if (!spellId || !bot->HasSpell(spellId))
                continue;

            if (spell.ground && !atHold)
                continue;

            if (!SideAoeRunesReady(bot, spellId))
                continue;

            std::vector<std::pair<Unit*, Position>> tries;
            if (spell.shape == SideAoeShape::Point)
                for (float shift : ANUBARAK_SIDE_AOE_SHIFTS)
                    tries.emplace_back(own[0], Offset(mid, away, shift));
            else if (spell.shape == SideAoeShape::Caster)
                tries.emplace_back(own[0], bot->GetPosition());
            else
                for (Unit* target : own)
                    if (spell.shape != SideAoeShape::Disease || Diseased(botAI, target))
                        tries.emplace_back(target, target->GetPosition());

            for (auto const& [target, centre] : tries)
            {
                if (!SideAoeInReach(bot, spell, target, centre) || !botAI->CanCastSpell(spell.name, target))
                    continue;

                SideAoeVerdict const verdict =
                    JudgeHits(SideAoeCastHits(botAI, spell, spellId, target, centre, units), capped, true);
                if (verdict == SideAoeVerdict::Pass)
                {
                    out.spell = spell.name;
                    out.target = target;
                    out.point = spell.shape == SideAoeShape::Point;
                    out.pos = centre;
                    return true;
                }
            }
        }
    }

    return false;
}

bool AnubarakSideTankAreaAllowed(PlayerbotAI* botAI, std::string const& spellName, Unit* target, bool area)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    int8 const side = bot ? GetAnubarakBurrowerTankSide(bot) : -1;
    AnubarakState* state = side >= 0 ? ReadPhase(botAI) : nullptr;
    if (!state)
        return true;

    SideAoeSpell const* spell = FindSideAoeSpell(spellName);
    if (!spell && !area)
        return true;

    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
    if (spell && spell->ground &&
        (!AtSideTankHold(bot, static_cast<uint8>(side)) || !SideAoeRunesReady(bot, spellId)))
        return false;

    std::vector<SideAoeUnit> const units = SideAoeUnits(bot, *state, static_cast<uint8>(side));
    Unit* aimed = target && target != bot ? target : botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!spell)
        return std::none_of(units.begin(), units.end(), [&](SideAoeUnit const& unit)
                            {
                                return unit.kind == SideAoeKind::Other &&
                                       (InsideArea(bot->GetPosition(), unit.unit, ANUBARAK_SIDE_AOE_OTHER_REACH) ||
                                        (aimed &&
                                         InsideArea(aimed->GetPosition(), unit.unit, ANUBARAK_SIDE_AOE_OTHER_REACH)));
                            });

    if (!aimed && spell->shape != SideAoeShape::Caster)
        return true;

    if (spell->shape == SideAoeShape::Disease && !Diseased(botAI, aimed))
        return true;

    Position const centre = aimed ? aimed->GetPosition() : bot->GetPosition();
    // One burrower is enough here: Heart Strike on a lone one is still the side tank's best threat
    return JudgeHits(SideAoeCastHits(botAI, *spell, spellId, aimed ? aimed : bot, centre, units),
                     AnubarakSideTankBossCapped(bot), false) == SideAoeVerdict::Pass;
}

bool AnubarakBurrowerNear(Player* bot, Position const& at, float range, bool waiting)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
    {
        if (burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || Dist2d(burrower->GetPosition(), at) > range)
            continue;

        if (!waiting || (burrower->GetHealthPct() >= ANUBARAK_FOCUS_OFF_FROST_PCT && !BurrowerSettled(bot, *state, burrower)))
            return true;
    }

    return false;
}

bool AnubarakLooseBurrowerReaches(Player* bot, Position const& at, float radius, float seconds)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
        if (!burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
            Dist2d(burrower->GetPosition(), at) <=
                radius + burrower->GetCombatReach() + burrower->GetSpeed(MOVE_RUN) * seconds &&
            !BurrowerSettled(bot, *state, burrower))
            return true;

    return false;
}

Unit* GetAnubarakDpsTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!IsToCDpsBot(bot) || !AnubarakEngaged(botAI))
        return nullptr;

    // Read first: a new pull's first read clears the focus latches
    AnubarakState* state = ReadPhase(botAI);
    if (!state)
        return nullptr;

    // The sphere and grip nodes outrank this one, so with a pick here the bot would swap targets every
    // tick. A DK waiting on its spot keeps hitting him from there.
    if (PlayerbotAI::IsRangedDps(bot) && GetAnubarakSphereToShoot(bot))
        return nullptr;

    AnubarakGripDuty grip;
    if (GetAnubarakGripDuty(bot, grip) && grip.step != AnubarakGripStep::Hold)
        return nullptr;

    // Read even under a skull, so both focus latches stay current. A melee bot that can't hit its side's
    // focus from outside the patches stays on him.
    Creature* const focus[2] = {SideFocus(bot, *state, 0), SideFocus(bot, *state, 1)};
    int8 const side = DpsSide(bot, *state);
    bool const melee = PlayerbotAI::IsMelee(bot);
    Creature* own = side >= 0 ? focus[side] : (focus[0] ? focus[0] : focus[1]);
    Creature* other = side >= 0 ? focus[1 - side] : nullptr;
    if (own && melee && !MeleeReachesBurrower(bot, *state, own))
        own = nullptr;

    // Holding casts alone, white swings, Auto Shot and procs still pulled it off the side tank
    bool const capped = own && AnubarakBurrowerThreatCapped(bot, own);
    if (capped)
        own = nullptr;

    bool const otherCapped = other && !melee && AnubarakBurrowerThreatCapped(bot, other);
    if (otherCapped)
        other = nullptr;

    if (Unit* skull = GetToCHumanSkull(botAI, &IsAnubarakSkullTarget))
        return skull;

    // Gathered melee take only the scarabs and loose burrowers that come to the raid: their side's focus would
    // walk them out and the gather back every few seconds
    bool const gathered = melee && AnubarakGathers(botAI);
    if (gathered)
        if (Creature* scarab = NearestDpsScarab(bot, *state, ANUBARAK_GATHER_MELEE_SCARAB))
            return scarab;

    if (melee)
        if (Creature* loose = LooseBurrowerNear(bot, *state))
            return loose;

    if (gathered)
        return nullptr;

    // A bot capped on him helps more on the burrowers
    if (state->push && state->phaseValue == AnubarakPhase::Surface && !AnubarakThreatCapped(bot))
        if (Creature* him = GetAnubarak(bot); him && !him->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return him;

    // the ranged still take the waves
    if (own && melee && state->phaseValue == AnubarakPhase::Swarm && AnubarakHeroic(bot))
        if (Creature* him = GetAnubarak(bot); him && SwarmBerserkBehind(*state, him, getMSTime()))
            return him;

    if (own)
        return own;

    // A scarab left over from phase 2 goes before him: its Acid-Drenched Mandibles stacks last a minute
    // and every hit refreshes them. Melee take only one close by, so none walks off him across the room.
    bool const surfaced = state->phaseValue == AnubarakPhase::Surface || state->phaseValue == AnubarakPhase::Swarm;
    if (surfaced)
        if (Creature* scarab = melee ? NearestDpsScarab(bot, *state, ANUBARAK_GATHER_MELEE_SCARAB)
                                     : NearestDpsScarab(bot, *state))
            return scarab;

    // Ranged reach the other side's from where they stand; melee never walk over to it
    if (!melee && other)
        return other;

    Creature* boss = GetAnubarak(bot);
    if (boss && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return boss;

    if (Creature* scarab = state->phaseValue == AnubarakPhase::Submerged ? NearestDpsScarab(bot, *state) : nullptr)
        return scarab;

    return nullptr;
}

bool GetAnubarakMeleeBurrowerStand(Player* bot, Creature* burrower, Position& out)
{
    AnubarakState* state = bot && burrower ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return false;

    std::vector<Position> const patches = PatchPositions(bot, *state);
    MeleeRing const ring = MeleeBurrowerRing(bot, *state, burrower);
    if (!BurrowerNearPatch(burrower, patches, ring))
        return false;

    // Held, so the bot walks once per burrower and not after every turn he makes
    auto const held = state->meleeStands.find(bot->GetGUID());
    if (held != state->meleeStands.end() && held->second.burrower == burrower->GetGUID() &&
        Dist2d(burrower->GetPosition(), held->second.at) <= ANUBARAK_MELEE_STAND_DRIFT &&
        Dist2d(burrower->GetPosition(), held->second.stand) <= bot->GetMeleeRange(burrower) &&
        std::none_of(patches.begin(), patches.end(), [&held, &ring](Position const& patch)
                     { return Dist2d(held->second.stand, patch) < ring.clearance; }))
    {
        out = held->second.stand;
        return true;
    }

    if (!MeleeBurrowerStand(bot, burrower, patches, ring, true, out))
        return false;

    state->meleeStands[bot->GetGUID()] = MeleeStandLatch{burrower->GetGUID(), burrower->GetPosition(), out};
    return true;
}

bool AnubarakMeleeBurrowerStandApplies(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!IsToCDpsBot(bot) || !PlayerbotAI::IsMelee(bot) || bot->HasAura(SPELL_MARK) || !AnubarakEngaged(botAI))
        return false;

    AnubarakState const* state = ReadPhase(botAI);
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    Creature* burrower = target ? target->ToCreature() : nullptr;
    if (!state || !burrower || burrower->GetEntry() != static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER))
        return false;

    std::vector<Position> const patches = PatchPositions(bot, *state);
    MeleeRing const ring = MeleeBurrowerRing(bot, *state, burrower);
    Position stand;
    return BurrowerNearPatch(burrower, patches, ring) && MeleeBurrowerStand(bot, burrower, patches, ring, false, stand);
}

bool AnubarakSideTankSwarmLeash(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !AnubarakEngaged(botAI) || GetAnubarakPhase(botAI) != AnubarakPhase::Swarm)
        return false;

    int8 const side = GetAnubarakBurrowerTankSide(bot);
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (side < 0 || !target || target->GetEntry() != static_cast<uint32>(ToCNpcs::NPC_NERUBIAN_BURROWER))
        return false;

    // on us it walks in by itself; walking out to meet it left the healers 40+ yd off
    if (target->GetVictim() == bot)
        return true;

    Position const hold = GetAnubarakSideTankHold(bot, static_cast<uint8>(side));
    return target->GetExactDist2d(hold.GetPositionX(), hold.GetPositionY()) > ANUBARAK_TAUNT_ARRIVE;
}

bool AnubarakSwarmSavesHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !bot->IsAlive() || IsAnubarakTankPlayer(bot) || !AnubarakLeechingSwarmActive(botAI) ||
        bot->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot)))
        return false;

    for (Creature* burrower : GetAnubarakBurrowers(bot))
        if (burrower->GetVictim() == bot)
            return false;

    return true;
}

bool AnubarakMeleeNeedsBurrowerStand(PlayerbotAI* botAI)
{
    if (!AnubarakMeleeBurrowerStandApplies(botAI))
        return false;

    Player* bot = botAI->GetBot();
    Unit* burrower = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    AnubarakState const* state = ReadPhase(botAI);
    std::vector<Position> const patches = PatchPositions(bot, *state);
    float const clearance = MeleeBurrowerRing(bot, *state, burrower).clearance;
    bool const clear = std::none_of(patches.begin(), patches.end(), [bot, clearance](Position const& patch)
                                    { return Dist2d(bot->GetPosition(), patch) < clearance; });
    if (!clear || !bot->IsWithinMeleeRange(burrower))
        return true;

    auto const held = state->meleeStands.find(bot->GetGUID());
    return held != state->meleeStands.end() && held->second.burrower == burrower->GetGUID() &&
           Dist2d(bot->GetPosition(), held->second.stand) > ANUBARAK_MELEE_STAND_ARRIVE;
}

bool AnubarakNonTankOnPermafrost(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->HasAura(SPELL_MARK) || IsAnubarakTankPlayer(bot))
        return false;

    AnubarakState const* state = ReadPhase(GET_PLAYERBOT_AI(bot));
    if (!state)
        return false;

    // The gripped sphere comes down on the DK, which leaves once it is killing it
    AnubarakGripDuty grip;
    if (GetAnubarakGripDuty(bot, grip) && grip.step != AnubarakGripStep::Kill)
        return false;

    if (UnitOnPermafrost(bot))
        return true;

    std::vector<Position> const patches = PatchPositions(bot, *state);
    return std::any_of(patches.begin(), patches.end(), [bot](Position const& patch)
                       { return Dist2d(bot->GetPosition(), patch) < ANUBARAK_PERMAFROST_SLOW_REACH; });
}

bool GetAnubarakPermafrostStepOut(Player* bot, Position& out)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    AnubarakState const* state = botAI && AnubarakNonTankOnPermafrost(bot) ? ReadPhase(botAI) : nullptr;
    if (!state)
        return false;

    std::vector<HazardCircle> hazards;
    for (Position const& patch : PatchPositions(bot, *state))
        hazards.emplace_back(patch, ANUBARAK_TANK_KEEPOUT);

    // Off the dodge's own danger, so the step out never lands where the dodge would walk it off again
    auto const offSpike = [bot](float x, float y)
    { return !AnubarakSpikeDangerAt(bot, Position(x, y, bot->GetPositionZ())); };

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    float const reach = !target                     ? 0.0f
                        : PlayerbotAI::IsMelee(bot) ? bot->GetMeleeRange(target) - 0.5f
                                                    : botAI->GetRange("spell") - ANUBARAK_SHOOT_MARGIN;
    auto const inReach = [&](float x, float y)
    { return !target || target->GetExactDist2d(x, y) <= reach; };

    Position const prefer = target ? target->GetPosition() : bot->GetPosition();
    HazardSweepCache cache;
    for (int pass = 0; pass < 3; ++pass)
    {
        std::function<bool(float, float)> accept;
        if (pass == 0)
            accept = [&](float x, float y) { return offSpike(x, y) && inReach(x, y); };
        else if (pass == 1)
            accept = offSpike;

        Position const spot = FindNearestPositionClearOfHazards(bot, hazards, ANUBARAK_TANK_STEP_SEARCH, 1.5f,
                                                                static_cast<float>(M_PI) / 8.0f, &prefer, accept,
                                                                &cache);
        if (spot == Position())
            continue;

        out = spot;
        return true;
    }

    return false;
}

Position GetAnubarakPatchSafeLeg(Player* bot, Position const& to)
{
    AnubarakState const* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return to;

    // The first patch the walk would cut, nearest the bot
    Position const from = bot->GetPosition();
    Position const* cut = nullptr;
    std::vector<Position> const patches = PatchPositions(bot, *state);
    for (Position const& patch : patches)
        if (PathClips(from, to, patch) && (!cut || Dist2d(from, patch) < Dist2d(from, *cut)))
            cut = &patch;

    return cut ? DetourWaypoint(bot, *cut, to) : to;
}

Creature* GetAnubarakScarabToPickUp(Player* bot)
{
    AnubarakState const* state = Sweep(bot);
    if (!state)
        return nullptr;

    bool const heroic = AnubarakHeroic(bot);
    bool const ownOnly = heroic && state->phaseValue == AnubarakPhase::Submerged && IsAnubarakPickupTank(bot);
    bool const healersOnly = heroic && GetAnubarakBurrowerTankSide(bot) >= 0;
    Creature* nearest = nullptr;
    float nearestDist = ANUBARAK_SCARAB_PICKUP_RANGE;
    for (Creature* scarab : ResolveAlive(bot, state->scarabs))
    {
        // The ten roaming at the pre-pull are the same entry, non-attackable and hitting nobody
        if (scarab->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || scarab->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            continue;

        Unit* victim = scarab->GetVictim();
        if (!victim || (victim == bot) != ownOnly)
            continue;

        Player* victimPlayer = victim->ToPlayer();
        if (!ownOnly && ((victimPlayer && IsAnubarakTankPlayer(victimPlayer)) ||
                         (healersOnly && !(victimPlayer && PlayerbotAI::IsHeal(victimPlayer)))))
            continue;

        float const dist = bot->GetExactDist2d(scarab);
        if (dist <= nearestDist)
        {
            nearest = scarab;
            nearestDist = dist;
        }
    }

    return nearest;
}

// 0.1 s steps along the walk, the scarab heading for where the bot is each step: it never gets within
// floorGap
static bool OutrunsScarab(Position const& from, Position const& to, float botSpeed, Position const& scarabAt,
                          float scarabSpeed, float floorGap)
{
    float const length = Dist2d(from, to);
    if (botSpeed <= 0.0f || length < 0.1f)
        return false;

    constexpr float STEP_S = 0.1f;
    float sx = scarabAt.GetPositionX();
    float sy = scarabAt.GetPositionY();
    int const steps = static_cast<int>(std::ceil(length / botSpeed / STEP_S));
    for (int k = 1; k <= steps; ++k)
    {
        float const share = std::min(1.0f, static_cast<float>(k) * STEP_S * botSpeed / length);
        float const bx = from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * share;
        float const by = from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * share;
        float const gap = std::hypot(bx - sx, by - sy);
        float const move = std::min(gap, scarabSpeed * STEP_S);
        if (gap > 0.0f)
        {
            sx += (bx - sx) / gap * move;
            sy += (by - sy) / gap * move;
        }

        if (std::hypot(bx - sx, by - sy) < floorGap)
            return false;
    }

    return true;
}

// The gather while he's down, else his tank
static Position ScarabKiteMeleePoint(Player* bot, AnubarakState const& state)
{
    Position point = bot->GetPosition();
    if (state.phaseValue == AnubarakPhase::Submerged && GatherCentre(bot, state, point))
        return point;

    Creature* boss = GetAnubarak(bot);
    Unit* tank = boss && state.phaseValue != AnubarakPhase::Submerged ? boss->GetVictim() : nullptr;
    if (!tank)
        tank = ObjectAccessor::GetPlayer(*bot, state.mainTank);

    return tank && tank->IsAlive() ? tank->GetPosition() : point;
}

static bool ScarabKiteStep(Player* bot, AnubarakState const& state, Creature* scarab, Position& out)
{
    Position const from = bot->GetPosition();
    Position const melee = ScarabKiteMeleePoint(bot, state);
    std::vector<Position> const patches = PatchPositions(bot, state);
    AliveCreatures const burrowers = ResolveAlive(bot, state.burrowers);
    float const start = bot->GetExactDist2d(scarab);
    float const floorGap = std::min(start, ANUBARAK_SCARAB_MELEE_REACH) - ANUBARAK_SCARAB_KITE_SLACK;
    bool found = false;
    float best = 0.0f;
    for (int i = 0; i < ANUBARAK_SCARAB_KITE_HEADINGS; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / ANUBARAK_SCARAB_KITE_HEADINGS;
        Position const raw = Offset(from, {std::cos(angle), std::sin(angle)}, ANUBARAK_SCARAB_KITE_STEP);
        Position to;
        if (Dist2d(raw, ANUBARAK_ROOM_CENTER) > ANUBARAK_REFILL_FLOOR || !OnFloor(bot, raw, to) ||
            scarab->GetExactDist2d(to.GetPositionX(), to.GetPositionY()) <= start)
            continue;

        bool const nearPatch = std::any_of(patches.begin(), patches.end(), [&](Position const& patch)
                                           { return SegmentDistance2d(from, to, patch) < ANUBARAK_TANK_KEEPOUT; });
        bool const nearBurrower = std::any_of(burrowers.begin(), burrowers.end(), [&](Creature* burrower)
        {
            return !burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
                   SegmentDistance2d(from, to, burrower->GetPosition()) < ANUBARAK_SCARAB_KITE_BURROWER_CLEARANCE;
        });
        if (nearPatch || nearBurrower || AnubarakSpikeDangerAt(bot, to) || !AnubarakWalkClearOfSpike(bot, to) ||
            !OutrunsScarab(from, to, bot->GetSpeed(MOVE_RUN), scarab->GetPosition(), scarab->GetSpeed(MOVE_RUN),
                           floorGap))
            continue;

        float const toMelee = Dist2d(to, melee);
        if (!found || toMelee < best)
        {
            out = to;
            best = toMelee;
            found = true;
        }
    }

    return found;
}

bool GetAnubarakScarabKite(PlayerbotAI* botAI, Position& out)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !bot->IsAlive() || IsAnubarakTankPlayer(bot) || !AnubarakEngaged(botAI))
        return false;

    AnubarakState* state = ReadPhase(botAI);
    if (!state)
        return false;

    Creature* scarab = nullptr;
    float gap = ANUBARAK_SCARAB_KITE_REACH;
    for (Creature* candidate : ResolveAlive(bot, state->scarabs))
    {
        float const dist = bot->GetExactDist2d(candidate);
        if (candidate->GetVictim() == bot && !candidate->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && dist <= gap)
        {
            scarab = candidate;
            gap = dist;
        }
    }

    if (!scarab)
        return false;

    AnubarakGripDuty grip;
    if (bot->HasAura(SPELL_MARK) || IsAnubarakEscort(botAI) || GetAnubarakDutySphere(bot) ||
        GetAnubarakGripDuty(bot, grip) || IsAnubarakDodgeHeld(botAI) || scarab->HasAura(SPELL_SCARAB_DETERMINATION))
        return false;

    return ScarabKiteStep(bot, *state, scarab, out);
}

// Any HoT or shield, whoever cast it
static bool HealCovered(Unit* member)
{
    return !member->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL).empty() ||
           !member->GetAuraEffectsByType(SPELL_AURA_SCHOOL_ABSORB).empty();
}

// What keeps healing a carrier between casts. Paladins have none to spare: Sacred Shield stays on the
// tank.
static char const* const COLD_CARRIER_COVERS[] = {"power word: shield", "renew", "rejuvenation", "lifebloom",
                                                  "riptide"};

static char const* ColdCarrierCover(PlayerbotAI* botAI, Player* member)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() == CLASS_PALADIN || HealCovered(member) ||
        !member->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, member)))
        return nullptr;

    for (char const* cover : COLD_CARRIER_COVERS)
    {
        if (std::strcmp(cover, "power word: shield") == 0 && member->HasAura(SPELL_WEAKENED_SOUL))
            continue;

        if (botAI->CanCastSpell(cover, member))
            return cover;
    }

    return nullptr;
}

bool CastAnubarakColdCarrierCover(PlayerbotAI* botAI, Player* member)
{
    char const* cover = botAI && member ? ColdCarrierCover(botAI, member) : nullptr;
    return cover && botAI->CastSpell(cover, member);
}

static char const* const SWARM_HOTS[] = {"riptide", "renew", "rejuvenation"};

// For a non-tank the swarm has dragged under ANUBARAK_SWARM_HOT_HP: never the healer itself, never on top
// of another HoT
static char const* SwarmHot(PlayerbotAI* botAI, Player* member)
{
    Player* bot = botAI->GetBot();
    if (member == bot || bot->getClass() == CLASS_PALADIN || IsAnubarakTankPlayer(member) ||
        member->GetHealth() >= ANUBARAK_SWARM_HOT_HP ||
        member->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, member)) ||
        !member->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL).empty())
        return nullptr;

    for (char const* hot : SWARM_HOTS)
        if (botAI->CanCastSpell(hot, member))
            return hot;

    return nullptr;
}

bool CastAnubarakSwarmHot(PlayerbotAI* botAI, Player* member)
{
    char const* hot = botAI && member ? SwarmHot(botAI, member) : nullptr;
    return hot && botAI->CastSpell(hot, member);
}

// Living Penetrating Cold carriers in group order
static std::vector<ObjectGuid> const& ColdCarriers(Player* bot, AnubarakState const& state)
{
    return state.coldCarriers.Get(bot, [bot](std::vector<ObjectGuid>& carriers)
    {
        uint32 const cold = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (member->IsAlive() && member->HasAura(cold))
                carriers.push_back(member->GetGUID());

            return false;
        });
    });
}

// Tanks by role or spec in group order, dead ones too
static std::vector<ObjectGuid> const& TankPlayers(Player* bot, AnubarakState const& state)
{
    return state.tankPlayers.Get(bot, [bot](std::vector<ObjectGuid>& tanks)
    {
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (IsAnubarakTankPlayer(member))
                tanks.push_back(member->GetGUID());

            return false;
        });
    });
}

Player* GetAnubarakPenetratingColdHealTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || AnubarakSwarmHealing(botAI))
        return nullptr;

    AnubarakState const* state = AnubarakStateOf(bot);
    if (!state)
        return nullptr;

    uint32 const cold = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
    float const range = botAI->GetRange("heal");

    Player* lowest = nullptr;
    for (ObjectGuid const& guid : ColdCarriers(bot, *state))
    {
        Player* member = ObjectAccessor::GetPlayer(*bot, guid);
        if (!member || !member->IsInWorld() || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!member->HasAura(cold) ||
            (member->GetHealthPct() >= ANUBARAK_PENETRATING_COLD_HEAL_PCT && !ColdCarrierCover(botAI, member)))
            continue;

        if (!bot->IsWithinDistInMap(member, range) || !bot->IsWithinLOSInMap(member))
            continue;

        if (!lowest || member->GetHealthPct() < lowest->GetHealthPct())
            lowest = member;
    }

    return lowest;
}

constexpr uint32 SPELL_ANTI_MAGIC_SHELL = 48707;

// A carrier's next Penetrating Cold tick: what it deals before resists, and the health plus frost
// absorbs the carrier has left when it lands
struct ColdTick
{
    bool due = false;
    int32 inMs = 0;
    int32 damage = 0;
    int64 left = 0;
};

// The swarm ticks before it take their share of current health, absorbs soaking it first. No tick left
// before the aura runs out means not due.
static ColdTick NextColdTick(Player* member)
{
    ColdTick tick;
    Aura* cold = member->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, member));
    AuraEffect* effect = cold ? cold->GetEffect(EFFECT_0) : nullptr;
    if (!effect || effect->GetAuraType() != SPELL_AURA_PERIODIC_DAMAGE ||
        (cold->GetDuration() >= 0 && cold->GetDuration() < effect->GetPeriodicTimer()))
        return tick;

    tick.due = true;
    tick.inMs = std::max(0, effect->GetPeriodicTimer());
    // Divine Shield, Pain Suppression and the like already cover it
    float const taken =
        member->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, SPELL_SCHOOL_MASK_FROST);
    tick.damage = member->IsImmunedToDamage(SPELL_SCHOOL_MASK_FROST)
                      ? 0
                      : static_cast<int32>(static_cast<float>(effect->GetAmount()) * taken);

    int64 health = member->GetHealth();
    int64 absorb = 0;
    // Anti-Magic Shell takes only its share of each hit, up to what it has left
    int64 shell = 0;
    int64 shellPct = 0;
    for (AuraEffect const* shield : member->GetAuraEffectsByType(SPELL_AURA_SCHOOL_ABSORB))
    {
        if (!(shield->GetMiscValue() & SPELL_SCHOOL_MASK_FROST))
            continue;

        if (shield->GetId() != SPELL_ANTI_MAGIC_SHELL)
        {
            absorb += std::max(0, shield->GetAmount());
            continue;
        }

        // 5 s, so it can run out before the tick
        int32 const shellLeftMs = shield->GetBase()->GetDuration();
        if (shellLeftMs >= 0 && shellLeftMs < tick.inMs)
            continue;

        shell += std::max(0, shield->GetAmount());
        shellPct = shield->GetSpellInfo()->Effects[EFFECT_0].CalcValue();
    }

    auto const soak = [&absorb, &shell, shellPct](int64 hit)
    {
        int64 const plain = std::min(absorb, hit);
        absorb -= plain;
        int64 const shelled = std::min(shell, (hit - plain) * shellPct / 100);
        shell -= shelled;
        return hit - plain - shelled;
    };

    AuraEffect const* swarm =
        member->GetAuraEffect(sSpellMgr->GetSpellIdForDifficulty(SPELL_LEECHING_SWARM, member), EFFECT_0);
    if (swarm && swarm->GetAmplitude() > 0)
    {
        for (int32 at = swarm->GetPeriodicTimer(); at < tick.inMs && health > 0; at += swarm->GetAmplitude())
        {
            int64 const drain = std::max<int64>(ANUBARAK_SWARM_MIN_DRAIN, health * swarm->GetAmount() / 100);
            health = std::max<int64>(0, health - soak(drain));
        }
    }

    tick.left = health + absorb + std::min(shell, std::max<int64>(0, tick.damage - absorb) * shellPct / 100);
    return tick;
}

constexpr uint32 SPELL_CLOAK_OF_SHADOWS = 31224;
constexpr uint32 SPELL_DISPERSION = 47585;
constexpr uint32 SPELL_DIVINE_PROTECTION = 498;
constexpr uint32 SPELL_SHAMANISTIC_RAGE = 30823;
constexpr uint32 SPELL_BARKSKIN = 22812;

// Cloak, Ice Block and Divine Shield remove it; the rest cut it, Anti-Magic Shell the swarm too
constexpr std::array<uint32, 8> COLD_SELF_SAVES = {
    SPELL_CLOAK_OF_SHADOWS, SPELL_ICE_BLOCK,         SPELL_DIVINE_SHIELD,    SPELL_ANTI_MAGIC_SHELL,
    SPELL_DISPERSION,       SPELL_DIVINE_PROTECTION, SPELL_SHAMANISTIC_RAGE, SPELL_BARKSKIN};

bool IsAnubarakColdBubble(uint32 spellId) { return spellId == SPELL_ICE_BLOCK || spellId == SPELL_DIVINE_SHIELD; }

bool AnubarakColdBubbleUp(Player* bot)
{
    return bot && (bot->HasAura(SPELL_ICE_BLOCK) || bot->HasAura(SPELL_DIVINE_SHIELD));
}

uint32 GetAnubarakColdSelfSave(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || PlayerbotAI::IsHeal(bot) || !AnubarakEngaged(botAI) ||
        GetAnubarakPhase(botAI) != AnubarakPhase::Swarm || !NextColdTick(bot).due)
        return 0;

    for (uint32 const save : COLD_SELF_SAVES)
        if (bot->HasAura(save))
            return 0;

    // A bubble halves a tank's threat, and the burrowers leave a tank immune to them unless it taunts them
    bool const tank = PlayerbotAI::IsTank(bot);
    for (uint32 const save : COLD_SELF_SAVES)
    {
        if (tank && save != SPELL_ANTI_MAGIC_SHELL && save != SPELL_DIVINE_PROTECTION)
            continue;

        // The exclude auras CanCastSpell's probe doesn't stop
        if ((save == SPELL_DIVINE_SHIELD || save == SPELL_DIVINE_PROTECTION) && bot->HasAura(SPELL_FORBEARANCE))
            continue;
        if (save == SPELL_ICE_BLOCK && bot->HasAura(SPELL_HYPOTHERMIA))
            continue;

        if (botAI->CanCastSpell(save, bot))
            return save;
    }

    return 0;
}

static bool ColdTickDeadly(ColdTick const& tick)
{
    return tick.due && tick.left - tick.damage <= ANUBARAK_SWARM_MIN_DRAIN;
}

static AnubarakHealDuty ClassHealDuty(Player* healer)
{
    switch (healer->getClass())
    {
        case CLASS_PRIEST:
        case CLASS_DRUID:
            return AnubarakHealDuty::Cold;
        default:
            return AnubarakHealDuty::Tank;
    }
}

static AnubarakHealDuty DutyOf(AnubarakState const& state, ObjectGuid const& healer)
{
    auto const duty = state.healDuty.find(healer);
    return duty != state.healDuty.end() ? duty->second : AnubarakHealDuty::None;
}

// With two healers or more and every one on the same side, the lowest guid goes over
static void SetHealDuties(AnubarakState& state, std::vector<Player*> healers)
{
    std::sort(healers.begin(), healers.end(),
              [](Player* lhs, Player* rhs) { return lhs->GetGUID() < rhs->GetGUID(); });
    size_t const cold = static_cast<size_t>(std::count_if(
        healers.begin(), healers.end(),
        [](Player* healer) { return ClassHealDuty(healer) == AnubarakHealDuty::Cold; }));

    bool const oneSided = healers.size() >= 2 && (cold == 0 || cold == healers.size());
    for (Player* healer : healers)
    {
        AnubarakHealDuty duty = ClassHealDuty(healer);
        if (oneSided && healer == healers.front())
            duty = duty == AnubarakHealDuty::Cold ? AnubarakHealDuty::Tank : AnubarakHealDuty::Cold;

        state.healDuty[healer->GetGUID()] = duty;
    }

    state.healDutySet = true;
}

static bool HealerReaches(Player* healer, Player* member)
{
    if (healer == member)
        return true;

    PlayerbotAI* healerAI = GET_PLAYERBOT_AI(healer);
    return healerAI && healer->IsWithinDistInMap(member, healerAI->GetRange("heal")) &&
           healer->IsWithinLOSInMap(member);
}

struct ColdCarrierAtRisk
{
    Player* member;
    ColdTick tick;
    bool coming;
};

// Who the casts and channels in progress are aimed at, the healer's own left out unless asked. Instants
// have landed already
static std::vector<ObjectGuid> HealsOnTheWay(Player* healer, AnubarakState const& state, bool own = false)
{
    std::vector<HealOnTheWay> const& heals =
        state.healsOnTheWay.Get(healer, [healer](std::vector<HealOnTheWay>& casts)
        {
            ForEachGroupMember(healer, [&](Player* member)
            {
                if (!member->IsAlive())
                    return false;

                for (CurrentSpellTypes const type : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
                {
                    Spell const* spell = member->GetCurrentSpell(type);
                    if (spell && spell->GetSpellInfo()->IsPositive() && !spell->m_targets.GetUnitTargetGUID().IsEmpty())
                        casts.push_back({member->GetGUID(), spell->m_targets.GetUnitTargetGUID()});
                }

                return false;
            });
        });

    std::vector<ObjectGuid> targets;
    for (HealOnTheWay const& heal : heals)
        if (own || heal.caster != healer->GetGUID())
            targets.push_back(heal.target);

    return targets;
}

// Null outside heroic phase 3. Every healer asks each tick, so one read per instance per ms.
static AnubarakState* RefreshColdHeals(PlayerbotAI* botAI)
{
    if (!AnubarakSwarmHealing(botAI))
        return nullptr;

    AnubarakState* state = ReadPhase(botAI);
    if (!state)
        return nullptr;

    uint32 const now = getMSTime();
    if (state->coldMemoValid && state->coldMemoMs == now)
        return state;

    state->coldMemoValid = true;
    state->coldMemoMs = now;

    Player* bot = botAI->GetBot();
    std::vector<Player*> healers;
    std::vector<ColdCarrierAtRisk> atRisk;
    std::vector<ObjectGuid> const onTheWay = HealsOnTheWay(bot, *state, true);
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (!member->IsAlive())
            return false;

        if (PlayerbotAI::IsHeal(member) && GET_PLAYERBOT_AI(member))
            healers.push_back(member);

        ColdTick const tick = NextColdTick(member);
        if (ColdTickDeadly(tick))
            atRisk.push_back({member, tick,
                              std::find(onTheWay.begin(), onTheWay.end(), member->GetGUID()) != onTheWay.end()});

        return false;
    });

    if (!state->healDutySet)
        SetHealDuties(*state, healers);

    // A healer back from a battle res takes its class duty
    state->coldHealersAlive = 0;
    state->tankHealersAlive = 0;
    for (Player* healer : healers)
    {
        state->healDuty.try_emplace(healer->GetGUID(), ClassHealDuty(healer));
        ++(DutyOf(*state, healer->GetGUID()) == AnubarakHealDuty::Cold ? state->coldHealersAlive
                                                                        : state->tankHealersAlive);
    }

    auto const living = [&healers](ObjectGuid const& guid)
    {
        return std::any_of(healers.begin(), healers.end(), [&guid](Player* healer) { return healer->GetGUID() == guid; });
    };
    auto const risky = [&atRisk](ObjectGuid const& guid)
    {
        return std::any_of(atRisk.begin(), atRisk.end(),
                           [&guid](ColdCarrierAtRisk const& carrier) { return carrier.member->GetGUID() == guid; });
    };

    std::vector<ObjectGuid> ended;
    for (auto const& [healer, carrier] : state->coldOf)
        if (!living(healer) || !risky(carrier))
            ended.push_back(healer);

    for (ObjectGuid const& healer : ended)
        state->coldOf.erase(healer);

    for (ColdCarrierAtRisk const& carrier : atRisk)
        if (living(carrier.member->GetGUID()))
            state->coldOf[carrier.member->GetGUID()] = carrier.member->GetGUID();

    // The earliest tick first, then one nothing is being cast on yet so the healers spread over a wave,
    // then the least left at it
    std::sort(atRisk.begin(), atRisk.end(), [](ColdCarrierAtRisk const& lhs, ColdCarrierAtRisk const& rhs)
    {
        return std::make_tuple(lhs.tick.inMs, lhs.coming, lhs.tick.left, lhs.member->GetGUID()) <
               std::make_tuple(rhs.tick.inMs, rhs.coming, rhs.tick.left, rhs.member->GetGUID());
    });

    state->coldAtRisk.clear();
    for (ColdCarrierAtRisk const& carrier : atRisk)
    {
        ObjectGuid const guid = carrier.member->GetGUID();
        state->coldAtRisk.push_back(guid);
        if (std::any_of(state->coldOf.begin(), state->coldOf.end(),
                        [&guid](auto const& duty) { return duty.second == guid; }))
            continue;

        Player* nearest = nullptr;
        float nearestDist = 0.0f;
        for (Player* healer : healers)
        {
            if (DutyOf(*state, healer->GetGUID()) != AnubarakHealDuty::Cold || state->coldOf.count(healer->GetGUID()) ||
                !HealerReaches(healer, carrier.member))
                continue;

            float const dist = healer->GetExactDist(carrier.member);
            if (!nearest || dist < nearestDist || (!(nearestDist < dist) && healer->GetGUID() < nearest->GetGUID()))
            {
                nearest = healer;
                nearestDist = dist;
            }
        }

        if (nearest)
            state->coldOf[nearest->GetGUID()] = guid;
    }

    return state;
}

// Rows under the urgent ones, SWARM_HEAL_TIERS for a heal that would only feed the swarm. A carrier's cover
// and a HoT on a non-tank under ANUBARAK_SWARM_HOT_HP rank ahead of the tanks' top-off, which can wait
static constexpr uint8 SWARM_HEAL_TIERS = 4;
static constexpr uint8 SWARM_HEAL_HOT = 1;
static constexpr uint8 SWARM_HEAL_TANK_TOPOFF = 2;
static constexpr uint8 SWARM_HEAL_COLD_TOPOFF = 3;

static uint8 SwarmHealTier(PlayerbotAI* botAI, Player* member, uint32 cold)
{
    float const pct = member->GetHealthPct();
    bool const tank = IsAnubarakTankPlayer(member);
    bool const carrier = member->HasAura(cold);
    if (carrier && ColdCarrierCover(botAI, member))
        return 0;
    if (SwarmHot(botAI, member))
        return SWARM_HEAL_HOT;
    if (tank && pct < ANUBARAK_SWARM_TANK_TOPOFF_PCT)
        return SWARM_HEAL_TANK_TOPOFF;
    if (carrier && pct < ANUBARAK_SWARM_COLD_TOPOFF_PCT)
        return SWARM_HEAL_COLD_TOPOFF;

    return SWARM_HEAL_TIERS;
}

// Everyone SwarmHealTier can rank under SWARM_HEAL_TIERS for some healer: carriers, tanks under the
// top-off, non-tanks under ANUBARAK_SWARM_HOT_HP with no HoT. Keep in step with SwarmHealTier.
static std::vector<ObjectGuid> const& SwarmHealable(Player* bot, AnubarakState const& state)
{
    return state.swarmHealable.Get(bot, [bot](std::vector<ObjectGuid>& healable)
    {
        uint32 const cold = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
        ForEachGroupMember(bot, [&](Player* member)
        {
            bool listed = member->HasAura(cold);
            if (!listed && IsAnubarakTankPlayer(member))
                listed = member->GetHealthPct() < ANUBARAK_SWARM_TANK_TOPOFF_PCT;
            else if (!listed)
                listed = member->GetHealth() < ANUBARAK_SWARM_HOT_HP &&
                         member->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL).empty();

            if (listed)
                healable.push_back(member->GetGUID());

            return false;
        });
    });
}

AnubarakSwarmHealPick GetAnubarakSwarmHealPick(PlayerbotAI* botAI)
{
    AnubarakSwarmHealPick pick;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AnubarakState const* state = bot && PlayerbotAI::IsHeal(bot) ? RefreshColdHeals(botAI) : nullptr;
    if (!state)
        return pick;

    float const range = botAI->GetRange("heal");
    auto const reaches = [&](Player* member)
    {
        return member && member->IsAlive() && member->IsInMap(bot) && bot->IsWithinDistInMap(member, range) &&
               bot->IsWithinLOSInMap(member);
    };
    std::vector<ObjectGuid> const onTheWay = HealsOnTheWay(bot, *state);
    auto const incoming = [&onTheWay](Player* member)
    { return std::find(onTheWay.begin(), onTheWay.end(), member->GetGUID()) != onTheWay.end(); };
    auto const lowestTank = [&](float below)
    {
        Player* lowest = nullptr;
        Player* open = nullptr;
        ForEachListed(bot, TankPlayers(bot, *state), [&](Player* member)
        {
            if (!IsAnubarakTankPlayer(member) || member->GetHealthPct() >= below || !reaches(member))
                return false;

            if (!lowest || member->GetHealthPct() < lowest->GetHealthPct())
                lowest = member;
            if ((!open || member->GetHealthPct() < open->GetHealthPct()) && !incoming(member))
                open = member;

            return false;
        });

        return open ? open : lowest;
    };

    auto const take = [&](Player* target, bool rescue)
    {
        pick.target = target;
        pick.urgent = true;
        pick.rescue = rescue;
    };

    uint32 const cold = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
    // The lowest member of the best tier in reach and sight, null with none below SWARM_HEAL_TIERS. In the
    // tanks' top-off a tank with a heal on the way ranks behind one without
    auto const tierPick = [&](uint8& bestTier)
    {
        Player* best = nullptr;
        bool bestIncoming = false;
        bestTier = SWARM_HEAL_TIERS;
        ForEachListed(bot, SwarmHealable(bot, *state), [&](Player* member)
        {
            if (!member->IsAlive() || !bot->IsWithinDistInMap(member, range))
                return false;

            uint8 const tier = SwarmHealTier(botAI, member, cold);
            if (tier == SWARM_HEAL_TIERS || tier > bestTier)
                return false;

            bool const topOff = tier == SWARM_HEAL_TANK_TOPOFF;
            bool const heading = topOff && incoming(member);
            bool const lower = !best || member->GetHealthPct() < best->GetHealthPct();
            bool const wins = tier < bestTier || (heading != bestIncoming ? !heading : lower);
            if (!wins || !bot->IsWithinLOSInMap(member))
                return false;

            bestTier = tier;
            best = member;
            bestIncoming = heading;
            return false;
        });

        return best;
    };

    if (ColdTickDeadly(NextColdTick(bot)))
        take(bot, true);
    else if (DutyOf(*state, bot->GetGUID()) == AnubarakHealDuty::Cold)
    {
        auto const assigned = state->coldOf.find(bot->GetGUID());
        Player* carrier = assigned != state->coldOf.end() ? ObjectAccessor::GetPlayer(*bot, assigned->second) : nullptr;
        float const tankLine =
            state->tankHealersAlive ? ANUBARAK_SWARM_COLD_HEALER_TANK_PCT : ANUBARAK_SWARM_TANK_URGENT_PCT;
        if (reaches(carrier))
            take(carrier, true);
        else if (Player* tank = lowestTank(tankLine))
            take(tank, false);
        // Two tank healers alone fall behind the swarm on three tanks, so with no carrier of its own at risk
        // a cold healer tops the tanks up too, once nobody's dying and no carrier waits for its cover
        else if (!carrier)
        {
            uint8 tier = SWARM_HEAL_TIERS;
            Player* low = tierPick(tier);
            if (low && tier == SWARM_HEAL_TANK_TOPOFF)
                take(low, false);
        }
    }
    else if (Player* tank = lowestTank(ANUBARAK_SWARM_TANK_URGENT_PCT))
        take(tank, false);
    else
    {
        // A carrier at risk that no healer in reach is assigned to, one nobody else is casting on first
        Player* first = nullptr;
        Player* open = nullptr;
        for (ObjectGuid const& guid : state->coldAtRisk)
        {
            Player* carrier = ObjectAccessor::GetPlayer(*bot, guid);
            if (!reaches(carrier))
                continue;

            bool covered = false;
            for (auto const& [healerGuid, carrierGuid] : state->coldOf)
            {
                Player* healer = carrierGuid == guid ? ObjectAccessor::GetPlayer(*bot, healerGuid) : nullptr;
                covered = covered || (healer && healer->IsAlive() && HealerReaches(healer, carrier));
            }

            if (covered)
                continue;

            if (!first)
                first = carrier;
            if (!incoming(carrier))
            {
                open = carrier;
                break;
            }
        }

        if (Player* carrier = open ? open : first)
            take(carrier, true);
    }

    if (!pick.target)
    {
        uint8 bestTier = SWARM_HEAL_TIERS;
        pick.target = tierPick(bestTier);
        if (pick.target)
            pick.hot = bestTier == SWARM_HEAL_HOT;
    }

    return pick;
}

static bool TryCast(PlayerbotAI* botAI, char const* spell, Unit* target)
{
    return botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target);
}

static char const* const COLD_RESCUE_DEFENSIVES[] = {"divine shield", "divine protection", "barkskin"};
static char const* const COLD_RESCUE_INSTANTS[] = {"holy shock", "riptide"};
// Fastest first, each only when it lands before the tick
static char const* const COLD_RESCUE_HEALS[] = {"flash of light", "flash heal",   "lesser healing wave",
                                                "nourish",        "regrowth",     "holy light",
                                                "greater heal",   "healing wave"};

bool CastAnubarakColdRescue(PlayerbotAI* botAI, Player* member)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !member)
        return false;

    ColdTick const tick = NextColdTick(member);
    if (!ColdTickDeadly(tick))
        return false;

    bool const shieldable = !member->HasAura(SPELL_WEAKENED_SOUL);
    if (member == bot)
    {
        for (char const* defensive : COLD_RESCUE_DEFENSIVES)
            if (TryCast(botAI, defensive, bot))
                return true;

        // the shield covers the tick for less, so Pain Suppression waits for Weakened Soul
        if (!shieldable && TryCast(botAI, "pain suppression", bot))
            return true;
    }

    if (shieldable && TryCast(botAI, "power word: shield", member))
        return true;

    for (char const* instant : COLD_RESCUE_INSTANTS)
        if (TryCast(botAI, instant, member))
            return true;

    if (member->HasAuraState(AURA_STATE_SWIFTMEND) && TryCast(botAI, "swiftmend", member))
        return true;

    for (char const* heal : COLD_RESCUE_HEALS)
    {
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", heal)->Get();
        SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (info && static_cast<int32>(info->CalcCastTime(bot)) + ANUBARAK_COLD_RESCUE_MARGIN_MS < tick.inMs &&
            TryCast(botAI, heal, member))
            return true;
    }

    return CastAnubarakColdCarrierCover(botAI, member);
}

bool AnubarakAuraMasteryDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_PALADIN || !AnubarakSwarmHealing(botAI))
        return false;

    bool due = false;
    AnubarakState const* state = AnubarakStateOf(bot);
    if (bot->GetAuraOfRankedSpell(SPELL_FROST_RESISTANCE_AURA_RANK_1, bot->GetGUID()) &&
        botAI->CanCastSpell("aura mastery", bot) && state)
    {
        uint32 const coldId = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
        uint32 under = 0;
        ForEachListed(bot, ColdCarriers(bot, *state), [&](Player* member)
        {
            Aura* cold = member->IsAlive() ? member->GetAura(coldId) : nullptr;
            AuraEffect* effect = cold ? cold->GetEffect(EFFECT_0) : nullptr;
            if (!effect || cold->GetMaxDuration() - cold->GetDuration() >= ANUBARAK_MASTERY_ROUND_MS)
                return false;

            if (static_cast<int64>(member->GetHealth()) < effect->GetAmount())
                ++under;

            return false;
        });

        due = under >= ANUBARAK_MASTERY_MIN_CARRIERS;
    }

    return due;
}

// A healer that comes into phase 3 at half mana runs dry inside 45 s, so the mana tools go out before it
constexpr float ANUBARAK_PLEA_MANA_PCT = 80.0f;
// Divine Plea halves the paladin's healing for 15 s
constexpr float ANUBARAK_PLEA_SAFE_PCT = 50.0f;
constexpr float ANUBARAK_INNERVATE_MANA_PCT = 50.0f;

static bool AnubarakManaWindow(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && AnubarakHeroic(bot) && AnubarakEngaged(botAI) && !bot->HasAura(SPELL_MARK);
}

bool AnubarakDivinePleaDue(PlayerbotAI* botAI)
{
    if (!AnubarakManaWindow(botAI) || GetAnubarakPhase(botAI) == AnubarakPhase::Swarm)
        return false;

    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_PALADIN || !PlayerbotAI::IsHeal(bot) ||
        bot->GetPowerPct(POWER_MANA) >= ANUBARAK_PLEA_MANA_PCT || !botAI->CanCastSpell("divine plea", bot))
        return false;

    AnubarakState const* state = AnubarakStateOf(bot);
    if (!state)
        return false;

    uint32 const cold = sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, bot);
    bool hurt = false;
    auto const check = [&](Player* member)
    {
        hurt = member->IsAlive() && member->GetHealthPct() < ANUBARAK_PLEA_SAFE_PCT &&
               (IsAnubarakTankPlayer(member) || member->HasAura(cold));
        return hurt;
    };
    ForEachListed(bot, TankPlayers(bot, *state), check);
    if (!hurt)
        ForEachListed(bot, ColdCarriers(bot, *state), check);

    return !hurt;
}

// Never a tank: Innervate's caster form drops bear form mid-fight
Player* GetAnubarakInnervateTarget(PlayerbotAI* botAI)
{
    if (!AnubarakManaWindow(botAI) || botAI->GetBot()->getClass() != CLASS_DRUID ||
        IsAnubarakTankPlayer(botAI->GetBot()))
        return nullptr;

    Player* bot = botAI->GetBot();
    Unit* low = botAI->GetAiObjectContext()->GetValue<Unit*>("healer low mana")->Get();
    Player* healer = low ? low->ToPlayer() : nullptr;
    if (!healer || healer->GetPowerPct(POWER_MANA) >= ANUBARAK_INNERVATE_MANA_PCT ||
        healer->GetPowerPct(POWER_MANA) >= bot->GetPowerPct(POWER_MANA) || botAI->HasAura("innervate", healer) ||
        !bot->IsWithinDistInMap(healer, botAI->GetRange("spell")) || !bot->IsWithinLOSInMap(healer))
        return nullptr;

    uint32 const spell = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "innervate")->Get();
    return spell && !bot->HasSpellCooldown(spell) ? healer : nullptr;
}

bool AnubarakInnervateHealerDue(PlayerbotAI* botAI) { return GetAnubarakInnervateTarget(botAI) != nullptr; }

bool AnubarakSelfInnervateHeld(PlayerbotAI* botAI)
{
    if (!AnubarakManaWindow(botAI) || botAI->GetBot()->getClass() != CLASS_DRUID)
        return false;

    if (IsAnubarakTankPlayer(botAI->GetBot()))
        return true;

    Unit* low = botAI->GetAiObjectContext()->GetValue<Unit*>("healer low mana")->Get();
    return low && low->GetPowerPct(POWER_MANA) < botAI->GetBot()->GetPowerPct(POWER_MANA);
}

bool AnubarakSwarmHealWasted(Player* target)
{
    return target && !IsAnubarakTankPlayer(target) &&
           !target->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_PENETRATING_COLD, target));
}

bool AnubarakSwarmShieldsTank(Player* bot, Player* target)
{
    return bot->getClass() == CLASS_PRIEST && IsAnubarakTankPlayer(target) && !target->HasAura(SPELL_WEAKENED_SOUL);
}

constexpr uint32 SPELL_PAIN_SUPPRESSION = 33206;
constexpr uint32 SPELL_GUARDIAN_SPIRIT = 47788;
// Lay on Hands first: a full heal, the rest only cut what lands. No Hand of Sacrifice: under the swarm
// the paladin sits at 5-20%, and 30% of a tank's hits kills it
constexpr char const* TANK_RESCUES[] = {"lay on hands", "pain suppression", "guardian spirit"};

bool GetAnubarakTankRescue(PlayerbotAI* botAI, AnubarakTankRescue& out)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_PRIEST) || !bot->IsAlive() ||
        bot->HasAura(SPELL_MARK) || !AnubarakHeroic(bot) || !AnubarakEngaged(botAI) ||
        GetAnubarakPhase(botAI) != AnubarakPhase::Swarm)
        return false;

    out = AnubarakTankRescue();
    AnubarakState const* state = AnubarakStateOf(bot);
    if (!state)
        return false;

    ForEachListed(bot, TankPlayers(bot, *state), [&](Player* member)
    {
        if (!IsAnubarakTankPlayer(member) || !member->IsAlive() ||
            member->GetHealthPct() >= ANUBARAK_TANK_RESCUE_PCT ||
            (out.tank && member->GetHealthPct() >= out.tank->GetHealthPct()) ||
            member->HasAura(SPELL_PAIN_SUPPRESSION) || member->HasAura(SPELL_GUARDIAN_SPIRIT) ||
            !bot->IsWithinLOSInMap(member))
            return false;

        for (char const* spell : TANK_RESCUES)
        {
            if (!botAI->CanCastSpell(spell, member))
                continue;

            out.tank = member;
            out.spell = spell;
            break;
        }

        return false;
    });

    return out.tank != nullptr;
}

constexpr uint32 SPELL_LUST_EXHAUSTION = 57723;
constexpr uint32 SPELL_LUST_SATED = 57724;

char const* GetAnubarakLust(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_SHAMAN || !bot->IsAlive() || bot->HasAura(SPELL_MARK) ||
        bot->HasAura(SPELL_LUST_SATED) || bot->HasAura(SPELL_LUST_EXHAUSTION) || !AnubarakEngaged(botAI) ||
        !AnubarakLustOpen(botAI))
        return nullptr;

    for (char const* lust : {"heroism", "bloodlust"})
        if (botAI->CanCastSpell(lust, bot))
            return lust;

    return nullptr;
}

// Phase 3 has no time to spare for mana: the warlocks tap from his 40%, the mages Evocate in the submerge
// before it
constexpr float ANUBARAK_TOPUP_PCT = 40.0f;
constexpr float ANUBARAK_TOPUP_TAP_MANA_PCT = 95.0f;
constexpr float ANUBARAK_TOPUP_TAP_HEALTH_PCT = 50.0f;
constexpr float ANUBARAK_TOPUP_EVOCATION_MANA_PCT = 60.0f;

char const* GetAnubarakManaTopUp(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    uint8 const cls = bot ? bot->getClass() : 0;
    if (!bot || (cls != CLASS_WARLOCK && cls != CLASS_MAGE) || !bot->IsAlive() || !AnubarakEngaged(botAI))
        return nullptr;

    AnubarakState const* state = ReadPhase(botAI);
    Creature* boss = state ? GetAnubarak(bot) : nullptr;
    if (!boss || state->phaseValue == AnubarakPhase::Swarm || boss->GetHealthPct() > ANUBARAK_TOPUP_PCT)
        return nullptr;

    char const* spell = nullptr;
    if (!bot->HasAura(SPELL_MARK) && !IsAnubarakEscort(botAI) && !GetAnubarakDutySphere(bot))
    {
        float const mana = bot->GetPowerPct(POWER_MANA);
        if (cls == CLASS_WARLOCK && mana < ANUBARAK_TOPUP_TAP_MANA_PCT &&
            bot->GetHealthPct() > ANUBARAK_TOPUP_TAP_HEALTH_PCT && botAI->CanCastSpell("life tap", bot))
            spell = "life tap";
        else if (cls == CLASS_MAGE && state->phaseValue == AnubarakPhase::Submerged &&
                 mana < ANUBARAK_TOPUP_EVOCATION_MANA_PCT && botAI->CanCastSpell("evocation", bot))
            spell = "evocation";
    }

    return spell;
}

bool AnubarakManaTopUpDue(PlayerbotAI* botAI) { return GetAnubarakManaTopUp(botAI) != nullptr; }

constexpr uint32 SPELL_JUDGEMENT_OF_LIGHT_CAST = 20271;
constexpr uint32 SPELL_JUDGEMENT_OF_LIGHT = 20185;
// The holder refreshes it this long before it runs out. A healer takes the turn only with this much more
// mana than the others.
constexpr int32 ANUBARAK_LIGHT_REFRESH_MS = 3000;
constexpr float ANUBARAK_LIGHT_HEALER_MANA_HANDICAP = 20.0f;

// Known, off cooldown, affordable, in reach and in sight of him, and not capped on him
static bool CanJudgeLight(Player* paladin, Creature* boss)
{
    if (paladin->getClass() != CLASS_PALADIN || !paladin->IsAlive() || !GET_PLAYERBOT_AI(paladin) ||
        paladin->HasAura(SPELL_MARK) || !ReadyInstant(paladin, SPELL_JUDGEMENT_OF_LIGHT_CAST) ||
        AnubarakThreatCapped(paladin))
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_JUDGEMENT_OF_LIGHT_CAST);
    return info && paladin->IsWithinCombatRange(boss, info->GetMaxRange(false, paladin)) &&
           paladin->IsWithinLOSInMap(boss);
}

Player* GetAnubarakLightJudge(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    Creature* boss = state && state->phaseValue == AnubarakPhase::Swarm ? GetAnubarak(bot) : nullptr;
    if (!boss)
        return nullptr;

    uint32 const now = getMSTime();
    if (!state->lightMemoValid || state->lightMemoMs != now)
    {
        state->lightMemoValid = true;
        state->lightMemoMs = now;
        Aura const* light = boss->GetAura(SPELL_JUDGEMENT_OF_LIGHT);
        bool const due = !light || light->GetDuration() < ANUBARAK_LIGHT_REFRESH_MS;
        Player* holder = ObjectAccessor::GetPlayer(*bot, state->lightJudge);
        if (!due)
            state->lightDue = false;
        else if (!state->lightDue || !holder || !CanJudgeLight(holder, boss))
        {
            state->lightDue = true;
            Player* best = nullptr;
            float bestMana = 0.0f;
            ForEachGroupMember(bot, [&](Player* member)
            {
                float const mana = member->GetPowerPct(POWER_MANA) -
                                   (PlayerbotAI::IsHeal(member) ? ANUBARAK_LIGHT_HEALER_MANA_HANDICAP : 0.0f);
                if (CanJudgeLight(member, boss) && (!best || mana > bestMana))
                {
                    best = member;
                    bestMana = mana;
                }

                return false;
            });

            ObjectGuid const picked = best ? best->GetGUID() : ObjectGuid::Empty;
            state->lightJudge = picked;
        }
    }

    return state->lightJudge.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state->lightJudge);
}

bool AnubarakLightJudgeDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AnubarakState const* state = bot && GetAnubarakLightJudge(bot) == bot ? ReadPhase(botAI) : nullptr;
    return state && state->lightDue;
}

bool AnubarakSwarmLightHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && bot->getClass() == CLASS_PALADIN && AnubarakLeechingSwarmActive(botAI);
}

constexpr uint32 SPELL_HEALING_STREAM_FIRST_RANK = 5394;
constexpr uint32 SPELL_MANA_TIDE_FIRST_RANK = 16190;

bool AnubarakSwarmJudgementHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Creature* boss = bot && bot->getClass() == CLASS_PALADIN && AnubarakLeechingSwarmActive(botAI) ? GetAnubarak(bot)
                                                                                                    : nullptr;
    return boss && boss->HasAura(SPELL_JUDGEMENT_OF_LIGHT, bot->GetGUID());
}

bool AnubarakHealingStreamDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_SHAMAN || !bot->IsAlive() || bot->HasAura(SPELL_MARK) ||
        !AnubarakEngaged(botAI) || !AnubarakLeechingSwarmActive(botAI))
        return false;

    Creature* totem = bot->GetMap()->GetCreature(bot->m_SummonSlot[SUMMON_SLOT_TOTEM_WATER]);
    if (!totem || !totem->IsAlive())
        return true;

    uint32 const spell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spell);
    uint32 const first = info ? info->GetFirstRankSpell()->Id : 0;
    return first != SPELL_HEALING_STREAM_FIRST_RANK && first != SPELL_MANA_TIDE_FIRST_RANK;
}

static AnubarakState* ResolveDuties(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return nullptr;

    Group* group = bot->GetGroup();
    ObjectGuid const groupGuid = group ? group->GetGUID() : bot->GetGUID();
    uint32 const now = getMSTime();
    if (!state->dutyMemoValid || state->dutyMemoMs != now || state->dutyGroup != groupGuid)
    {
        state->dutyMemoValid = true;
        state->dutyMemoMs = now;
        state->dutyGroup = groupGuid;
        ComputeDuties(bot, *state);
    }

    return state;
}

Creature* GetAnubarakSphereToShoot(Player* bot)
{
    if (!bot || !PlayerbotAI::IsRangedDps(bot))
        return nullptr;

    AnubarakState* state = ResolveDuties(bot);
    if (!state)
        return nullptr;

    for (SphereDuty const& duty : state->duties)
    {
        if (duty.shooter != bot->GetGUID())
            continue;

        Creature* sphere = ResolveCreature(bot, duty.sphere);
        if (!IsFrostSphereFlying(sphere))
            break;

        return sphere;
    }

    return nullptr;
}

bool GetAnubarakGripDuty(Player* bot, AnubarakGripDuty& out)
{
    out = AnubarakGripDuty();
    if (!bot || bot->getClass() != CLASS_DEATH_KNIGHT)
        return false;

    AnubarakState* state = ResolveDuties(bot);
    GripDuty const* duty = nullptr;
    if (state)
        for (GripDuty const& candidate : state->gripDuties)
            if (candidate.dk == bot->GetGUID())
                duty = &candidate;

    if (!duty)
        return false;

    out.side = duty->side;
    out.spot = duty->stand;
    Creature* sphere = ResolveCreature(bot, duty->sphere);
    out.sphere = IsFrostSphereFlying(sphere) ? sphere : nullptr;

    // Cast once in reach from here, inside the step-out or no further from the spot than the stand, and off
    // the other side's patch: the stand follows the sphere, so walking on would only chase it
    bool const ready = out.sphere && !bot->GetSpellCooldownDelay(SPELL_DEATH_GRIP);
    bool const inReach = ready && bot->GetExactDist(out.sphere) <= ANUBARAK_GRIP_RANGE;
    std::vector<Creature*> const spheres = ResolveSpheres(bot, *state);
    ObjectGuid const held[2] = {state->patch0, state->patch1};
    Position const sideSpot = SideSpot(*state, spheres, held, duty->side);
    Creature* otherPatch = PatchToBe(*state, spheres, held, 1 - duty->side);
    Position const otherSpot = otherPatch ? otherPatch->GetPosition() : SideSpot(*state, spheres, held, 1 - duty->side);
    bool const offOther =
        Dist2d(bot->GetPosition(), otherSpot) + ANUBARAK_TANK_PATCH_ARRIVE >= Dist2d(sideSpot, otherSpot) &&
        (!otherPatch || Dist2d(bot->GetPosition(), otherSpot) >= ANUBARAK_GRIP_CLEAR);
    bool const reachesFromHere =
        inReach && offOther &&
        Dist2d(bot->GetPosition(), sideSpot) <= ANUBARAK_GRIP_STEP_OUT + ANUBARAK_TANK_PATCH_ARRIVE;
    bool const reachesEarly = inReach && Dist2d(bot->GetPosition(), sideSpot) <=
                                             Dist2d(out.spot, sideSpot) + ANUBARAK_TANK_PATCH_ARRIVE;

    GripCast const& cast = state->grips[duty->side];
    GripRelay const& relay = state->relays[duty->side];
    if (duty->relay)
    {
        // Posted until the grip before it lands, then it grips as soon as it reaches
        bool const grips = relay.landedMs && reachesEarly;
        if (grips)
            out.step = AnubarakGripStep::Cast;
        else if (Dist2d(bot->GetPosition(), out.spot) > ANUBARAK_TANK_PATCH_ARRIVE)
            out.step = AnubarakGripStep::Walk;
        else
            out.step = AnubarakGripStep::Hold;
    }
    else if (cast.valid && cast.dk == bot->GetGUID())
    {
        // A relay link grips it again from nearer the spot
        bool const kill = cast.landed && relay.dk.IsEmpty();
        out.step = kill ? AnubarakGripStep::Kill : AnubarakGripStep::Hold;
    }
    else if (reachesFromHere || (reachesEarly && offOther))
        out.step = AnubarakGripStep::Cast;
    else if (Dist2d(bot->GetPosition(), out.spot) > ANUBARAK_TANK_PATCH_ARRIVE)
        out.step = AnubarakGripStep::Walk;
    else if (ready)
        out.step = AnubarakGripStep::Cast;
    else
        out.step = AnubarakGripStep::Hold;

    return true;
}

// No duty re-read here: a recompute after the cast sees Death Grip on cooldown and drops the DK, and the
// landed sphere would go unrecorded
void TrackAnubarakGripCast(Player* bot, Creature* sphere, uint8 side)
{
    AnubarakState* state = bot && sphere && side < 2 ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return;

    state->dutyMemoValid = false;
    GripCast& cast = state->grips[side];
    GripRelay& relay = state->relays[side];
    bool const relayLink = relay.dk == bot->GetGUID() && cast.valid && cast.landed && cast.sphere == sphere->GetGUID();
    cast.prevDk = relayLink ? cast.dk : ObjectGuid::Empty;
    cast.prevFrom = relayLink ? cast.from : Position();
    cast.relayLink = relayLink;
    cast.valid = true;
    cast.dk = bot->GetGUID();
    cast.sphere = sphere->GetGUID();
    cast.castMs = getMSTime();
    cast.from = bot->GetPosition();
    cast.landed = false;
    cast.startDist = bot->GetExactDist(sphere);

    // The next link, if any, is posted from where this one lands
    if (relay.dk == bot->GetGUID())
        relay = GripRelay();
}

Creature* GetAnubarakDutySphere(Player* bot)
{
    if (Creature* sphere = GetAnubarakSphereToShoot(bot))
        return sphere;

    // Held off it while a relay link has still to grip it again
    AnubarakGripDuty grip;
    return GetAnubarakGripDuty(bot, grip) &&
                   (grip.step == AnubarakGripStep::Kill || grip.step == AnubarakGripStep::Cast)
               ? grip.sphere
               : nullptr;
}

bool AnubarakSphereInReach(Player* bot, Unit* sphere)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI || !sphere)
        return false;

    float const reach = botAI->GetRange("spell") - ANUBARAK_SHOOT_MARGIN;
    return bot->IsWithinDist3d(sphere->GetPositionX(), sphere->GetPositionY(), sphere->GetPositionZ(), reach) &&
           bot->IsWithinLOSInMap(sphere);
}

bool GetAnubarakSphereStand(Player* bot, Unit* sphere, Position& out)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI || !sphere)
        return false;

    // A flying sphere sits about 13.5 yd up, so the reach across the floor is the rest of the 3D reach
    float const reach = botAI->GetRange("spell") - ANUBARAK_SHOOT_MARGIN - ANUBARAK_SPHERE_STAND_SLACK;
    float const height = sphere->GetPositionZ() - bot->GetPositionZ();
    if (reach <= std::fabs(height))
        return false;

    float const across = std::sqrt(reach * reach - height * height);
    Position const ground(sphere->GetPositionX(), sphere->GetPositionY(), bot->GetPositionZ());
    Vec2 dir = Delta(ground, bot->GetPosition());
    float const length = Length(dir);
    if (length < ANUBARAK_SPHERE_STAND_SLACK)
        return false;

    dir = {dir.x / length, dir.y / length};
    // Already in reach means out of sight from here, so it closes half the gap instead
    float const offset = length > across ? across : length / 2.0f;

    Position stand;
    if (!OnFloor(bot, Offset(ground, dir, offset), stand))
        return false;

    Creature* spike = GetPursuingSpike(bot);
    if (spike && Dist2d(spike->GetPosition(), stand) < ANUBARAK_SPIKE_DANGER_RADIUS)
        return false;

    if (AnubarakSpikeLaneCrosses(bot, stand))
        return false;

    // The leave-Permafrost node would walk it straight back out
    for (Creature* patch : GetFrostSpheres(bot))
        if (IsPermafrostPatch(patch) && Dist2d(patch->GetPosition(), stand) < ANUBARAK_PERMAFROST_SLOW_REACH)
            return false;

    out = stand;
    return true;
}

bool GetAnubarakKiteStand(Player* bot, AnubarakKiteStand& out)
{
    out = AnubarakKiteStand();
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    Creature* spike = state ? GetPursuingSpike(bot) : nullptr;
    if (!spike)
        return false;

    KitePlan const& plan = PlanKite(bot, spike, *state);
    std::vector<AnubarakPursuit::Point> const& track = plan.candidate.track;
    if (track.empty())
        return false;

    int const at = PlanTick(plan, bot);
    int const ahead = AnubarakKitePlanner::AheadTick(
        track, at, static_cast<int>(std::lround(ANUBARAK_KITE_AHEAD_S / AnubarakPursuit::TICK_S)), ANUBARAK_KITE_AHEAD_YD);
    AnubarakPursuit::Point const target = track[static_cast<size_t>(ahead)];
    out.stand = ValidateFloorPoint(bot, Position(target.x, target.y, bot->GetPositionZ()));

    bool const walking = plan.candidate.patch >= 0 && at >= plan.candidate.turnTick;
    AnubarakPursuit::Point const end = track.back();
    bool const arrived = walking && target.x == end.x && target.y == end.y &&
                         bot->GetExactDist2d(end.x, end.y) <= ANUBARAK_KITE_ARRIVE;
    out.patch = walking ? plan.patchGuid : ObjectGuid::Empty;
    out.branch = arrived ? AnubarakKiteBranch::Hold : walking ? AnubarakKiteBranch::Patch : AnubarakKiteBranch::Ring;
    return true;
}

AnubarakKiteStage GetAnubarakKiteStage(Player* kiter, AnubarakKiteBoost* boost)
{
    if (!kiter || !kiter->HasAura(SPELL_MARK) || !AnubarakHeroic(kiter))
        return AnubarakKiteStage::None;

    AnubarakState* state = LatchesFor(kiter) ? anubarakStates.Find(kiter->GetInstanceId()) : nullptr;
    Creature* spike = state ? GetPursuingSpike(kiter) : nullptr;
    if (!spike)
        return AnubarakKiteStage::None;

    KiteStageRead const& read = ReadKiteStage(kiter, spike, *state);
    if (boost)
        *boost = read.boost;

    return read.stage;
}

// On the line from the room centre to the kiter this far short of it, the centre when the kiter is nearer.
// The escort lags its spot, and at 25 yd that put it past the Hand's 30
constexpr float ANUBARAK_ESCORT_SHORT = 20.0f;
// Off the spike by the dodge's clearance, or the dodge sends the escort off its spot: 15 yd off a spike
// faster than the paladin is inside the dodge's 16 yd, and the two swap every half second
constexpr float ANUBARAK_ESCORT_SPIKE_CLEARANCE = 15.0f;
constexpr float ANUBARAK_ESCORT_SPIKE_CLEARANCE_FAST = ANUBARAK_SPIKE_CLEARANCE_FAST;
constexpr float ANUBARAK_ESCORT_RINGS[] = {15.0f, 20.0f, 25.0f};
// The escort shadows the kiter until the held catch is this near, then waits at a post by it. Read any
// earlier, the catch point swings 30-60 yd with each new heading and the escort walks after it.
constexpr float ANUBARAK_ESCORT_COMMIT_S = 4.0f;
// A post is built again once the catch point drifts this far from the one it was built on
constexpr float ANUBARAK_ESCORT_REAIM = 6.0f;
constexpr float ANUBARAK_ESCORT_CENTRE_REACH = 45.0f;
constexpr int ANUBARAK_ESCORT_TRACK_TICKS = 5;
// Hand of Freedom from the paladin due to cast the Hand would spend the GCD the Hand needs
constexpr float ANUBARAK_FREEDOM_HAND_DUE_S = 3.0f;

constexpr float ANUBARAK_SHADOW_TURNS_DEG[] = {0.0f, 15.0f, -15.0f, 30.0f, -30.0f};
constexpr uint32 ANUBARAK_RING_BEARINGS = 12;
constexpr float ANUBARAK_RING_BEARING_STEP_DEG = 30.0f;

// Short of the kiter toward the room centre, turned round it until clear and on the floor. A spike crossing
// the centre can leave none, so then ANUBARAK_ESCORT_RINGS round the kiter, nearest the paladin first
static bool ShadowSpot(Player* paladin, Player* kiter, Creature* spike, std::vector<Position> const& patches,
                       float spikeClearance, float laneClearance, Position& out)
{
    Vec2 const toCenter = Delta(kiter->GetPosition(), ANUBARAK_ROOM_CENTER);
    float const length = Length(toCenter);
    Position const base = length > ANUBARAK_ESCORT_SHORT
                              ? Offset(kiter->GetPosition(), {toCenter.x / length, toCenter.y / length}, ANUBARAK_ESCORT_SHORT)
                              : ANUBARAK_ROOM_CENTER;
    auto const clear = [&](Position const& raw)
    {
        return Dist2d(raw, spike->GetPosition()) >= spikeClearance &&
               SegmentDistance2d(spike->GetPosition(), kiter->GetPosition(), raw) >= laneClearance &&
               Dist2d(raw, kiter->GetPosition()) <= ANUBARAK_HOP_RANGE - 1.0f &&
               std::none_of(patches.begin(), patches.end(), [&raw](Position const& patch)
                            { return Dist2d(raw, patch) < ANUBARAK_HEALER_PATCH_CLEARANCE; });
    };

    for (float degrees : ANUBARAK_SHADOW_TURNS_DEG)
    {
        Vec2 const arm = Rotate(Delta(ANUBARAK_ROOM_CENTER, base), degrees * ANUBARAK_DEG_TO_RAD);
        Position const raw = Offset(ANUBARAK_ROOM_CENTER, arm, 1.0f);
        Position valid;
        if (clear(raw) && OnFloor(paladin, raw, valid))
        {
            out = valid;
            return true;
        }
    }

    std::vector<Position> ring;
    for (float radius : ANUBARAK_ESCORT_RINGS)
        for (uint32 i = 0; i < ANUBARAK_RING_BEARINGS; ++i)
        {
            float const angle = static_cast<float>(i) * ANUBARAK_RING_BEARING_STEP_DEG * ANUBARAK_DEG_TO_RAD;
            Position const raw = Offset(kiter->GetPosition(), {std::cos(angle), std::sin(angle)}, radius);
            if (clear(raw))
                ring.push_back(raw);
        }

    std::sort(ring.begin(), ring.end(), [paladin](Position const& a, Position const& b)
              { return Dist2d(paladin->GetPosition(), a) < Dist2d(paladin->GetPosition(), b); });
    for (Position const& raw : ring)
    {
        Position valid;
        if (OnFloor(paladin, raw, valid))
        {
            out = valid;
            return true;
        }
    }

    return false;
}

// EscortPosts round the plan's catch, the first on the floor, kept until the catch point drifts or the spike
// closes on it
static bool CatchPost(Player* paladin, Player* kiter, Creature* spike, AnubarakState& state, KitePlan const& plan,
                      float catchS, std::vector<Position> const& patches, float spikeClearance, float laneClearance,
                      Position& out)
{
    PlanPath const run = PathFrom(plan, kiter);
    AnubarakPursuit::Point const catchPoint = plan.outcome.catchPoint;
    Position const catchPos(catchPoint.x, catchPoint.y, kiter->GetPositionZ());
    EscortPostLatch& latch = state.escortPost;
    if (latch.valid && latch.kiter == kiter->GetGUID() && Dist2d(latch.catchPoint, catchPos) <= ANUBARAK_ESCORT_REAIM &&
        Dist2d(latch.post, spike->GetPosition()) >= spikeClearance &&
        SegmentDistance2d(spike->GetPosition(), catchPos, latch.post) >= laneClearance)
    {
        out = latch.post;
        return true;
    }

    latch = EscortPostLatch();
    AnubarakPursuit::EscortRules rules;
    rules.centre = ToPoint(ANUBARAK_ROOM_CENTER);
    rules.centreReach = ANUBARAK_ESCORT_CENTRE_REACH;
    rules.patchClearance = ANUBARAK_HEALER_PATCH_CLEARANCE;
    rules.trackClearance = spikeClearance;
    rules.laneClearance = laneClearance;
    std::vector<AnubarakPursuit::Point> points;
    for (Position const& patch : patches)
        points.push_back(ToPoint(patch));

    std::vector<AnubarakPursuit::Point> const track =
        AnubarakPursuit::SpikeTrack(PursuitSpike(spike), run, catchS, ANUBARAK_ESCORT_TRACK_TICKS);
    for (AnubarakPursuit::Point const& post :
         AnubarakPursuit::EscortPosts(catchPoint, ToPoint(spike->GetPosition()), ToPoint(kiter->GetPosition()),
                                      ToPoint(paladin->GetPosition()), track, points, rules))
    {
        Position valid;
        if (OnFloor(paladin, Position(post.x, post.y, paladin->GetPositionZ()), valid))
        {
            latch = {true, kiter->GetGUID(), catchPos, valid};
            out = valid;
            return true;
        }
    }

    return false;
}

// A boosted kiter outruns its escort, so the escort waits where the kite ends: on these rings round the
// patch, off the spike's way in and the kiter's, the nearest the paladin first
constexpr float ANUBARAK_ESCORT_PATCH_RINGS[] = {15.0f, 17.5f, 20.0f};

static bool PatchPost(Player* paladin, Player* kiter, Creature* spike, AnubarakState& state, KitePlan const& plan,
                      std::vector<Position> const& patches, float spikeClearance, float laneClearance, Position& out)
{
    Position const& centre = plan.patchPos;
    auto const clear = [&](Position const& raw)
    {
        return Dist2d(raw, spike->GetPosition()) >= spikeClearance &&
               SegmentDistance2d(spike->GetPosition(), centre, raw) >= laneClearance &&
               SegmentDistance2d(kiter->GetPosition(), centre, raw) >= laneClearance &&
               Dist2d(raw, ANUBARAK_ROOM_CENTER) <= ANUBARAK_ESCORT_CENTRE_REACH &&
               std::none_of(patches.begin(), patches.end(), [&raw](Position const& patch)
                            { return Dist2d(raw, patch) < ANUBARAK_HEALER_PATCH_CLEARANCE; });
    };

    EscortPatchLatch& latch = state.escortPatch;
    if (latch.valid && latch.kiter == kiter->GetGUID() && latch.patch == plan.patchGuid &&
        Dist2d(latch.post, spike->GetPosition()) >= spikeClearance)
    {
        out = latch.post;
        return true;
    }

    std::vector<Position> ring;
    for (float radius : ANUBARAK_ESCORT_PATCH_RINGS)
        for (uint32 i = 0; i < ANUBARAK_RING_BEARINGS; ++i)
        {
            float const angle = static_cast<float>(i) * ANUBARAK_RING_BEARING_STEP_DEG * ANUBARAK_DEG_TO_RAD;
            Position const raw = Offset(centre, {std::cos(angle), std::sin(angle)}, radius);
            if (clear(raw))
                ring.push_back(raw);
        }

    std::sort(ring.begin(), ring.end(), [paladin](Position const& a, Position const& b)
              { return Dist2d(paladin->GetPosition(), a) < Dist2d(paladin->GetPosition(), b); });
    for (Position const& raw : ring)
    {
        Position valid;
        if (OnFloor(paladin, raw, valid))
        {
            latch = {true, kiter->GetGUID(), plan.patchGuid, valid};
            out = valid;
            return true;
        }
    }

    return false;
}

// Shadows the kiter until the catch is near, then posts by it. A boosted bot kiter outruns the Hand's reach,
// so with a patch planned its escort waits by the patch to the kite's end
static bool EscortSpot(Player* paladin, Player* kiter, Creature* spike, AnubarakState& state, KitePlan const& plan,
                       Position& out)
{
    std::vector<Position> const patches = PatchPositions(kiter, state);
    SpikeDodgeReach const reach = DodgeReach(paladin, spike);
    float const spikeClearance = spike->GetSpeed(MOVE_RUN) > paladin->GetSpeed(MOVE_RUN)
                                     ? ANUBARAK_ESCORT_SPIKE_CLEARANCE_FAST
                                     : ANUBARAK_ESCORT_SPIKE_CLEARANCE;
    EscortPatchLatch const& latched = state.escortPatch;
    AnubarakKiteBoost boost;
    bool const waits = plan.candidate.patch >= 0 && GET_PLAYERBOT_AI(kiter) &&
                       ((latched.valid && latched.kiter == kiter->GetGUID() && latched.patch == plan.patchGuid) ||
                        SpeedBoostRunning(kiter) || ReadySpeedBoost(kiter, boost));
    if (!waits)
        state.escortPatch = EscortPatchLatch();

    if (waits && PatchPost(paladin, kiter, spike, state, plan, patches, spikeClearance, reach.laneClearance, out))
        return true;

    float const catchS = CatchLeftS(plan);
    bool const commit = catchS >= 0.0f && catchS <= ANUBARAK_ESCORT_COMMIT_S;
    if (!commit)
        state.escortPost = EscortPostLatch();

    bool const post = commit && CatchPost(paladin, kiter, spike, state, plan, catchS, patches, spikeClearance,
                                          reach.laneClearance, out);
    return post || ShadowSpot(paladin, kiter, spike, patches, spikeClearance, reach.laneClearance, out);
}

// A bot kiter's standby escort: the one kept for this kite while it can still land a Hand and isn't holding
// a burrower for its side, else the nearest such paladin bot
static Player* StandbyEscort(Player* kiter, AnubarakState& state)
{
    auto const qualifies = [kiter](Player* paladin)
    {
        PlayerbotAI* paladinAI =
            paladin && paladin != kiter && paladin->IsAlive() ? GET_PLAYERBOT_AI(paladin) : nullptr;
        return paladinAI && !paladin->HasAura(SPELL_MARK) && HandReady(paladin) && !HoldsSideBurrower(paladin);
    };

    if (state.standbyKiter == kiter->GetGUID() && !state.standby.IsEmpty())
    {
        Player* kept = ObjectAccessor::GetPlayer(*kiter, state.standby);
        if (qualifies(kept))
            return kept;
    }

    Player* best = nullptr;
    for (Player* paladin : ProtectorOrder(kiter))
        if (qualifies(paladin) && (!best || kiter->GetExactDist2d(paladin) < kiter->GetExactDist2d(best)))
            best = paladin;

    state.standbyKiter = kiter->GetGUID();
    state.standby = best ? best->GetGUID() : ObjectGuid::Empty;
    return best;
}

// Heroic, once per instance per ms: the plan's Hand paladin, the one DeriveProtection claims first. A bot
// kiter with no other paladin's Hand planned gets a standby, so a Hand is in reach whatever the plan comes to
static EscortRead const* DeriveEscort(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return nullptr;

    EscortRead& escort = state->escort;
    uint32 const now = getMSTime();
    if (escort.memoValid && escort.memoMs == now)
        return &escort;

    escort = EscortRead();
    escort.memoValid = true;
    escort.memoMs = now;

    Creature* spike = AnubarakHeroic(bot) ? GetPursuingSpike(bot) : nullptr;
    Player* kiter = spike ? GetSpikeTarget(bot) : nullptr;
    if (!kiter || AnubarakKiterImmune(kiter))
        return &escort;

    KitePlan const& plan = PlanKite(kiter, spike, *state);
    if (HandKite(plan) && plan.hand != kiter->GetGUID())
    {
        Player* paladin = ObjectAccessor::GetPlayer(*bot, plan.hand);
        if (!paladin || !paladin->IsAlive())
            return &escort;

        escort.paladin = plan.hand;
        escort.spotValid = EscortSpot(paladin, kiter, spike, *state, plan, escort.spot);
        return &escort;
    }

    Player* standby = GET_PLAYERBOT_AI(kiter) && !kiter->HasAura(SPELL_FORBEARANCE) ? StandbyEscort(kiter, *state)
                                                                                    : nullptr;
    if (!standby)
        return &escort;

    SpikeDodgeReach const reach = DodgeReach(standby, spike);
    float const spikeClearance = spike->GetSpeed(MOVE_RUN) > standby->GetSpeed(MOVE_RUN)
                                     ? ANUBARAK_ESCORT_SPIKE_CLEARANCE_FAST
                                     : ANUBARAK_ESCORT_SPIKE_CLEARANCE;
    escort.paladin = standby->GetGUID();
    escort.spotValid = ShadowSpot(standby, kiter, spike, PatchPositions(kiter, *state), spikeClearance,
                                  reach.laneClearance, escort.spot);
    return &escort;
}

// Once per instance per ms. Never from a paladin with a Hand already on the kiter or one due for its Hand of
// Protection: one paladin's Hands on a target replace each other
static FreedomRead const* DeriveFreedom(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return nullptr;

    FreedomRead& freedom = state->freedom;
    uint32 const now = getMSTime();
    if (freedom.memoValid && freedom.memoMs == now)
        return &freedom;

    freedom = FreedomRead();
    freedom.memoValid = true;
    freedom.memoMs = now;

    Player* kiter = GetSpikeTarget(bot);
    if (!kiter || !UnitOnPermafrost(kiter) || kiter->HasAura(ai::paladin::SPELL_HAND_OF_FREEDOM))
        return &freedom;

    ProtectClaim const* claim = DeriveProtection(bot);
    ObjectGuid const protector = claim ? claim->protector : ObjectGuid::Empty;
    ObjectGuid handDue;
    Creature* spike = AnubarakHeroic(bot) ? GetPursuingSpike(bot) : nullptr;
    if (spike)
    {
        KitePlan const& plan = PlanKite(kiter, spike, *state);
        float const left = CatchLeftS(plan);
        if (HandKite(plan) && left >= 0.0f && left <= ANUBARAK_FREEDOM_HAND_DUE_S)
            handDue = plan.hand;
    }

    auto const frees = [&](Player* paladin)
    {
        PlayerbotAI* paladinAI =
            paladin->getClass() == CLASS_PALADIN && paladin->IsAlive() ? GET_PLAYERBOT_AI(paladin) : nullptr;
        return paladinAI && paladin->GetGUID() != protector && paladin->GetGUID() != handDue &&
               !ai::paladin::HasAnyPaladinHandFromCaster(kiter, paladin) &&
               (paladin == kiter ||
                (paladin->IsWithinDistInMap(kiter, ANUBARAK_HOP_RANGE) && paladin->IsWithinLOSInMap(kiter))) &&
               paladinAI->CanCastSpell("hand of freedom", kiter);
    };

    freedom.kiter = kiter->GetGUID();
    if (frees(kiter))
    {
        freedom.paladin = kiter->GetGUID();
        return &freedom;
    }

    for (Player* paladin : ProtectorOrder(bot))
    {
        if (frees(paladin))
        {
            freedom.paladin = paladin->GetGUID();
            break;
        }
    }

    return &freedom;
}

bool GetAnubarakEscortSpot(Player* bot, Position& out)
{
    EscortRead const* escort = bot && bot->getClass() == CLASS_PALADIN ? DeriveEscort(bot) : nullptr;
    if (!escort || escort->paladin != bot->GetGUID() || !escort->spotValid)
        return false;

    out = escort->spot;
    return true;
}

bool AnubarakHandReserved(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_PALADIN || !AnubarakHeroic(bot))
        return false;

    AnubarakState const* state = ReadPhase(botAI);
    if (!state)
        return false;

    KitePlan const& plan = state->kitePlan;
    ObjectGuid const guid = bot->GetGUID();
    bool const planned = HandKite(plan) && plan.hand == guid;
    bool const claimed = state->protect.protector == guid && state->protect.kiter == plan.kiter;
    if (!plan.valid || (!planned && !claimed))
        return false;

    float const left = CatchLeftS(plan);
    if (left < 0.0f || left > ANUBARAK_KITE_HAND_RESERVE_S)
        return false;

    Player* kiter = ObjectAccessor::GetPlayer(*bot, plan.kiter);
    return kiter && kiter->HasAura(SPELL_MARK) && !UnderHandOfProtection(kiter);
}

bool IsAnubarakEscort(PlayerbotAI* botAI)
{
    Position spot;
    return botAI && GetAnubarakEscortSpot(botAI->GetBot(), spot);
}

Player* GetAnubarakKiterToFree(Player* bot)
{
    FreedomRead const* freedom = bot && bot->getClass() == CLASS_PALADIN ? DeriveFreedom(bot) : nullptr;
    bool const duty = freedom && freedom->paladin == bot->GetGUID();
    return duty ? ObjectAccessor::GetPlayer(*bot, freedom->kiter) : nullptr;
}

// A side tank with no burrower to hold gathers too: marked on its patch, it would walk the spike straight
// into it. So does the main tank. The escort walks after the kiter instead
static AnubarakState const* GatherState(Player* bot)
{
    PlayerbotAI* botAI = bot && AnubarakHeroic(bot) ? GET_PLAYERBOT_AI(bot) : nullptr;
    AnubarakState const* state = botAI ? ReadPhase(botAI) : nullptr;
    if (!state || state->phaseValue != AnubarakPhase::Submerged || !state->submergePosValid ||
        bot->HasAura(SPELL_MARK) || (GetAnubarakBurrowerTankSide(bot) >= 0 && GetAnubarakBurrowerPick(botAI)) ||
        IsAnubarakEscort(botAI))
        return nullptr;

    return state;
}

bool GetAnubarakGatherSpot(Player* bot, Position& out)
{
    AnubarakState const* state = GatherState(bot);
    Position gather;
    if (!state || !GatherCentre(bot, *state, gather))
        return false;

    // On the edge away from where he went down: the spike is summoned there 2.5 s in
    if (bot->GetGUID() == state->mainTank)
    {
        Vec2 const away = Delta(state->submergePos, gather);
        float const length = Length(away);
        out = length > 1.0f ? Offset(gather, {away.x / length, away.y / length}, ANUBARAK_GATHER_TANK_EDGE) : gather;
        return true;
    }

    // A slot per bot round the point, so twenty bots don't stand on one spot
    float const angle = static_cast<float>(bot->GetGUID().GetCounter() % ANUBARAK_RING_BEARINGS) *
                        ANUBARAK_RING_BEARING_STEP_DEG * ANUBARAK_DEG_TO_RAD;
    out = Offset(gather, {std::cos(angle), std::sin(angle)}, ANUBARAK_GATHER_SPREAD);
    return true;
}

bool AnubarakGathers(PlayerbotAI* botAI) { return botAI && GatherState(botAI->GetBot()); }

static AnubarakState const* HealerStandState(Player* bot)
{
    PlayerbotAI* botAI = bot && bot->IsAlive() && PlayerbotAI::IsHeal(bot) ? GET_PLAYERBOT_AI(bot) : nullptr;
    AnubarakState const* state = botAI ? ReadPhase(botAI) : nullptr;
    if (!state || !state->raidPointValid || bot->HasAura(SPELL_MARK) ||
        (state->phaseValue != AnubarakPhase::Surface && state->phaseValue != AnubarakPhase::Swarm))
        return nullptr;

    return state;
}

bool AnubarakHealerStandApplies(PlayerbotAI* botAI) { return botAI && HealerStandState(botAI->GetBot()); }

// Heroic phase 3 burrowers go for the healers: behind the tank patches the line from an exit to a healer
// crosses a patch's slow, where a burrower crawls at 20% and its side tank takes it first
constexpr float ANUBARAK_BURROW_EXITS[][2] = {{740.2f, 193.4f}, {694.5f, 185.4f}, {732.0f, 83.4f}, {694.9f, 102.5f}};
// Healers set off in a phase 1 from his 35%, one at a time, none with a tank under 50%: they can't cast
// while they walk. The ranged DPS bots go all at once in phase 3.
constexpr float ANUBARAK_BEHIND_FROM_PCT = 35.0f;
constexpr float ANUBARAK_BEHIND_TANK_LOW_PCT = 50.0f;
// From the tank patches' midpoint, away from the exits. Deepest first: a burrower pulled off its side
// tank has the longest walk there.
constexpr float ANUBARAK_BEHIND_DEPTHS[] = {10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f,
                                            22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f};
constexpr float ANUBARAK_BEHIND_SIDEWAYS[] = {0.0f, 3.0f, -3.0f, 6.0f, -6.0f, 9.0f, -9.0f, 12.0f, -12.0f};
// A healer this close to him still heals the melee Penetrating Cold carriers; a ranged bot keeps him in
// spell range less a margin
constexpr float ANUBARAK_BEHIND_HEALER_BOSS_REACH = 32.0f;
constexpr float ANUBARAK_BEHIND_RANGED_BOSS_REACH = 28.0f;
// The deepest stands sit right at those reaches, so a held one is kept this much past them, or every
// tank step would move a healer, and it can't cast while it walks
constexpr float ANUBARAK_BEHIND_KEEP_SLACK = 3.0f;
constexpr uint32 ANUBARAK_BEHIND_RETRY_MS = 1000;

struct BehindSlot
{
    Position pos;
    uint8 crossings = 0;
    float depth = 0.0f;
    bool healerFits = false;
    // OnFloor asked once per slot: -1 not yet
    int8 floor = -1;
    Position valid;
};

// Exit lines through a tank patch's slow
static uint8 BehindCrossings(Position const& at, std::vector<Position> const& tankPatches)
{
    uint8 crossings = 0;
    for (auto const& exit : ANUBARAK_BURROW_EXITS)
    {
        Position const from(exit[0], exit[1], at.GetPositionZ());
        crossings += std::any_of(tankPatches.begin(), tankPatches.end(), [&](Position const& patch)
                                 { return SegmentDistance2d(from, at, patch) < ANUBARAK_PERMAFROST_SLOW_REACH; })
                         ? 1
                         : 0;
    }

    return crossings;
}

static bool OffPatches(Position const& at, std::vector<Position> const& patches)
{
    return std::none_of(patches.begin(), patches.end(), [&at](Position const& patch)
                        { return Dist2d(at, patch) < ANUBARAK_HEALER_PATCH_CLEARANCE; });
}

// Where the stands measure him and the tanks from: his drag anchor and each side tank's hold, so a tank's
// chase or his step off the anchor moves no stand. Live positions with no anchor
struct BehindRefs
{
    Position boss;
    std::vector<Position> tanks;
};

static Position BehindBossRef(Player* bot, Creature* boss)
{
    Position anchor;
    return GetAnubarakDragAnchor(bot, anchor) ? anchor : boss->GetPosition();
}

static BehindRefs BehindReferences(Player* bot, AnubarakState const& state, Creature* boss)
{
    BehindRefs refs;
    Position anchor;
    bool const fixed = GetAnubarakDragAnchor(bot, anchor);
    refs.boss = fixed ? anchor : boss->GetPosition();
    for (uint8 i = 0; i < 3; ++i)
    {
        ObjectGuid const guid = i == 0 ? state.mainTank : state.sideTanks[i - 1];
        Player* tank = guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, guid);
        if (!tank || !tank->IsAlive())
            continue;

        refs.tanks.push_back(!fixed ? tank->GetPosition()
                             : i == 0 ? anchor
                                      : GetAnubarakSideTankHold(bot, static_cast<uint8>(i - 1)));
    }

    return refs;
}

// Within reach of him and the three tanks, plus slack
static bool HealerBehindFits(BehindRefs const& refs, Position const& at, float slack = 0.0f)
{
    return Dist2d(at, refs.boss) <= ANUBARAK_BEHIND_HEALER_BOSS_REACH + slack &&
           std::all_of(refs.tanks.begin(), refs.tanks.end(), [&](Position const& tank)
                       { return Dist2d(at, tank) <= ANUBARAK_HEALER_MAIN_TANK_REACH + slack; });
}

static bool BehindRanged(Player* player)
{
    return PlayerbotAI::IsRangedDps(player) && !PlayerbotAI::IsHeal(player) && !IsAnubarakTankPlayer(player);
}

// Healer bots by guid, then in phase 3 the ranged DPS bots within heal reach of every healer stand. Held
// stands stay, and a new one is the slot crossing the most exit lines, then the deepest
static void AssignBehindStands(Player* bot, AnubarakState& state, Creature* boss)
{
    std::vector<ObjectGuid> healers;
    std::vector<ObjectGuid> ranged;
    bool const swarm = state.phaseValue == AnubarakPhase::Swarm;
    ForEachGroupMember(bot, [&](Player* member)
    {
        if (member->IsAlive() && GET_PLAYERBOT_AI(member) && PlayerbotAI::IsHeal(member))
            healers.push_back(member->GetGUID());
        else if (swarm && member->IsAlive() && GET_PLAYERBOT_AI(member) && BehindRanged(member))
            ranged.push_back(member->GetGUID());

        return false;
    });

    std::sort(healers.begin(), healers.end());
    std::sort(ranged.begin(), ranged.end());
    std::vector<Position> taken;
    std::vector<Position> healerStands;
    for (auto it = state.behindStands.begin(); it != state.behindStands.end();)
    {
        bool const healer = std::binary_search(healers.begin(), healers.end(), it->first);
        if (!healer && !std::binary_search(ranged.begin(), ranged.end(), it->first))
        {
            it = state.behindStands.erase(it);
            continue;
        }

        taken.push_back(it->second);
        if (healer)
            healerStands.push_back(it->second);

        ++it;
    }

    std::vector<Position> tankPatches;
    for (uint8 side = 0; side < 2; ++side)
        if (Creature* patch = LatchedTankPatch(bot, state, side))
            tankPatches.push_back(patch->GetPosition());

    if (tankPatches.empty())
        return;

    float midX = 0.0f;
    float midY = 0.0f;
    for (Position const& patch : tankPatches)
    {
        midX += patch.GetPositionX() / static_cast<float>(tankPatches.size());
        midY += patch.GetPositionY() / static_cast<float>(tankPatches.size());
    }

    float exitX = 0.0f;
    float exitY = 0.0f;
    for (auto const& exit : ANUBARAK_BURROW_EXITS)
    {
        exitX += exit[0] / static_cast<float>(std::size(ANUBARAK_BURROW_EXITS));
        exitY += exit[1] / static_cast<float>(std::size(ANUBARAK_BURROW_EXITS));
    }

    Position const mid(midX, midY, tankPatches.front().GetPositionZ());
    Position const exits(exitX, exitY, mid.GetPositionZ());

    Vec2 away = Delta(exits, mid);
    float const length = Length(away);
    if (length < 1.0f)
        return;

    away = {away.x / length, away.y / length};
    Vec2 const sideways = {-away.y, away.x};
    std::vector<Position> const patches = PatchPositions(bot, state);
    BehindRefs const refs = BehindReferences(bot, state, boss);
    std::vector<BehindSlot> slots;
    for (float depth : ANUBARAK_BEHIND_DEPTHS)
        for (float off : ANUBARAK_BEHIND_SIDEWAYS)
        {
            BehindSlot slot;
            slot.pos = Offset(Offset(mid, away, depth), sideways, off);
            slot.crossings = BehindCrossings(slot.pos, tankPatches);
            slot.depth = depth;
            if (!slot.crossings || !OffPatches(slot.pos, patches))
                continue;

            slot.healerFits = HealerBehindFits(refs, slot.pos);
            slots.push_back(slot);
        }

    std::stable_sort(slots.begin(), slots.end(), [](BehindSlot const& a, BehindSlot const& b)
                     {
                         if (a.crossings != b.crossings)
                             return a.crossings > b.crossings;
                         return a.depth > b.depth;
                     });

    auto const take = [&](ObjectGuid const& guid, auto const& fits)
    {
        if (state.behindStands.count(guid))
            return;

        for (BehindSlot& slot : slots)
        {
            if (!fits(slot) || std::any_of(taken.begin(), taken.end(), [&](Position const& other)
                                           { return Dist2d(other, slot.pos) < ANUBARAK_GATHER_SPREAD; }))
                continue;

            if (slot.floor < 0)
                slot.floor = OnFloor(bot, slot.pos, slot.valid) ? 1 : 0;

            if (slot.floor)
            {
                taken.push_back(slot.valid);
                state.behindStands[guid] = slot.valid;
                return;
            }
        }
    };

    for (ObjectGuid const& healer : healers)
    {
        if (state.behindStands.count(healer))
            continue;

        take(healer, [](BehindSlot const& slot) { return slot.healerFits; });
        auto const stand = state.behindStands.find(healer);
        if (stand != state.behindStands.end())
            healerStands.push_back(stand->second);
    }

    for (ObjectGuid const& guid : ranged)
        take(guid, [&](BehindSlot const& slot)
             {
                 return Dist2d(slot.pos, refs.boss) <= ANUBARAK_BEHIND_RANGED_BOSS_REACH &&
                        std::all_of(healerStands.begin(), healerStands.end(), [&slot](Position const& stand)
                                    { return Dist2d(slot.pos, stand) <= ANUBARAK_HEALER_MAIN_TANK_REACH; });
             });
}

// A stand that stops fitting goes at once; a bot without one asks again once a second
static void RefreshBehindStands(Player* bot, AnubarakState& state, Creature* boss, bool lost)
{
    if (lost)
        state.behindStands.erase(bot->GetGUID());

    uint32 const now = getMSTime();
    bool const retry = !state.behindStands.count(bot->GetGUID()) &&
                       (lost || getMSTimeDiff(state.behindTriedMs, now) >= ANUBARAK_BEHIND_RETRY_MS);
    if (state.behindSet && !retry)
        return;

    AssignBehindStands(bot, state, boss);
    state.behindSet = true;
    state.behindTriedMs = now;
}

// Heroic, from his 35% in a phase 1 and in phase 3: when the behind stands apply. Null outside it.
static AnubarakState* BehindWindow(Player* bot, Creature*& boss)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AnubarakState* state = botAI && AnubarakHeroic(bot) ? ReadPhase(botAI) : nullptr;
    boss = state ? GetAnubarak(bot) : nullptr;
    if (!boss || (state->phaseValue != AnubarakPhase::Swarm &&
                  (state->phaseValue != AnubarakPhase::Surface || boss->GetHealthPct() > ANUBARAK_BEHIND_FROM_PCT)))
        return nullptr;

    return state;
}

static bool AnyTankLow(Player* bot, AnubarakState const& state)
{
    for (ObjectGuid const& guid : {state.mainTank, state.sideTanks[0], state.sideTanks[1]})
    {
        Player* tank = guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, guid);
        if (tank && tank->IsAlive() && tank->GetHealthPct() < ANUBARAK_BEHIND_TANK_LOW_PCT)
            return true;
    }

    return false;
}

// Living healer bots by guid
static std::vector<ObjectGuid> const& HealerBots(Player* bot, AnubarakState const& state)
{
    return state.healerBots.Get(bot, [bot](std::vector<ObjectGuid>& healers)
    {
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (member->IsAlive() && GET_PLAYERBOT_AI(member) && PlayerbotAI::IsHeal(member))
                healers.push_back(member->GetGUID());

            return false;
        });

        std::sort(healers.begin(), healers.end());
    });
}

bool AnubarakHealerStandWaits(Player* bot)
{
    Creature* boss = nullptr;
    AnubarakState const* state = bot ? BehindWindow(bot, boss) : nullptr;
    return state && AnyTankLow(bot, *state);
}

// Heroic, from his 35% in a phase 1 and in phase 3: this healer's stand behind the tank patches once every
// healer before it by guid has reached its own. False while it waits its turn or has none.
static bool BehindStand(Player* bot, Position& out)
{
    Creature* boss = nullptr;
    AnubarakState* state = BehindWindow(bot, boss);
    if (!state)
        return false;

    auto mine = state->behindStands.find(bot->GetGUID());
    std::vector<Position> const patches = PatchPositions(bot, *state);
    bool const lost =
        mine != state->behindStands.end() &&
        (!OffPatches(mine->second, patches) ||
         !HealerBehindFits(BehindReferences(bot, *state, boss), mine->second, ANUBARAK_BEHIND_KEEP_SLACK));
    RefreshBehindStands(bot, *state, boss, lost);
    mine = state->behindStands.find(bot->GetGUID());
    if (mine == state->behindStands.end())
        return false;

    if (!state->behindGone.count(bot->GetGUID()))
    {
        for (ObjectGuid const& guid : HealerBots(bot, *state))
        {
            if (guid == bot->GetGUID())
                break;

            Player* before = ObjectAccessor::GetPlayer(*bot, guid);
            auto const stand = state->behindStands.find(guid);
            if (before && stand != state->behindStands.end() &&
                Dist2d(before->GetPosition(), stand->second) > ANUBARAK_HEALER_STAND_LEAVE)
                return false;
        }

        if (AnyTankLow(bot, *state))
            return false;

        state->behindGone.insert(bot->GetGUID());
    }

    out = mine->second;
    return true;
}

bool GetAnubarakHealerStand(Player* bot, Position& out)
{
    AnubarakState const* state = HealerStandState(bot);
    if (!state)
        return false;

    if (BehindStand(bot, out))
        return true;

    std::vector<ObjectGuid> const healers = HealerBots(bot, *state);
    size_t index = static_cast<size_t>(
        std::distance(healers.begin(), std::find(healers.begin(), healers.end(), bot->GetGUID())));
    size_t count = std::max<size_t>(healers.size(), 1);
    std::vector<Position> const patches = PatchPositions(bot, *state);
    auto const clear = [&patches](Position const& slot)
    {
        return std::none_of(patches.begin(), patches.end(), [&slot](Position const& patch)
                            { return Dist2d(slot, patch) < ANUBARAK_HEALER_PATCH_CLEARANCE; });
    };

    // Patches this far apart put a side tank past a heal from the raid point, so the healers split by guid,
    // each group off its side tank toward the raid point, not the anchor, which keeps them off him
    Position centre = state->raidPoint;
    Position const spots[2] = {state->spot0, state->spot1};
    Player* mainTank = state->mainTank.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state->mainTank);
    bool split = false;
    if (Dist2d(spots[0], spots[1]) >= ANUBARAK_HEALER_SPLIT_GAP && index < healers.size())
    {
        Position const& spot = spots[index % 2];
        Vec2 const toward = Delta(spot, state->raidPoint);
        float const length = Length(toward);
        Position const own = length > 1.0f
                                 ? Offset(spot, {toward.x / length, toward.y / length},
                                          std::min(length, ANUBARAK_HEALER_SIDE_OFFSET))
                                 : state->raidPoint;
        if (clear(own) && (!mainTank || !mainTank->IsAlive() ||
                           Dist2d(own, mainTank->GetPosition()) <= ANUBARAK_HEALER_MAIN_TANK_REACH))
        {
            centre = own;
            count = (healers.size() - index % 2 + 1) / 2;
            index /= 2;
            split = true;
        }
    }

    // A main tank who takes him far off the anchor (a human, heroic's submerge drag) is past a heal from the
    // raid point. With a side tank still in reach, half the healers by guid stay and half follow him
    if (mainTank && mainTank->IsAlive() && Dist2d(centre, mainTank->GetPosition()) > ANUBARAK_HEALER_MAIN_TANK_REACH)
    {
        bool const sides = !split && healers.size() > 1 && index < healers.size() &&
                           std::any_of(std::begin(state->sideTanks), std::end(state->sideTanks),
                                       [&](ObjectGuid const& guid)
                                       {
                                           Player* tank = guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, guid);
                                           return tank && tank->IsAlive() &&
                                                  Dist2d(centre, tank->GetPosition()) <= ANUBARAK_HEALER_MAIN_TANK_REACH;
                                       });
        bool const follows = !sides || index % 2 == 1;
        if (sides)
        {
            count = follows ? healers.size() / 2 : (healers.size() + 1) / 2;
            index /= 2;
        }

        Vec2 const toward = Delta(centre, mainTank->GetPosition());
        float const length = Length(toward);
        Position const raw =
            Offset(centre, {toward.x / length, toward.y / length}, length - ANUBARAK_HEALER_MAIN_TANK_REACH);
        Position valid;
        if (follows && clear(raw) && OnFloor(bot, raw, valid))
            centre = valid;
    }

    float const base = 2.0f * static_cast<float>(M_PI) * static_cast<float>(index) / static_cast<float>(count);
    for (uint32 turn = 0; turn < ANUBARAK_HEALER_SLOT_TRIES; ++turn)
    {
        float const angle = base + 2.0f * static_cast<float>(M_PI) * static_cast<float>(turn) /
                                       static_cast<float>(ANUBARAK_HEALER_SLOT_TRIES);
        Position const slot = Offset(centre, {std::cos(angle), std::sin(angle)}, ANUBARAK_GATHER_SPREAD);
        if (clear(slot))
        {
            out = slot;
            return true;
        }
    }

    out = centre;
    return true;
}

bool GetAnubarakRangedStand(Player* bot, Position& out)
{
    PlayerbotAI* botAI = bot && bot->IsAlive() && BehindRanged(bot) ? GET_PLAYERBOT_AI(bot) : nullptr;
    AnubarakState* state = botAI && AnubarakHeroic(bot) ? ReadPhase(botAI) : nullptr;
    Creature* boss = state && state->phaseValue == AnubarakPhase::Swarm ? GetAnubarak(bot) : nullptr;
    if (!boss || GetAnubarakDutySphere(bot) || GetAnubarakBattleResTarget(botAI))
        return false;

    auto mine = state->behindStands.find(bot->GetGUID());
    std::vector<Position> const patches = PatchPositions(bot, *state);
    bool const lost =
        mine != state->behindStands.end() &&
        (!OffPatches(mine->second, patches) ||
         Dist2d(mine->second, BehindBossRef(bot, boss)) > ANUBARAK_BEHIND_RANGED_BOSS_REACH + ANUBARAK_BEHIND_KEEP_SLACK);
    RefreshBehindStands(bot, *state, boss, lost);
    mine = state->behindStands.find(bot->GetGUID());
    if (mine == state->behindStands.end())
        return false;

    out = mine->second;
    return true;
}

bool AnubarakRangedStandApplies(PlayerbotAI* botAI)
{
    Position stand;
    return botAI && GetAnubarakRangedStand(botAI->GetBot(), stand);
}

bool AnubarakDashRunningOrReady(Player* bot)
{
    for (uint32 rank : DASH_RANKS)
        if (bot->HasAura(rank))
            return true;

    uint32 const rank = KnownRank(bot, DASH_RANKS);
    return rank && ReadyInstant(bot, rank);
}

bool AnubarakKiteCatHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && AnubarakEngaged(botAI) && bot->GetMap()->IsHeroic() && bot->getClass() == CLASS_DRUID &&
           bot->HasAura(SPELL_MARK) && bot->GetShapeshiftForm() == FORM_CAT && AnubarakDashRunningOrReady(bot);
}

bool AnubarakKiteCatDropDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    // the tank strategy, not the spec: a cat DPS with Thick Hide reads as a tank by spec
    return bot && bot->IsAlive() && bot->getClass() == CLASS_DRUID && bot->GetShapeshiftForm() == FORM_CAT &&
           (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsRangedDps(bot)) &&
           AnubarakEngaged(botAI) && !AnubarakKiteCatHeld(botAI);
}

bool AnubarakMainTankKeepsBear(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AnubarakState const* state =
        bot && bot->getClass() == CLASS_DRUID && AnubarakEngaged(botAI) ? ReadPhase(botAI) : nullptr;
    if (!state || state->phaseValue != AnubarakPhase::Submerged || bot->GetGUID() != state->mainTank)
        return false;

    int32 const toEmerge = static_cast<int32>(state->submergedMs + ANUBARAK_SPIKE_LIFE_MS - getMSTime());
    return toEmerge <= ANUBARAK_EMERGE_BEAR_MS;
}

bool AnubarakDemonicCircleDue(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_WARLOCK || !bot->IsAlive() || !bot->HasSpell(SPELL_CIRCLE_SUMMON) ||
        bot->HasAura(SPELL_MARK) || !AnubarakHeroic(bot))
        return false;

    Aura const* circle = bot->GetAura(SPELL_CIRCLE_SUMMON, bot->GetGUID());
    int32 const left = circle ? circle->GetDuration() : 0;
    if (AnubarakEngaged(botAI))
    {
        AnubarakPhase const phase = GetAnubarakPhase(botAI);
        return !bot->isMoving() && (phase == AnubarakPhase::Surface || phase == AnubarakPhase::Swarm) &&
               (!circle || (left >= 0 && left < ANUBARAK_CIRCLE_REFRESH_MS));
    }

    // no wait for the stand: walking to the centre unparks it, and the stand walk would undo the trip
    if (GetToCStagingBoss(botAI) != ToCStagingBoss::Anubarak || !IsToCStagingHeld(botAI))
        return false;

    GameObject* placed = WarlockCircle(bot);
    return !circle || !placed || Dist2d(placed->GetPosition(), ANUBARAK_ROOM_CENTER) > ANUBARAK_CIRCLE_CENTRE_SLACK ||
           (left >= 0 && left < ANUBARAK_CIRCLE_PREPULL_MS);
}

bool GetAnubarakCircleSpot(PlayerbotAI* botAI, Position& spot)
{
    if (!botAI || AnubarakEngaged(botAI))
        return false;

    spot = ANUBARAK_ROOM_CENTER;
    return true;
}

// The dodge track's points sit a bot tick apart
constexpr int ANUBARAK_DODGE_TRACK_TICKS = 2;
// A spike this close to where it heads stands
constexpr float ANUBARAK_DODGE_STILL = 0.5f;

// Where the spike heads with nobody marked: a fail idles it 4 s still chasing its last victim at the base
// speed, which can drift it through the gather
static AnubarakPursuit::Point SpikeMotion(Creature* spike)
{
    Position to;
    if (Unit* victim = spike->GetVictim())
        to = victim->GetPosition();
    else if (!spike->movespline->Finalized())
    {
        G3D::Vector3 const end = spike->movespline->FinalDestination();
        to.Relocate(end.x, end.y, end.z);
    }
    else
        return {};

    Vec2 const toward = Delta(spike->GetPosition(), to);
    float const length = Length(toward);
    if (length < ANUBARAK_DODGE_STILL)
        return {};

    float const speed = spike->GetSpeed(MOVE_RUN);
    return {toward.x / length * speed, toward.y / length * speed};
}

// The spike's path over ANUBARAK_DODGE_TRACK_S, per instance per ms: along the kite plan's track while it has
// one, since a chase that turns with its kiter cuts through dodge spots a straight-on read calls clear
static std::vector<AnubarakPursuit::Point> const& DodgeTrack(Player* bot, Creature* spike, AnubarakState& state)
{
    uint32 const now = getMSTime();
    if (state.dodgeTrackMemoValid && state.dodgeTrackMemoMs == now)
        return state.dodgeTrack;

    std::vector<AnubarakPursuit::Point> track;
    Player* kiter = GetSpikeTarget(bot);
    if (!kiter)
        track = AnubarakPursuit::MotionTrack(ToPoint(spike->GetPosition()), SpikeMotion(spike), ANUBARAK_DODGE_TRACK_S,
                                             ANUBARAK_DODGE_TRACK_TICKS * AnubarakPursuit::TICK_S);
    else
    {
        // Read, not judged: the kiter's own ticks judge it, and judging here could reach back into the dodge
        KitePlan const& plan = state.kitePlan;
        if (plan.valid && plan.kiter == kiter->GetGUID() && plan.candidate.track.size() >= 2)
            track = AnubarakPursuit::SpikeTrack(PursuitSpike(spike), PathFrom(plan, kiter), ANUBARAK_DODGE_TRACK_S,
                                                ANUBARAK_DODGE_TRACK_TICKS);
        else
            track = AnubarakPursuit::MotionTrack(ToPoint(spike->GetPosition()), SpikeMotion(spike),
                                                 ANUBARAK_DODGE_TRACK_S,
                                                 ANUBARAK_DODGE_TRACK_TICKS * AnubarakPursuit::TICK_S);
    }

    state.dodgeTrack = std::move(track);
    state.dodgeTrackMemoValid = true;
    state.dodgeTrackMemoMs = now;
    return state.dodgeTrack;
}

static bool InDodgeDanger(Creature* spike, std::vector<AnubarakPursuit::Point> const& track,
                          SpikeDodgeReach const& reach, Position const& at)
{
    return Dist2d(spike->GetPosition(), at) < reach.danger ||
           !AnubarakPursuit::TrackClear(track, ToPoint(at), reach.lane);
}

// to keeps the clearance off the track, and the walk there the lane off it and the danger radius off the spike
static bool DodgeWalkClear(Creature* spike, std::vector<AnubarakPursuit::Point> const& track,
                           SpikeDodgeReach const& reach, Position const& from, Position const& to)
{
    return AnubarakPursuit::TrackClear(track, ToPoint(to), reach.clearance) &&
           AnubarakPursuit::TrackClear(track, ToPoint(from), ToPoint(to), reach.lane) &&
           SegmentDistance2d(from, to, spike->GetPosition()) >= reach.danger;
}

// Where the bot goes back to once the spike has passed
static Position DodgeHome(Player* bot)
{
    Position home;
    return (GetAnubarakGatherSpot(bot, home) || GetAnubarakHealerStand(bot, home)) ? home : bot->GetPosition();
}

bool AnubarakSpikeDangerAt(Player* bot, Position const& at)
{
    Creature* spike = bot ? GetPursuingSpike(bot) : nullptr;
    AnubarakState* state = spike ? anubarakStates.Find(bot->GetInstanceId()) : nullptr;
    return state && InDodgeDanger(spike, DodgeTrack(bot, spike, *state), DodgeReach(bot, spike), at);
}

bool AnubarakInSpikeDanger(Player* bot)
{
    return bot && !bot->HasAura(SPELL_MARK) && AnubarakEngaged(GET_PLAYERBOT_AI(bot)) &&
           AnubarakSpikeDangerAt(bot, bot->GetPosition());
}

bool GetAnubarakSpikeDodgeSpot(Player* bot, Position& out)
{
    Creature* spike = bot ? GetPursuingSpike(bot) : nullptr;
    AnubarakState* state = spike ? anubarakStates.Find(bot->GetInstanceId()) : nullptr;
    if (!state)
        return false;

    // Clear of every point on the path, so the spot never lies ahead of the spike
    SpikeDodgeReach const reach = DodgeReach(bot, spike);
    std::vector<HazardCircle> onTrack;
    for (AnubarakPursuit::Point const& point : DodgeTrack(bot, spike, *state))
        onTrack.emplace_back(Position(point.x, point.y, bot->GetPositionZ()), reach.clearance);

    std::vector<HazardCircle> withPatches = onTrack;
    if (AnubarakHeroic(bot))
        for (Creature* sphere : GetFrostSpheres(bot))
            if (IsPermafrostPatch(sphere))
                withPatches.emplace_back(sphere->GetPosition(), ANUBARAK_PERMAFROST_SLOW_REACH);

    // Heroic patches slow 80%, but a dodge into one still beats staying in the lane. Ties in a ring go to
    // the slot's side, for a short walk back.
    Position const home = DodgeHome(bot);
    HazardSweepCache cache;
    Position spot = FindNearestPositionClearOfHazards(bot, withPatches, ANUBARAK_SPIKE_DODGE_SEARCH, 2.0f,
                                                      static_cast<float>(M_PI) / 8.0f, &home, {}, &cache);
    if (spot == Position() && withPatches.size() > onTrack.size())
        spot = FindNearestPositionClearOfHazards(bot, onTrack, ANUBARAK_SPIKE_DODGE_SEARCH, 2.0f,
                                                 static_cast<float>(M_PI) / 8.0f, &home, {}, &cache);

    if (spot == Position())
        return false;

    out = spot;
    return true;
}

static DodgeHold* ValidDodgeHold(Player* bot)
{
    if (!InToCInstance(bot) || !bot->IsAlive() || bot->HasAura(SPELL_MARK) ||
        !AnubarakEngaged(GET_PLAYERBOT_AI(bot)))
        return nullptr;

    AnubarakState* state = anubarakStates.Find(bot->GetInstanceId());
    if (!state)
        return nullptr;

    auto const it = state->dodgeHolds.find(bot->GetGUID());
    if (it == state->dodgeHolds.end())
        return nullptr;

    DodgeHold& hold = it->second;
    uint32 const heldMs = getMSTimeDiff(hold.setMs, getMSTime());
    if (heldMs >= ANUBARAK_DODGE_HOLD_MS ||
        (heldMs >= ANUBARAK_DODGE_WALK_MS && Dist2d(bot->GetPosition(), hold.spot) > ANUBARAK_DODGE_HOLD_TRIGGER))
        return nullptr;

    // The spike idles 4 s between marks with nobody marked, so an empty kiter is a kiter too
    Creature* spike = GetPursuingSpike(bot);
    if (!spike)
        return nullptr;

    Player* kiter = GetSpikeTarget(bot);
    if ((kiter ? kiter->GetGUID() : ObjectGuid::Empty) != hold.kiter)
        return nullptr;

    // Kept until the gather slot and the walk back clear the path too: let go once it stood clear, a bot
    // walked straight back into it
    SpikeDodgeReach const reach = DodgeReach(bot, spike);
    std::vector<AnubarakPursuit::Point> const& track = DodgeTrack(bot, spike, *state);
    if (InDodgeDanger(spike, track, reach, hold.spot) || DodgeWalkClear(spike, track, reach, hold.spot, hold.home))
        return nullptr;

    return &hold;
}

bool GetAnubarakDodgeHold(Player* bot, Position& spot)
{
    DodgeHold const* hold = ValidDodgeHold(bot);
    if (!hold)
        return false;

    spot = hold->spot;
    return true;
}

void SetAnubarakDodgeHold(Player* bot, Position const& spot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state || !GetPursuingSpike(bot))
        return;

    Player* kiter = GetSpikeTarget(bot);
    DodgeHold& hold = state->dodgeHolds[bot->GetGUID()];
    hold.spot = spot;
    hold.home = DodgeHome(bot);
    hold.kiter = kiter ? kiter->GetGUID() : ObjectGuid::Empty;
    hold.setMs = getMSTime();
    hold.arrived = false;
}

bool IsAnubarakDodgeHeld(PlayerbotAI* botAI)
{
    Position spot;
    return botAI && GetAnubarakDodgeHold(botAI->GetBot(), spot);
}

bool AnubarakDodgeWalking(Player* bot)
{
    DodgeHold* hold = ValidDodgeHold(bot);
    if (!hold || hold->arrived)
        return false;

    hold->arrived = Dist2d(bot->GetPosition(), hold->spot) <= ANUBARAK_DODGE_ARRIVE;
    return !hold->arrived;
}

bool AnubarakWalkClearOfSpike(Player* bot, Position const& to)
{
    Creature* spike = bot ? GetPursuingSpike(bot) : nullptr;
    AnubarakState* state = spike ? anubarakStates.Find(bot->GetInstanceId()) : nullptr;
    return !state ||
           DodgeWalkClear(spike, DodgeTrack(bot, spike, *state), DodgeReach(bot, spike), bot->GetPosition(), to);
}

Player* GetAnubarakKiterToProtect(Player* bot)
{
    ProtectClaim const* claim = DeriveProtection(bot);
    if (!claim)
        return nullptr;

    char const* role = ProtectRole(bot, *claim);
    if (std::strcmp(role, "hop") != 0)
        return nullptr;

    Player* kiter = ObjectAccessor::GetPlayer(*bot, claim->kiter);
    return kiter && kiter->IsAlive() ? kiter : nullptr;
}

bool AnubarakKiterNeedsImmunity(Player* bot)
{
    ProtectClaim const* claim = DeriveProtection(bot);
    if (!claim)
        return false;

    char const* role = ProtectRole(bot, *claim);
    return std::strcmp(role, "self") == 0;
}

bool AnubarakKiterKeepsMark(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    Creature* spike = state ? GetPursuingSpike(bot) : nullptr;
    if (!spike)
        return false;

    if (bot->HasAura(SPELL_MARK))
    {
        KitePlan const& plan = PlanKite(bot, spike, *state);
        if (plan.candidate.own)
            return false;

        if ((HandKite(plan) && plan.hand == bot->GetGUID()) || plan.candidate.patch >= 0)
            return true;
    }

    if (KitePatchAvailable(bot, *state, true))
        return true;

    int32 const toEmerge = static_cast<int32>(state->submergedMs + ANUBARAK_SPIKE_LIFE_MS - getMSTime());
    return state->phaseValue == AnubarakPhase::Submerged && toEmerge <= static_cast<int32>(ANUBARAK_KITE_HOP_EMERGE_MS);
}

bool GetAnubarakRedirect(Player* bot, AnubarakRedirect& out)
{
    out = AnubarakRedirect();
    uint8 const cls = bot ? bot->getClass() : 0;
    PlayerbotAI* botAI = cls == CLASS_HUNTER || cls == CLASS_ROGUE ? GET_PLAYERBOT_AI(bot) : nullptr;
    AnubarakState* state = botAI ? ReadPhase(botAI) : nullptr;
    if (!state || !bot->IsAlive() || bot->HasAura(SPELL_MARK))
        return false;

    RedirectRoster const roster = state->redirect.Get(bot, [bot](RedirectRoster& found)
    {
        ForEachGroupMember(bot, [&](Player* member)
        {
            if (!member->IsAlive() || !GET_PLAYERBOT_AI(member))
                return false;

            if (member->getClass() == CLASS_HUNTER)
                found.hunters.push_back(member->GetGUID());
            else if (member->getClass() == CLASS_ROGUE && found.firstRogue.IsEmpty())
                found.firstRogue = member->GetGUID();

            return false;
        });
    });

    ObjectGuid const firstHunter = roster.hunters.empty() ? ObjectGuid::Empty : roster.hunters.front();
    bool const pull = getMSTimeDiff(state->pullMs, getMSTime()) < ANUBARAK_REDIRECT_PULL_MS;
    if (pull && (bot->GetGUID() == firstHunter || bot->GetGUID() == roster.firstRogue))
    {
        Player* tank = state->mainTank.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(*bot, state->mainTank);
        if (!tank || !tank->IsAlive() || tank == bot)
            return false;

        out.tank = tank;
        out.dump = GetAnubarak(bot);
        return true;
    }

    // Selectable burrowers not on the bot side tank they belong to, by guid
    std::vector<std::pair<Creature*, Player*>> loose;
    std::unordered_map<ObjectGuid, uint8> const& owners = BurrowerOwners(bot, *state);
    for (Creature* burrower : ResolveAlive(bot, state->burrowers))
    {
        auto const owner = owners.find(burrower->GetGUID());
        if (burrower->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || owner == owners.end())
            continue;

        Player* tank = UsableSideTank(bot, *state, owner->second);
        if (tank && GET_PLAYERBOT_AI(tank) && burrower->GetVictim() != tank)
            loose.emplace_back(burrower, tank);
    }

    std::sort(loose.begin(), loose.end(),
              [](auto const& a, auto const& b) { return a.first->GetGUID() < b.first->GetGUID(); });

    // Tricks only moves the threat of what the rogue hits, so it goes out on the burrower it's on
    if (cls == CLASS_ROGUE)
    {
        Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        for (auto const& [burrower, tank] : loose)
        {
            if (burrower != current || !bot->IsWithinDistInMap(tank, ANUBARAK_TRICKS_RANGE))
                continue;

            out.tank = tank;
            out.dump = burrower;
            return true;
        }

        return false;
    }

    // One hunter a burrower, in group order, the pull's Misdirection aside
    size_t index = 0;
    for (ObjectGuid const& hunter : roster.hunters)
    {
        if (hunter == bot->GetGUID())
            break;

        if (!(pull && hunter == firstHunter))
            ++index;
    }

    if (index >= loose.size())
        return false;

    out.tank = loose[index].second;
    out.dump = loose[index].first;
    return true;
}

bool AnubarakKiterImmune(Unit* kiter)
{
    if (!kiter)
        return false;

    for (uint32 spell : ANUBARAK_KITE_IMMUNITIES)
        if (kiter->HasAura(spell))
            return true;

    return false;
}

Creature* GetAnubarakShadowStrikeDuty(Player* bot)
{
    AnubarakState* state = bot ? ReadPhase(GET_PLAYERBOT_AI(bot)) : nullptr;
    if (!state)
        return nullptr;

    Group* group = bot->GetGroup();
    ObjectGuid const groupGuid = group ? group->GetGUID() : bot->GetGUID();
    uint32 const now = getMSTime();
    if (!state->strikeMemoValid || state->strikeMemoMs != now || state->strikeGroup != groupGuid)
    {
        state->strikeMemoValid = true;
        state->strikeMemoMs = now;
        state->strikeGroup = groupGuid;
        ComputeStrikeDuties(bot, *state);
    }

    for (StrikeDuty const& duty : state->strikes)
    {
        if (duty.interrupter != bot->GetGUID())
            continue;

        Creature* caster = ResolveCreature(bot, duty.caster);
        if (caster && caster->IsAlive() && caster->FindCurrentSpellBySpellId(SPELL_SHADOW_STRIKE))
            return caster;
    }

    return nullptr;
}

char const* AnubarakReadyInterrupt(Player* bot, Unit* target)
{
    if (!bot || !target)
        return nullptr;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    // Holy Wrath stuns undead, and every burrower difficulty entry is undead
    static char const* const interrupts[] = {"kick",       "pummel",          "shield bash", "counterspell",
                                             "wind shear", "mind freeze",     "hammer of justice", "holy wrath",
                                             "shockwave",  "concussion blow", "bash",        "war stomp",
                                             "shadowfury"};

    for (char const* interrupt : interrupts)
        if (botAI->CanCastSpell(interrupt, target))
            return interrupt;

    return nullptr;
}

}
