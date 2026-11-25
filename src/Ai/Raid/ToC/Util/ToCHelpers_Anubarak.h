/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_ANUBARAK_H
#define PLAYERBOTS_RAID_TOCHELPERS_ANUBARAK_H

#include <vector>

#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "ToCData.h"

class Creature;
class Player;
class Unit;

namespace TrialOfTheCrusaderHelpers
{

// On a flying Frost Sphere only, lost when it lands as a Permafrost patch
inline constexpr uint32 SPELL_FROST_SPHERE = 67539;
// 10N ids, remap through sSpellMgr->GetSpellIdForDifficulty at the call
inline constexpr uint32 SPELL_PERMAFROST = 66193;
inline constexpr uint32 SPELL_PENETRATING_COLD = 66013;
// Heroic burrower cast, no difficulty row
inline constexpr uint32 SPELL_SHADOW_STRIKE = 66134;
// On the burrower's victim, +25% physical damage taken a stack. 10N id, remap it at the call
inline constexpr uint32 SPELL_EXPOSE_WEAKNESS = 67721;
inline constexpr uint32 SPELL_DEATH_GRIP = 49576;

// Middle of the open floor, walkable out to 50 yd
extern const Position ANUBARAK_ROOM_CENTER;
// Sphere and spike sweeps reach the whole room from anywhere in it
inline constexpr float ANUBARAK_ROOM_RADIUS = 120.0f;
// Tank spots sit at ROOM_CENTER.y +/- this, 24 yd apart: burrowers held 4 yd inside each patch toward him
// stay 16 yd apart, past Spider Frenzy's 12, and slot 3's sphere starts in grip reach of the north spot
inline constexpr float ANUBARAK_TANK_PATCH_OFFSET = 12.0f;
// Furthest a patch can be from its side point and still count as that side's tank patch
inline constexpr float ANUBARAK_TANK_PATCH_LATCH = 30.0f;
inline constexpr float ANUBARAK_TANK_PATCH_ARRIVE = 2.0f;
// 6 yd aura plus 1.5 player reach
inline constexpr float ANUBARAK_PERMAFROST_SLOW_REACH = 7.5f;
// The main tank keeps this far off every patch, so a step or a drag never ends in the slow
inline constexpr float ANUBARAK_TANK_KEEPOUT = ANUBARAK_PERMAFROST_SLOW_REACH + 1.0f;
// Pre-pull pickup stand, west of the room centre: he runs in from the east and stops on the anchor
inline constexpr float ANUBARAK_PICKUP_STAND_OFFSET = 10.0f;

// Shooting a tank patch: a sphere this close to the spot goes at once, the nearest within FAR after the wait
inline constexpr float ANUBARAK_SIDE_SHOOT_NEAR = 12.0f;
inline constexpr float ANUBARAK_SIDE_SHOOT_FAR = 20.0f;
inline constexpr uint32 ANUBARAK_SIDE_SHOOT_WAIT_MS = 10000;
// A sphere this close to the other tank patch would put both sides' burrowers, 4 yd in toward him, in
// one Spider Frenzy
inline constexpr float ANUBARAK_TANK_PATCH_GAP = 20.0f;

// Death Grip's 30 yd less a margin, 3D: a flying sphere sits about 13.5 yd up
inline constexpr float ANUBARAK_GRIP_RANGE = 28.0f;
// A gripped sphere comes down where the DK cast from. One not landed 3 s after the cast failed, and its
// side shoots until a Death Grip is back.
inline constexpr float ANUBARAK_GRIP_LANDED = 3.0f;
inline constexpr float ANUBARAK_GRIP_LANDED_HEIGHT = 4.0f;
inline constexpr uint32 ANUBARAK_GRIP_JUDGE_MS = 3000;
// A DK whose grip is back within this still grips; any later and the side shoots instead
inline constexpr uint32 ANUBARAK_GRIP_COOLDOWN_WAIT_MS = 3000;
// Only one sphere flies near the tank spots, so with none in reach the DK grips from up to this far off
// its spot, toward the nearest. The landing still counts within ANUBARAK_SIDE_SHOOT_FAR.
inline constexpr float ANUBARAK_GRIP_STEP_OUT = 16.0f;
// The stepped-out stand puts the sphere this far inside reach, since it keeps drifting
inline constexpr float ANUBARAK_GRIP_REACH_MARGIN = 3.0f;

// With its patch latched a side tank holds this far from its centre toward him, once every burrower on
// it carries Permafrost. Burrowers stop about 3 yd short, 3.5-5.7 yd in from every exit.
inline constexpr float ANUBARAK_SIDE_TANK_RIM = 7.0f;
// A melee bot hits a burrower from within this of it, at least this far from every patch centre: out of
// the 7.5 yd slow
inline constexpr float ANUBARAK_MELEE_BURROWER_REACH = 4.5f;
inline constexpr float ANUBARAK_MELEE_PATCH_CLEARANCE = 8.0f;
// Heroic phase 3 waves sit 2.5-3.5 yd into their patch, out of that ring's reach. There melee stand this
// much inside the burrower's melee range, just past the slow's 7.47 yd.
inline constexpr float ANUBARAK_SWARM_MELEE_REACH_MARGIN = 0.25f;
inline constexpr float ANUBARAK_SWARM_MELEE_PATCH_CLEARANCE = 7.9f;

// Burrowers a side tank holds before the next one goes to the other side (Spider Frenzy)
inline constexpr uint8 ANUBARAK_BURROWERS_PER_SIDE = 2;
// Burrower waves: 5-8 s after the pull or an emerge, then every 45 s, none while he is down
inline constexpr uint32 ANUBARAK_WAVE_FIRST_MS = 5000;
inline constexpr uint32 ANUBARAK_WAVE_INTERVAL_MS = 45000;
// New burrowers further apart than this belong to separate waves
inline constexpr uint32 ANUBARAK_WAVE_GAP_MS = 10000;
// Side tanks leave scarabs alone this long before a wave
inline constexpr uint32 ANUBARAK_WAVE_READY_MS = 8000;
// He submerges 80 s after the pull or an emerge and is selectable again 62 s later
inline constexpr uint32 ANUBARAK_SURFACED_MS = 80000;
inline constexpr uint32 ANUBARAK_SUBMERGED_MS = 62000;
// One this low off Permafrost still dies before its next 3 s submerge check
inline constexpr float ANUBARAK_FOCUS_OFF_FROST_PCT = 10.0f;
// A burrower is a focus pick once a tank it is on has stood this close this long: before, it is still
// walking in from its exit and the raid would chase it there, or pull it off the tank
inline constexpr float ANUBARAK_FOCUS_SETTLE = 5.0f;
inline constexpr uint32 ANUBARAK_FOCUS_SETTLE_MS = 4000;
// A burrower on a tank this deep in Expose Weakness is a pick at once, before more stacks get the tank killed
inline constexpr uint32 ANUBARAK_FOCUS_STACKS = 6;
// The script's first submerge check comes 30 s after a burrower spawns or emerges, so one off Permafrost
// is still worth killing this long after either
inline constexpr uint32 ANUBARAK_BURROWER_FREE_MS = 25000;

inline constexpr uint32 ANUBARAK_SIDE_DEFENSIVE_STACKS = 5;
inline constexpr float ANUBARAK_SIDE_DEFENSIVE_PCT = 50.0f;
// Heroic, from his health at this until phase 3, a side tank's long saves wait. One spent on a stack at
// full health in the last phase 1 is still down when phase 3's waves come.
inline constexpr float ANUBARAK_SIDE_DEFENSIVE_KEEP_PCT = 50.0f;
inline constexpr uint32 ANUBARAK_SIDE_DEFENSIVE_KEEP_COOLDOWN_MS = 120000;
// Rebirth's 30 yd less a margin
inline constexpr float ANUBARAK_BATTLE_RES_REACH = 28.0f;
// A ranged or healer bot a burrower is on walks toward its side tank once over REACH off, the 30 yd taunt
// less a margin, and stops STOP off it. Re-aimed once the point moves REAIM.
inline constexpr float ANUBARAK_LEAD_REACH = 25.0f;
inline constexpr float ANUBARAK_LEAD_STOP = 20.0f;
inline constexpr float ANUBARAK_LEAD_REAIM = 3.0f;
// A side tank's 3 s taunt from past ARRIVE runs out as the burrower (about 13 yd/s) gets there, and it goes to
// the top threat, usually a healer. Under RESCUE_PCT the player it hits gets the taunt anyway.
inline constexpr float ANUBARAK_TAUNT_ARRIVE = 40.0f;
inline constexpr float ANUBARAK_TAUNT_RESCUE_PCT = 50.0f;
// The spike's 8 yd sphere test adds both sizes: 1.5 spike, 2.0 patch, 1.0 falling sphere. So 11.5 yd
// round a patch and 10.5 round a falling one.
inline constexpr float ANUBARAK_SPIKE_PATCH_REACH = 11.5f;
// Past the patch on the far side from the spike: it fails 11.5 yd out, long before Impale's 6 + 1.5
// can reach a kiter standing here
inline constexpr float ANUBARAK_KITE_STAND_OFFSET = 8.5f;
inline constexpr float ANUBARAK_KITE_ARRIVE = 2.0f;
// Walkable all round, as is the 45 yd rim; the kite planner's ring paths run on both
inline constexpr float ANUBARAK_KITE_RING_RADIUS = 40.0f;
// Heroic: the spike this close and faster than the kiter sends the next speed boost out, and at CLOSE
// whatever its speed
inline constexpr float ANUBARAK_KITE_BOOST_RANGE = 15.0f;
inline constexpr float ANUBARAK_KITE_BOOST_CLOSE = 10.0f;
// Heroic: the spike's route keeps this far off a patch it isn't meant to take, its 11.5 yd test plus 3
inline constexpr float ANUBARAK_SPIKE_ROUTE_CLEARANCE = 14.5f;
// Impale's 6 yd plus 1.5 reach
inline constexpr float ANUBARAK_IMPALE_REACH = 7.5f;
// The teleport back to the Demonic Circle only goes when the circle is this much farther from the spike
inline constexpr float ANUBARAK_CIRCLE_TELEPORT_GAIN = 15.0f;
// A circle with less left than this is cast again
inline constexpr int32 ANUBARAK_CIRCLE_REFRESH_MS = 100000;
// Heroic pre-pull: the circle goes down this close to the room centre, where the teleport reaches the whole
// 38 yd kite ring, with this much left so it lasts to the second emerge 284 s in
inline constexpr float ANUBARAK_CIRCLE_CENTRE_SLACK = 2.0f;
inline constexpr int32 ANUBARAK_CIRCLE_PREPULL_MS = 290000;
inline constexpr float ANUBARAK_BLINK_DISTANCE = 20.0f;
// Heroic phase 2: slots this far round the raid point, held from up to ARRIVE off them. The healer stand
// uses the same ring while he is up, and walks back once a healer is LEAVE off its slot.
inline constexpr float ANUBARAK_GATHER_ARRIVE = 6.0f;
inline constexpr float ANUBARAK_GATHER_SPREAD = 3.0f;
inline constexpr float ANUBARAK_HEALER_STAND_LEAVE = 6.0f;
// A gathered melee bot takes a scarab this close before the focus burrower
inline constexpr float ANUBARAK_GATHER_MELEE_SCARAB = 10.0f;
inline constexpr float ANUBARAK_SPIKE_DANGER_RADIUS = 10.0f;
inline constexpr float ANUBARAK_SPIKE_CLEARANCE = 13.0f;
inline constexpr float ANUBARAK_SPIKE_LANE_HALF_WIDTH = 7.0f;
inline constexpr float ANUBARAK_SPIKE_LANE_CLEARANCE = 9.0f;
// A spike faster than the bot: 10 yd is under a second at 11.4 yd/s
inline constexpr float ANUBARAK_SPIKE_DANGER_RADIUS_FAST = 16.0f;
inline constexpr float ANUBARAK_SPIKE_CLEARANCE_FAST = 19.0f;
inline constexpr float ANUBARAK_SPIKE_LANE_HALF_WIDTH_FAST = 10.0f;
inline constexpr float ANUBARAK_SPIKE_LANE_CLEARANCE_FAST = 12.0f;
inline constexpr float ANUBARAK_SPIKE_DODGE_SEARCH = 30.0f;
inline constexpr float ANUBARAK_INTERRUPT_RANGE = 30.0f;
// He submerges 80 s after the pull or an emerge and the spike is summoned on him, so he is dragged off
// the patches 15 s before
inline constexpr uint32 ANUBARAK_SUBMERGE_PREP_MS = 65000;

// Drag deadband, his distance from the point. In the submerge window DONE is also the start radius.
inline constexpr float ANUBARAK_DRAG_START = 8.0f;
inline constexpr float ANUBARAK_DRAG_DONE = 4.0f;
// He has stood still past his 400 ms chase recheck
inline constexpr uint32 ANUBARAK_DRAG_SETTLE_MS = 500;
// A leg he has not settled from by then is dropped
inline constexpr uint32 ANUBARAK_DRAG_LEG_MS = 6000;
// Spike reach round a patch, plus its 3.5 yd/s * 1.5 s first walk, plus the drag's done radius
inline constexpr float ANUBARAK_SUBMERGE_PATCH_CLEARANCE = ANUBARAK_SPIKE_PATCH_REACH + 5.25f + ANUBARAK_DRAG_DONE;
// Last search when no spot clears the others
inline constexpr float ANUBARAK_SUBMERGE_PATCH_CLEARANCE_MIN = ANUBARAK_SPIKE_PATCH_REACH + 5.25f;

// DPS bots stop on him at this share of his tank victim's threat, resume under the release share
inline constexpr float ANUBARAK_THREAT_CAP = 0.9f;
inline constexpr float ANUBARAK_THREAT_RELEASE = 0.8f;
// A side tank's area threat leaves him out at this share of his tank victim's threat, until under the
// release share
inline constexpr float ANUBARAK_SIDE_BOSS_CAP = 0.7f;
inline constexpr float ANUBARAK_SIDE_BOSS_RELEASE = 0.6f;
// A melee DPS bot's area damage stops near a burrower it has this share of the side tank's threat on,
// until under the release share
inline constexpr float ANUBARAK_MELEE_AREA_CAP = 0.6f;
inline constexpr float ANUBARAK_MELEE_AREA_RELEASE = 0.5f;

inline constexpr uint8 ANUBARAK_SHOOTERS_PER_NEED = 2;
// A kite patch stocked while he's up has until the submerge, so the rest of the ranged stay on the burrower
// wave landing then
inline constexpr uint8 ANUBARAK_SHOOTERS_PER_STOCK_UP = 1;
// Non-tank patches kept ready in phase 2, falling spheres counted. Heroic never respawns its six spheres, so it
// keeps one submerge's two down together and shoots no more once both are.
inline constexpr uint8 ANUBARAK_P2_PATCH_STOCK = 2;
inline constexpr uint8 ANUBARAK_P2_PATCH_STOCK_HEROIC = 2;
// A shooter reaches a sphere within its "spell" range less this, 3D, in line of sight
inline constexpr float ANUBARAK_SHOOT_MARGIN = 2.0f;

inline constexpr float ANUBARAK_DODGE_ARRIVE = 2.0f;
// A held bot further off its spot lets go
inline constexpr float ANUBARAK_DODGE_HOLD_TRIGGER = 2.5f;
inline constexpr uint32 ANUBARAK_DODGE_WALK_MS = 3000;
inline constexpr uint32 ANUBARAK_DODGE_HOLD_MS = 8000;
// How far ahead the dodge reads the spike's path: 34 yd of warning at its top speed
inline constexpr float ANUBARAK_DODGE_TRACK_S = 3.0f;

// The escort walks back to its spot once further off it
inline constexpr float ANUBARAK_ESCORT_HOLD = 4.0f;
// Impale's 6 + 1.5 plus a margin: the kiter's own immunity
inline constexpr float ANUBARAK_KITE_IMMUNITY_RANGE = 9.0f;
inline constexpr float ANUBARAK_HOP_RANGE = 30.0f;
inline constexpr uint32 SPELL_FORBEARANCE = 25771;
// The kiter's own Hand of Protection keeps the spike off it within this much of the emerge
inline constexpr uint32 ANUBARAK_KITE_HOP_EMERGE_MS = 10000;
// Hand of Protection ranks, Divine Shield, Ice Block. Only the hand keeps the mark; the others, Feign
// Death and Vanish hand the spike on at its current speed.
inline constexpr uint32 ANUBARAK_KITE_IMMUNITIES[] = {1022, 5599, 10278, 642, 45438};

enum class AnubarakPhase : uint8
{
    None = 0,
    Surface = 1,
    Submerged = 2,
    Swarm = 3
};

enum class AnubarakKiteBranch : uint8
{
    None,
    // Walking the plan's patch ending
    Patch,
    // Running the plan's path
    Ring,
    // At the end of the patch walk
    Hold
};

// Heroic only
enum class AnubarakKiteStage : uint8
{
    None,
    Run,
    // Running, and the next speed boost goes out
    Boost,
    // Hand of Protection up, still running
    Immune,
    // The plan's walk onto its patch has begun
    Patch
};

struct AnubarakKiteBoost
{
    uint32 spellId = 0;
    // Nitro Boosts is the boots' use, no spell to cast
    bool nitro = false;
    // Blink goes the way the bot faces
    bool face = false;
    float facing = 0.0f;
    // Dash out of another form: Cat Form first, which no other form can cast
    bool catForm = false;
    // Feral Charge goes at this bug
    ObjectGuid target;
};

struct AnubarakKiteStand
{
    Position stand;
    ObjectGuid patch;
    AnubarakKiteBranch branch = AnubarakKiteBranch::None;
};

enum class AnubarakDragStep : uint8
{
    None,
    Walk,
    Settle
};

struct AnubarakDragLeg
{
    AnubarakDragStep step = AnubarakDragStep::None;
    Position tankSpot;
    // The walk is to a pre-step round him, the leg's spot after it
    bool preStep = false;
};

struct AnubarakBurrowerPick
{
    // On this side tank: one still on its taunt first, then its current target
    Creature* held = nullptr;
    // held is on this tank's taunt. Once that ends it goes to whoever has the most threat, so it needs
    // hitting before then.
    bool heldTaunted = false;
    // This side's, on someone else: one hitting a healer first, then the nearest the hold
    Creature* loose = nullptr;
    // GetAnubarakSideTankHold
    Position hold;
    uint8 side = 0;
    // Loose ones within 10 yd of the bot, for a challenging shout
    uint8 looseNear = 0;
    // One of the other side's within 10 yd, which a shout would take too
    bool otherNear = false;
    // The group member most of this side's loose ones hit, for Righteous Defense
    Player* rescue = nullptr;
    // Yards from the bot to the farthest of this side's loose ones on rescue
    float rescueFar = 0.0f;

