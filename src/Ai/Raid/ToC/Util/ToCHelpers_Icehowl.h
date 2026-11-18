/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCHELPERS_ICEHOWL_H
#define PLAYERBOTS_RAID_TOCHELPERS_ICEHOWL_H

#include "Define.h"
#include "Position.h"
#include "ToCData.h"

class PlayerbotAI;
class Unit;

namespace TrialOfTheCrusaderHelpers
{

// boss_northrend_beasts.cpp IcehowlSpells
inline constexpr uint32 SPELL_STAGGERED_DAZE = 66758;  // no difficulty row
inline constexpr uint32 SPELL_FROTHING_RAGE = 66759;
inline constexpr uint32 SPELL_TRAMPLE = 66734;         // no difficulty row
inline constexpr uint32 SPELL_ARCTIC_BREATH = 66689;

// Charge geometry from EVENT_JUMP_BACK: he jumps 35 yd back from ARENA_CENTER, then charges through
// it to 50 yd out, 46 when the line's angle falls in (1, 2) rad (the main gate).
inline constexpr float ICEHOWL_CHARGE_BACK = 35.0f;
inline constexpr float ICEHOWL_CHARGE_REACH = 50.0f;
inline constexpr float ICEHOWL_CHARGE_REACH_GATE = 46.0f;
// DoTrampleIfValid: any living player within 12 yd of him, 3D centre to centre
inline constexpr float ICEHOWL_TRAMPLE_RADIUS = 12.0f;
inline constexpr float ICEHOWL_CHARGE_TRIGGER = 14.0f;
inline constexpr float ICEHOWL_CHARGE_CLEARANCE = 16.0f;
inline constexpr float ICEHOWL_CHARGE_TIGHT_CLEARANCE = 13.0f;
// He lands on ARENA_CENTER exactly, this only absorbs float noise
inline constexpr float ICEHOWL_CENTRE_TOLERANCE = 3.0f;

// Arctic Breath's bisector runs through its target, so a neighbour is hit only within half the cone
// of that bearing, at any range. Bots spread by bearing, one step being half the cone plus the margin.
inline constexpr float ICEHOWL_BREATH_DEFAULT_CONE_DEG = 24.0f;  // TARGET_UNIT_CONE_ENEMY_24, no spell_cone row
inline constexpr float ICEHOWL_SPREAD_MARGIN_DEG = 6.0f;
// A bot this far off its bearing walks back, so two neighbours off toward each other still sit
// outside a breath on either
inline constexpr float ICEHOWL_SPREAD_TRIGGER_DEG = 3.0f;
inline constexpr float ICEHOWL_SPREAD_ARRIVE_DEG = 1.5f;
inline constexpr float ICEHOWL_SPREAD_HALF_ARC_DEG = 90.0f;      // bearings kept either side of his back
// A healer here at 17 yd is ~27 yd from his tank, ~30 straight behind
inline constexpr float ICEHOWL_SPREAD_HEALER_BEARING_DEG = 60.0f;
// Display 21601, scale 1
inline constexpr float ICEHOWL_COMBAT_REACH = 14.0f;
// Whirl, and a hunter's 5 yd minimum range past his melee range, all centre to centre. Whirl is a
// creature's area spell, so it adds no reach for players.
inline constexpr float ICEHOWL_MELEE_RANGE = PlayerMeleeRange(ICEHOWL_COMBAT_REACH);
inline constexpr float ICEHOWL_WHIRL_RADIUS = 15.0f;
inline constexpr float ICEHOWL_RANGED_MIN_RANGE = ICEHOWL_MELEE_RANGE + 5.0f;
// Radius bands from his centre. Melee sit between Whirl and the edge of his reach, so their band and
// trigger are tight.
inline constexpr float ICEHOWL_SPREAD_MELEE_MIN = 15.5f;
inline constexpr float ICEHOWL_SPREAD_MELEE_MAX = 16.2f;
inline constexpr float ICEHOWL_SPREAD_HEALER_MIN = 16.5f;
inline constexpr float ICEHOWL_SPREAD_HEALER_MAX = 18.5f;
inline constexpr float ICEHOWL_SPREAD_RANGED_MIN = 23.0f;
inline constexpr float ICEHOWL_SPREAD_RANGED_MAX = 26.5f;
// How far outside its band a bot may stand before it walks back
inline constexpr float ICEHOWL_SPREAD_TRIGGER = 1.0f;
inline constexpr float ICEHOWL_SPREAD_ARRIVE = 0.5f;
inline constexpr float ICEHOWL_SPREAD_MELEE_TRIGGER = 0.3f;
inline constexpr float ICEHOWL_SPREAD_MELEE_ARRIVE = 0.15f;
// A bearing walled closer than this is dropped, or kept for melee alone inside the melee arc with
// room for their band
inline constexpr float ICEHOWL_SPREAD_MIN_ROOM = 24.0f;
inline constexpr float ICEHOWL_SPREAD_MELEE_ROOM = 17.5f;
// Whirl throws his tank ~31 yd and he follows, so the layout re-latches when he moves or turns this far
inline constexpr float ICEHOWL_SPREAD_RELATCH_MOVE = 8.0f;
inline constexpr float ICEHOWL_SPREAD_RELATCH_TURN_DEG = 30.0f;
// While he stands, his victim out of his reach for less than this keeps the layout as it stands
inline constexpr uint32 ICEHOWL_SPREAD_VICTIM_GRACE_MS = 1000;
// Chasing a Whirl-thrown tank he keeps it mostly in his reach, so a reach test misses his walk. While
// he moves and this long after, the layout holds as latched with no re-latch or stand walk.
inline constexpr uint32 ICEHOWL_SPREAD_SETTLE_MS = 500;
// Past the ranged band by his drift before a re-latch, so an open bearing never reads as walled
inline constexpr float ICEHOWL_SPREAD_PROBE = ICEHOWL_SPREAD_RANGED_MAX + ICEHOWL_SPREAD_RELATCH_MOVE + 1.0f;
// Melee bearings stay this far round from his back. Past 90° off it he parries them and hastes his
// next swing, and a stand error or his turn before a re-latch must not take them there.
inline constexpr float ICEHOWL_SPREAD_MELEE_ARC_DEG =
    90.0f - ICEHOWL_SPREAD_RELATCH_TURN_DEG - ICEHOWL_SPREAD_TRIGGER_DEG;

static_assert(2.0f * ICEHOWL_SPREAD_TRIGGER_DEG <= ICEHOWL_SPREAD_MARGIN_DEG,
              "neighbours stay out of each other's breath");
static_assert(ICEHOWL_SPREAD_ARRIVE_DEG < ICEHOWL_SPREAD_TRIGGER_DEG && ICEHOWL_SPREAD_ARRIVE < ICEHOWL_SPREAD_TRIGGER &&
                  ICEHOWL_SPREAD_MELEE_ARRIVE < ICEHOWL_SPREAD_MELEE_TRIGGER,
              "a walk has to end inside its own trigger");
static_assert(ICEHOWL_SPREAD_MELEE_MAX + ICEHOWL_SPREAD_MELEE_TRIGGER < ICEHOWL_MELEE_RANGE, "melee stay in reach");
static_assert(ICEHOWL_SPREAD_MELEE_MIN - ICEHOWL_SPREAD_MELEE_TRIGGER > ICEHOWL_WHIRL_RADIUS, "melee stay out of Whirl");
static_assert(ICEHOWL_SPREAD_HEALER_MIN - ICEHOWL_SPREAD_TRIGGER > ICEHOWL_WHIRL_RADIUS, "healers stay out of Whirl");
static_assert(ICEHOWL_SPREAD_RANGED_MIN - ICEHOWL_SPREAD_TRIGGER > ICEHOWL_RANGED_MIN_RANGE,
              "hunters stay past their minimum range");
static_assert(ICEHOWL_SPREAD_MIN_ROOM - 1.0f >= ICEHOWL_SPREAD_RANGED_MIN, "a kept bearing fits the ranged band");
static_assert(ICEHOWL_SPREAD_MELEE_ROOM >= ICEHOWL_SPREAD_MELEE_MAX + 1.0f, "a melee-only bearing fits the melee band");

// A bot's Arctic Breath stand in Icehowl's frame: a breath aims by bearing, so a bot holds its stand
// while it stays near the bearing from him and inside the radius band.
struct IcehowlBreathStand
{
    Position spot;
    Position icehowl;
    float bearing = 0.0f;  // radians, from him
    float bandMin = 0.0f;
    float bandMax = 0.0f;  // capped by the room along the bearing
    // The role's radial tolerances outside the band: walk back past trigger, stop within arrive
    float trigger = 0.0f;
    float arrive = 0.0f;
};

// Degrees between two neighbouring breath bearings on this map's difficulty: half the remapped
// Arctic Breath cone plus the margin, so 36 on 10N and 18 elsewhere.
float IcehowlBreathStepDeg(Unit* anyUnitOnMap);

// The charge line, from where the jump back lands to where the charge stops. Latched from the gaze
// to the end of the charge, read once per instance per ms. False off map 649 and outside the charge.
bool IcehowlChargeLatched(PlayerbotAI* botAI, Position& start, Position& end);

// 2D distance from (x, y) to the latched line, FLT_MAX when nothing is latched.
float DistanceToIcehowlCharge(PlayerbotAI* botAI, float x, float y);

// Same, for a caller already holding the line.
float DistanceToIcehowlLane(Position const& start, Position const& end, float x, float y);

// The wall crash: 15 s stunned, +100% damage taken
bool IsIcehowlStaggered(Unit* icehowl);

// Nitro Boosts for a walk into the daze this long or more, with this much of it left
inline constexpr float ICEHOWL_DAZE_NITRO_FROM = 10.0f;
inline constexpr int32 ICEHOWL_DAZE_NITRO_MIN_LEFT_MS = 3000;

// A melee bot or tank free to move, Nitro Boosts ready, ICEHOWL_DAZE_NITRO_FROM or more short of a
// dazed Icehowl's melee range
bool IcehowlDazeNitroDue(PlayerbotAI* botAI);

bool HasIcehowlFrothingRage(Unit* icehowl);

// The bot's Arctic Breath stand along its bearing, its distance clamped into the role's band. False while
// the spread is off or held through his walk, and with no bearing (tanks, humans, undealt, walled-in melee).
bool GetIcehowlBreathStand(PlayerbotAI* botAI, IcehowlBreathStand& stand);

// (x, y) within `degrees` of the stand's bearing and `yards` of its band
bool IsOnIcehowlBreathStand(IcehowlBreathStand const& stand, float x, float y, float degrees, float yards);

// Also true while the layout holds through his walk, when GetIcehowlBreathStand gives no stand
bool HasIcehowlBreathSlot(PlayerbotAI* botAI);

}

#endif