    explicit operator bool() const { return held || loose; }
};

// A burrower on a ranged DPS or healer bot, and the usable side tank that owns it
struct AnubarakBurrowerLead
{
    Player* tank = nullptr;
    // The tank is over ANUBARAK_LEAD_REACH off: walk to `to`, ANUBARAK_LEAD_STOP short of it
    bool walk = false;
    Position to;
};

// A side tank's area or chain cast that takes in both of its burrowers and none of the other side's
struct AnubarakSideTankAoe
{
    char const* spell = nullptr;
    // The cast's target: a burrower of its own, the one Pestilence spreads from
    Unit* target = nullptr;
    // Death and Decay goes down at pos
    bool point = false;
    Position pos;
};

enum class AnubarakGripStep : uint8
{
    None,
    // To its stand
    Walk,
    // On its stand with nothing in reach, the gripped sphere on its way, or a relay link's grip still to come
    Hold,
    Cast,
    // The gripped sphere landed on the DK
    Kill
};

struct AnubarakGripDuty
{
    // Null while no sphere is in reach of the stand
    Creature* sphere = nullptr;
    // The stand: the side's spot, up to ANUBARAK_GRIP_STEP_OUT off it toward the sphere, with every
    // sphere further out just inside Death Grip's reach of the nearest, or a relay link's post
    Position spot;
    uint8 side = 0;
    AnubarakGripStep step = AnubarakGripStep::None;
};

// Through the instance's guid slot, so he still resolves while submerged and unselectable
Creature* GetAnubarak(Player* bot);
// He's attackable before the pull and the stage gate opens at the floor break, so only his combat counts
bool AnubarakEngaged(PlayerbotAI* botAI);
// None while not engaged. A read 5 s after the last one starts a new pull, which clears every latch.
AnubarakPhase GetAnubarakPhase(PlayerbotAI* botAI);
bool AnubarakLeechingSwarmActive(PlayerbotAI* botAI);
// Heroic phase 3, where a heal on a healthy non-tank goes back to him through Leeching Swarm
bool AnubarakSwarmHealing(PlayerbotAI* botAI);
// Heroic burst other than lust waits for phase 3 from 50%, unless he's losing health too slowly to reach
// 30% before he submerges; in phase 3 until the burrowers up at its start are dead, 15 s at most
bool AnubarakBurstHeld(PlayerbotAI* botAI);
// Phase 3; on heroic once those burrowers are dead too, 15 s at most
bool AnubarakLustOpen(PlayerbotAI* botAI);

// Unselectable, so only a grid search finds it
Creature* GetPursuingSpike(Player* bot);
Player* GetSpikeTarget(Player* bot);
// Spot sits within ANUBARAK_SPIKE_LANE_CLEARANCE of the spike's line to its target
bool AnubarakSpikeLaneCrosses(Player* bot, Position const& spot);
// The spike could run to spot within the bot's walk there plus 2 s
bool AnubarakSpikeReachesFirst(Player* bot, Position const& spot);

bool IsFrostSphereFlying(Unit* sphere);
// Killed but still dropping, turns into a patch 1.5 s after the kill
bool IsFrostSphereFalling(Unit* sphere);
// A patch never carries Permafrost itself, its own area aura skips the caster
bool IsPermafrostPatch(Unit* sphere);
// Every alive sphere: flying, falling and patches
std::vector<Creature*> GetFrostSpheres(Player* bot);
bool UnitOnPermafrost(Unit* unit);

// Side 0 sits at ROOM_CENTER.y + offset, side 1 at y - offset
Position GetAnubarakTankSidePoint(uint8 side);
// Null until a patch lies within the latch distance of the side point, nearer it than the other side's
Creature* GetAnubarakTankPatch(Player* bot, uint8 side);
// The side's patch, else its side point moved with the other side's patch so the pair keeps its spacing
Position GetAnubarakTankSpot(Player* bot, uint8 side);
// Where the side tank stands: ANUBARAK_SIDE_TANK_RIM from its patch toward him once every burrower on it
// carries Permafrost, the patch centre while one doesn't, so it trails in; no patch, the spot
Position GetAnubarakSideTankHold(Player* bot, uint8 side);
// Midpoint of both tank patches, else the room centre
Position GetAnubarakBossAnchor(Player* bot);
// Pre-pull stand of the pickup tank, west of the anchor and off both tank spots: heroic's anchor is the
// east pair's until both patches latch
Position GetAnubarakPickupStand(Player* bot);
// Pre-pull stand of a side tank: its side point, heroic the east pair's spot
Position GetAnubarakPrepullSideStand(Player* bot, uint8 side);
// Midpoint of both latched tank patches. False until both have landed: he is held where picked up.
bool GetAnubarakDragAnchor(Player* bot, Position& out);
// The anchor first, then fallbacks for a leg through Permafrost, a patch on his line or a stall. Empty without
// an anchor. Called by the hold action only, since it times the stall.
std::vector<Position> GetAnubarakDragPoints(Player* bot, Creature* boss);
// From 65 s after the pull or an emerge: a spot 30 yd off every patch, stepping down to the _MIN clearance,
// so the spike summoned on him burns none, on the tank's side away from the tank patches. False otherwise.
bool GetAnubarakSubmergeSpot(Player* bot, Position& out);
// The submerge spot, then waypoints round each patch on his line to it
std::vector<Position> GetAnubarakSubmergeDragPoints(Player* bot, Creature* boss, Position const& spot);
// The pickup tank's leg toward point, latched per instance. Called by the hold action only. No leg
// whose tank spot or walk comes within the slow reach of a patch.
AnubarakDragLeg GetAnubarakDragLeg(Player* bot, Creature* boss, Position const& point, bool submerge);
// The main tank stands in a patch's slow reach: the nearest spot clear of every patch by
// ANUBARAK_TANK_KEEPOUT, within his melee reach when there is one. False when it is clear already.
bool GetAnubarakTankStepOut(Player* bot, Creature* boss, Position& out);
bool AnubarakTankPatchLatched(Player* bot);
void ClearAnubarakDrag(Player* bot);
// A latched leg is walking or waiting for him to settle: the Block rule's predicate, a pure read
bool IsAnubarakDragInFlight(PlayerbotAI* botAI);

// Current target is him and his victim is a tank other than this bot
bool AnubarakTauntWouldSteal(PlayerbotAI* botAI);
// Latched per bot: capped at ANUBARAK_THREAT_CAP of his tank victim's threat, freed under RELEASE
bool AnubarakThreatCapped(Player* bot);
// The same for a side tank's area threat on him: capped at ANUBARAK_SIDE_BOSS_CAP, freed under its release
bool AnubarakSideTankBossCapped(Player* bot);
// While he is up, a side tank with two of its burrowers near: the first ready spell whose hit set, read the
// core's way, takes in two of its own, none of the other side's, and him only while not capped
bool GetAnubarakSideTankAoe(PlayerbotAI* botAI, AnubarakSideTankAoe& out);
// False for a side tank's area or chain spell aimed at target that fails the same test, or another area
// spell with one of the other side's burrowers near
bool AnubarakSideTankAreaAllowed(PlayerbotAI* botAI, std::string const& spell, Unit* target, bool area);
// A melee DPS bot's running Blade Flurry or Sweeping Strikes, with a burrower in swing reach it is over
// the area share on, else null
char const* GetAnubarakCleaveBuffToDrop(PlayerbotAI* botAI);
// The same per DPS bot and burrower, on a burrower a tank holds
bool AnubarakBurrowerThreatCapped(Player* bot, Unit* burrower);
// A selectable burrower within range of at that the bot is capped on
bool AnubarakBurrowerThreatCappedNear(Player* bot, Position const& at, float range);
// Latched per melee DPS bot and burrower a tank holds: at ANUBARAK_MELEE_AREA_CAP of the tank's threat,
// freed under its release. Area damage, Blade Flurry and Sweeping Strikes wait while one is near.
bool AnubarakBurrowerAreaCapped(Player* bot, Unit* burrower);
// One such burrower within range plus its combat reach of at
bool AnubarakBurrowerAreaCappedNear(Player* bot, Position const& at, float range);
// 0 or 1 for the two side tanks latched for the pull, else -1. Bots go first; a human takes a side no
// bot can, and his side is the tank spot he stands at.
int8 GetAnubarakBurrowerTankSide(Player* player);
// Expose Weakness at ANUBARAK_SIDE_DEFENSIVE_STACKS or health under the pct, on a side tank
bool AnubarakSideTankNeedsDefensive(Player* bot);
// The side tank's defensive pick waits for phase 3: ANUBARAK_SIDE_DEFENSIVE_KEEP_PCT, a cooldown of
// ANUBARAK_SIDE_DEFENSIVE_KEEP_COOLDOWN_MS or more, the tank at or over ANUBARAK_SIDE_DEFENSIVE_PCT
bool AnubarakSideTankDefensiveKept(PlayerbotAI* botAI, uint32 spellId);
// Phase 3: a side tank whose target is a burrower on it, or one not on it over ANUBARAK_TAUNT_ARRIVE from its
// hold. That one comes in anyway, and walking out to meet it takes the tank out of heal reach under the swarm.
bool AnubarakSideTankSwarmLeash(PlayerbotAI* botAI);
// Phase 3, a living non-tank bot with no Penetrating Cold and no burrower on it: the swarm alone can't
// kill it, yet keeps it at critical health, so its class saves and heal items would all go at once
bool AnubarakSwarmSavesHeld(PlayerbotAI* botAI);
// Phase 3, a non-healer bot carrying Penetrating Cold with a tick still due: the first of its own saves it can
// cast, a tank only Anti-Magic Shell and Divine Protection. 0 with none, or while one is already up.
uint32 GetAnubarakColdSelfSave(PlayerbotAI* botAI);
// Ice Block and Divine Shield remove Penetrating Cold as they go up, so the save cancels them right after
bool IsAnubarakColdBubble(uint32 spellId);
bool AnubarakColdBubbleUp(Player* bot);
// Who this druid can Rebirth now: a dead side tank, else the lowest guid dead healer bot, else null. One
// spent on a healer isn't there for a side tank that dies later.
Player* GetAnubarakBattleResTarget(PlayerbotAI* botAI);
// A burrower wave is due within ANUBARAK_WAVE_READY_MS while he is up
bool AnubarakWaveImminent(PlayerbotAI* botAI);
// Main tank while alive, else the side tank he's on, else the first bot side tank
bool IsAnubarakPickupTank(Player* bot);
// IsTank can read false for a tank for a tick, so the spec is asked too
bool IsAnubarakTankPlayer(Player* player);

// Who the pull's latches would pick if it started now, read fresh with nothing latched. pickup is
// the main tank while alive, else the first bot side tank, which then leaves its burrowers to the other.
struct AnubarakPrepullTanks
{
    ObjectGuid pickup;
    ObjectGuid side[2];
};
AnubarakPrepullTanks GetAnubarakPrepullTanks(Player* bot);

// Alive burrowers in the room, submerged ones too: they come back up where they went down
std::vector<Creature*> GetAnubarakBurrowers(Player* bot);
// What a side tank takes, empty with no side: two a side by burrow exit, and one on a side tank stays with it.
// A dead, marked or boss-holding side tank's go to the other, as do a human's still loose after 5 s, past two.
AnubarakBurrowerPick GetAnubarakBurrowerPick(PlayerbotAI* botAI);
// A selectable burrower is on this ranged DPS or healer bot and its owner has a usable side tank, the nearest
// when several. Not for a tank, a marked bot, escort, sphere or battle res duty, or while the raid gathers.
bool GetAnubarakBurrowerLead(PlayerbotAI* botAI, AnubarakBurrowerLead& out);
// This DPS bot's target by priority: skull, nearby scarabs and burrowers, him in the push, its side's focus,
// leftover scarabs, the other side's focus, him. Skips a capped pick. Null off the pull, non-DPS or sphere duty.
Unit* GetAnubarakDpsTarget(PlayerbotAI* botAI);
// A living, selectable burrower within range of at. waiting: only one no tank has held for the settle
// yet, above the off-Permafrost kill line.
bool AnubarakBurrowerNear(Player* bot, Position const& at, float range, bool waiting);
// A living, selectable burrower no tank has held for the settle yet that can come within radius of at in
// seconds at its run speed
bool AnubarakLooseBurrowerReaches(Player* bot, Position const& at, float radius, float seconds);
// A floor point on the melee ring round the burrower, clear of every patch, held while it stays put: in his
// melee reach first, behind him before in front, else nearest him. False with none, or no patch near enough.
bool GetAnubarakMeleeBurrowerStand(Player* bot, Creature* burrower, Position& out);
// A melee DPS bot's current target is a burrower near a patch with a stand off it, so its own walk
// replaces reach melee, set behind and flank: behind a held burrower is its patch
bool AnubarakMeleeBurrowerStandApplies(PlayerbotAI* botAI);
// That, and the bot is out of melee reach or inside the ring's clearance of a patch
bool AnubarakMeleeNeedsBurrowerStand(PlayerbotAI* botAI);
// A non-tank, unmarked, off grip duty (killing its gripped sphere aside), within Permafrost's slow reach
bool AnubarakNonTankOnPermafrost(Player* bot);
// For such a bot: the nearest floor point ANUBARAK_TANK_KEEPOUT clear of every patch, within reach of its
// current target and off the spike when one is. False otherwise.
bool GetAnubarakPermafrostStepOut(Player* bot, Position& out);
// The first leg toward to: a corner round a patch whose slow reach the straight walk would cut, else to
Position GetAnubarakPatchSafeLeg(Player* bot, Position const& to);
// Nearest scarab within 30 yd hitting someone who isn't a tank. Heroic spreads Acid-Drenched Mandibles:
// in phase 2 the main tank takes only one already on it, and a side tank only one on a healer.
Creature* GetAnubarakScarabToPickUp(Player* bot);
// A scarab within 15 yd is on this non-tank bot: out is a clear point 10 yd further off, toward the gather or tank.
// False when marked, on escort, sphere or grip duty or a dodge hold, or against Determination, which outruns all.
bool GetAnubarakScarabKite(PlayerbotAI* botAI, Position& out);
// What a hunter or rogue bot's Misdirection or Tricks of the Trade does now
struct AnubarakRedirect
{
    Player* tank = nullptr;
    // Where a hunter spends the redirected shots
    Unit* dump = nullptr;
};
// The pull's first 10 s, the first hunter and rogue bot to the main tank. Then to a bot side tank whose burrower
// isn't on it: one hunter per burrower by guid, a rogue only for the one it hits within 20 yd. False otherwise.
bool GetAnubarakRedirect(Player* bot, AnubarakRedirect& out);
inline constexpr float ANUBARAK_PENETRATING_COLD_HEAL_PCT = 90.0f;
// Lowest health Penetrating Cold carrier this healer can reach, under 90% or with no HoT or shield this
// healer can give it; none in heroic phase 3
Player* GetAnubarakPenetratingColdHealTarget(PlayerbotAI* botAI);

// Heroic phase 3, set per healer bot as the phase starts: priests and druids keep the Penetrating Cold carriers
// alive, paladins and shamans the tanks, each side at least one healer
enum class AnubarakHealDuty : uint8
{
    None,
    Cold,
    Tank
};

// Heroic phase 3's heal. urgent: itself at risk, then its duty's carrier or tank. rescue: a carrier at risk.
// floor: itself near the swarm's minimum tick. hot: that HoT and nothing bigger. A tank or carrier another
// heal already heads for ranks behind one with none.
struct AnubarakSwarmHealPick
{
    Player* target = nullptr;
    bool urgent = false;
    bool rescue = false;
    bool floor = false;
    bool hot = false;
};
// A `hot` pick's HoT, false once none is due
bool CastAnubarakSwarmHot(PlayerbotAI* botAI, Player* member);
// A `floor` pick's heal on itself, false once none is due
bool CastAnubarakSwarmFloor(PlayerbotAI* botAI);
AnubarakSwarmHealPick GetAnubarakSwarmHealPick(PlayerbotAI* botAI);
// A non-tank without Penetrating Cold, the healer itself too: healing it only feeds the swarm
bool AnubarakSwarmHealWasted(Player* target);
// A HoT or shield on a Penetrating Cold carrier with none from anyone, when this healer has one ready
bool CastAnubarakColdCarrierCover(PlayerbotAI* botAI, Player* member);
// For a carrier at risk: its own defensive when it's this healer, a shield, an instant, then a direct
// heal that lands before the tick
bool CastAnubarakColdRescue(PlayerbotAI* botAI, Player* member);
// Heroic phase 3, a paladin bot holding its own Frost Resistance Aura with Aura Mastery ready: a round
// landed under 1.5 s ago and 2 or more of its carriers are under the tick
bool AnubarakAuraMasteryDue(PlayerbotAI* botAI);
// Heroic before phase 3, an unmarked healer paladin bot under 80% mana with Divine Plea ready and no tank or
// Penetrating Cold carrier under 50%
bool AnubarakDivinePleaDue(PlayerbotAI* botAI);
// Heroic, an unmarked druid bot but a tank with Innervate ready: the healer in reach with the least mana but
// this bot, under 50% and under the bot's own, else null
Player* GetAnubarakInnervateTarget(PlayerbotAI* botAI);
bool AnubarakInnervateHealerDue(PlayerbotAI* botAI);
// Heroic, a druid tank always, else a druid bot while another healer has less mana: its Innervate goes to that one
bool AnubarakSelfInnervateHeld(PlayerbotAI* botAI);
// A priest's Power Word: Shield goes on a tank first: what it absorbs never heals him
bool AnubarakSwarmShieldsTank(Player* bot, Player* target);

inline constexpr float ANUBARAK_TANK_RESCUE_PCT = 35.0f;
struct AnubarakTankRescue
{
    Player* tank = nullptr;
    char const* spell = nullptr;
};
// Heroic phase 3: the lowest tank in sight under 35% with no Pain Suppression or Guardian Spirit on it, and the
// first of Lay on Hands, Pain Suppression and Guardian Spirit this bot can cast on it. False with none.
bool GetAnubarakTankRescue(PlayerbotAI* botAI, AnubarakTankRescue& out);
// Phase 3 once lust opens: the Heroism or Bloodlust this shaman bot casts on itself, whatever it has
// selected (the class trigger wants the boss selected). Null while it's Sated or Exhausted.
char const* GetAnubarakLust(PlayerbotAI* botAI);
// His health at or under 40% before the swarm, unmarked, off escort and sphere duty: "life tap" for a warlock
// under 95% mana and over 50% health, "evocation" for a mage under 60% while he is down. Null otherwise.
char const* GetAnubarakManaTopUp(PlayerbotAI* botAI);
bool AnubarakManaTopUpDue(PlayerbotAI* botAI);
// Phase 3: the paladin bot keeping Judgement of Light on him. Once it's missing or under 3 s, the turn goes to
// the unmarked paladin bot with the most mana (a healer's counted 20 under), held until due again or it can't.
Player* GetAnubarakLightJudge(Player* bot);
// This bot holds the turn and it's due
bool AnubarakLightJudgeDue(PlayerbotAI* botAI);
// A paladin bot in phase 3: the class Judgement of Light waits, the holder judges through the encounter
bool AnubarakSwarmLightHeld(PlayerbotAI* botAI);
// A paladin bot whose own Judgement of Light is on him in phase 3: any judgement it casts would replace it
bool AnubarakSwarmJudgementHeld(PlayerbotAI* botAI);
// Every difficulty, phase 3: a shaman bot with neither its Healing Stream nor its Mana Tide Totem out
bool AnubarakHealingStreamDue(PlayerbotAI* botAI);

// The flying sphere this bot shoots now, null when it has no sphere duty
Creature* GetAnubarakSphereToShoot(Player* bot);
// The DK's grip of a sphere for its side's tank patch. False without one.
bool GetAnubarakGripDuty(Player* bot, AnubarakGripDuty& out);
// Called once the DK's Death Grip on its duty sphere went out, with the side its duty had before the cast
void TrackAnubarakGripCast(Player* bot, Creature* sphere, uint8 side);
// The shot or gripped sphere this bot's duty is on, else null
Creature* GetAnubarakDutySphere(Player* bot);
// 3D within the bot's spell range less ANUBARAK_SHOOT_MARGIN, in line of sight
bool AnubarakSphereInReach(Player* bot, Unit* sphere);
// Floor point toward the sphere where it is in reach, off the spike and its lane. False when none.
bool GetAnubarakSphereStand(Player* bot, Unit* sphere, Position& out);
// Where the spike's target runs: its kite plan's track 1.2 s past the point nearest it. False while there's
// no spike.
bool GetAnubarakKiteStand(Player* bot, AnubarakKiteStand& out);
// Heroic only, None elsewhere and for an unmarked bot. boost is set to the speed boost to use now, if any,
// whatever the stage.
AnubarakKiteStage GetAnubarakKiteStage(Player* kiter, AnubarakKiteBoost* boost = nullptr);
// Heroic phase 2: this bot's spot in the raid's gather, not floor-checked, false when it doesn't gather
// (the kiter, a side tank a burrower is on, the escort). The main tank's is on the gather's edge.
bool GetAnubarakGatherSpot(Player* bot, Position& out);
bool AnubarakGathers(PlayerbotAI* botAI);
// While he is up, a healer bot's slot on a gather ring clear of every patch, not floor-checked, split per side
// with the tank patches 40+ yd apart. Heroic from his 35% in a phase 1, a stand behind the tank patches instead.
bool GetAnubarakHealerStand(Player* bot, Position& out);
bool AnubarakHealerStandApplies(PlayerbotAI* botAI);
// Heroic, from his 35% in a phase 1 and in phase 3: a tank under 50%, or the healer itself near the swarm's
// minimum tick, so it heals where it stands instead of walking to its stand
bool AnubarakHealerStandWaits(Player* bot);
// Heroic phase 3: a ranged DPS bot's stand behind the tank patches, after the healers', within 28 yd of him and
// heal reach of every healer stand. False with no stand left, and on sphere or battle res duty.
bool GetAnubarakRangedStand(Player* bot, Position& out);
bool AnubarakRangedStandApplies(PlayerbotAI* botAI);
// A druid's Dash running, or known and off cooldown
bool AnubarakDashRunningOrReady(Player* bot);
// Heroic: a marked druid in Cat Form while that holds, there for Dash
bool AnubarakKiteCatHeld(PlayerbotAI* botAI);
// While he is engaged: a druid bot in Cat Form that isn't a cat DPS, not held there. Tree of Life and the
// heals won't cast from Cat Form, and caster form waits on its mana.
bool AnubarakKiteCatDropDue(PlayerbotAI* botAI);
// The druid main tank bot, while he is down with 10 s or less to his emerge. Its own Rejuvenation would
// take it out of bear form for his first hits.
bool AnubarakMainTankKeepsBear(PlayerbotAI* botAI);
// A heroic warlock bot's Demonic Circle. Before the pull, once held in the pit, parked or not: missing, off
// the room centre or under 290 s left. While he is up: missing or under 100 s, cast where it stands.
bool AnubarakDemonicCircleDue(PlayerbotAI* botAI);
// Where the due circle goes down: the room centre before the pull, false where the warlock stands
bool GetAnubarakCircleSpot(PlayerbotAI* botAI, Position& spot);

// The spike within its danger radius of `at`, or its path over ANUBARAK_DODGE_TRACK_S within its lane;
// false with no spike
bool AnubarakSpikeDangerAt(Player* bot, Position const& at);
// The same for the bot where it stands, unmarked
bool AnubarakInSpikeDanger(Player* bot);
// The nearest spot clear of the spike's path by its clearance, the bot's gather slot side first. Heroic
// keeps out of Permafrost's slow reach when any spot allows it.
bool GetAnubarakSpikeDodgeSpot(Player* bot, Position& out);
// The bot's dodge spot while the hold is valid: still walking or parked, the mark unchanged, the spot safe and
// the spike still near its slot or the walk back. An invalid hold reads false and stays stored.
bool GetAnubarakDodgeHold(Player* bot, Position& spot);
void SetAnubarakDodgeHold(Player* bot, Position const& spot);
bool IsAnubarakDodgeHeld(PlayerbotAI* botAI);
// A valid hold the bot hasn't reached yet and is over ANUBARAK_DODGE_ARRIVE off
bool AnubarakDodgeWalking(Player* bot);
// The walk from where the bot stands to `to`, and `to` itself, keep clear of the spike's path: true
// with no spike
bool AnubarakWalkClearOfSpike(Player* bot, Position const& to);

// The kiter this paladin bot covers with Hand of Protection now, null when it has no such duty
Player* GetAnubarakKiterToProtect(Player* bot);
// Heroic: the planned Hand paladin shadows the kiter, then posts by the catch from 4 s before, or by the
// patch for a boosting kiter. No other paladin's Hand planned: a standby shadows.
bool GetAnubarakEscortSpot(Player* bot, Position& out);
bool IsAnubarakEscort(PlayerbotAI* botAI);
// Heroic: this paladin bot is the one planned or claimed for the kiter's Hand, within 2.5 s of the catch
bool AnubarakHandReserved(PlayerbotAI* botAI);
// The kiter, marked while it carries Permafrost, this paladin bot casts Hand of Freedom on now
Player* GetAnubarakKiterToFree(Player* bot);
// The marked bot must cast an immunity now. Heroic: as its plan times it, a non-paladin's 0.8 s before a planned
// Hand ends, or once the plan failed. Normal: the spike in immunity range, no protector, no reachable patch stand.
bool AnubarakKiterNeedsImmunity(Player* bot);
// The marked paladin's own Hand keeps it on the spike to a patch or the emerge, or the plan ends on it.
// Otherwise, and always for a plan ending on its own immunity, Divine Shield hands the spike on.
bool AnubarakKiterKeepsMark(Player* bot);
bool AnubarakKiterImmune(Unit* kiter);

// The burrower whose Shadow Strike this bot interrupts, null when it has none
Creature* GetAnubarakShadowStrikeDuty(Player* bot);
// Name of the first interrupt or stun the bot can cast on target now, null when none.
// Burrowers are silence-immune, so no silence-only spells here.
char const* AnubarakReadyInterrupt(Player* bot, Unit* target);

}

#endif
