/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40HELPERS_H
#define PLAYERBOTS_AQ40HELPERS_H

#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"

enum ReactStates : uint8;

class Creature;
class Player;
class PlayerbotAI;
class SpellInfo;
class Unit;
class WorldObject;

namespace Aq40Helpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Values match the instance script's boss data ids.
enum class Aq40Encounter : uint32
{
    None,
    Skeram,
    Trio,
    Sartura,
    Fankriss,
    Viscidus,
    Huhuran,
    Twins,
    Ouro,
    Cthun
};

enum class Aq40Npcs : uint32
{
    // The Prophet Skeram
    NPC_SKERAM = 15263,

    // Bug Trio
    NPC_KRI = 15511,
    NPC_YAUJ = 15543,
    NPC_VEM = 15544,
    NPC_YAUJ_BROOD = 15621,
    NPC_POISON_CLOUD = 15933,

    // Battleguard Sartura
    NPC_SARTURA = 15516,
    NPC_SARTURA_ROYAL_GUARD = 15984,

    // Fankriss the Unyielding
    NPC_FANKRISS = 15510,
    NPC_SPAWN_OF_FANKRISS = 15630,
    NPC_VEKNISS_HATCHLING = 15962,

    // Viscidus
    NPC_VISCIDUS = 15299,
    NPC_GLOB_OF_VISCIDUS = 15667,
    NPC_TOXIC_SLIME = 15925,

    // Princess Huhuran
    NPC_HUHURAN = 15509,

    // Twin Emperors
    NPC_VEKNILASH = 15275,
    NPC_VEKLOR = 15276,
    NPC_QIRAJI_SCARAB = 15316,
    NPC_QIRAJI_SCORPION = 15317,
    NPC_ANUBISATH_DEFENDER = 15277,

    // Ouro
    NPC_OURO = 15517,
    NPC_DIRT_MOUND = 15712,
    NPC_OURO_SCARAB = 15718,

    // C'Thun
    NPC_EYE_OF_CTHUN = 15589,
    NPC_CTHUN = 15727,
    NPC_EYE_TENTACLE = 15726,
    NPC_CLAW_TENTACLE = 15725,
    NPC_GIANT_EYE_TENTACLE = 15334,
    NPC_GIANT_CLAW_TENTACLE = 15728,
    NPC_FLESH_TENTACLE = 15802,
    NPC_EXIT_TRIGGER = 15800,

    // Warlock
    NPC_VOIDWALKER = 1860
};

enum class Aq40Spells : uint32
{
    // The Prophet Skeram
    SPELL_TRUE_FULFILLMENT = 785,
    SPELL_SKERAM_BLINK_1 = 4801,
    SPELL_SKERAM_BLINK_2 = 8195,
    SPELL_SKERAM_BLINK_3 = 20449,

    // Battleguard Sartura
    SPELL_WHIRLWIND = 26083,
    SPELL_GUARD_WHIRLWIND = 26038,

    // Fankriss the Unyielding
    SPELL_MORTAL_WOUND = 25646,

    // Viscidus
    SPELL_VISCIDUS_FREEZE = 25937,
    SPELL_INVIS_SELF = 25905,
    SPELL_FROSTBOLT_RANK_1 = 116,
    SPELL_CLEANSING_TOTEM = 8170,

    // Princess Huhuran
    SPELL_FRENZY = 26051,
    SPELL_WYVERN_STING = 26180,
    SPELL_ACID_SPIT = 26050,
    SPELL_POISON_BOLT = 26052,

    // Twin Emperors
    SPELL_MUTATE_BUG = 802,
    SPELL_EXPLODE_BUG = 804,
    SPELL_TWIN_TELEPORT = 799,
    SPELL_TWIN_TELEPORT_VISUAL = 26638,
    SPELL_BLIZZARD = 26607,

    // Anubisath Defender
    SPELL_SHADOW_FROST_REFLECT = 19595,
    SPELL_FIRE_ARCANE_REFLECT = 13022,

    // C'Thun
    SPELL_EYE_BEAM = 26134,
    SPELL_RED_COLORATION = 22518,  // shown while the Eye casts Dark Glare
    SPELL_DIGESTIVE_ACID = 26476,
    SPELL_CARAPACE_CTHUN = 26156,
    SPELL_PURPLE_COLORATION = 22581,  // shown while C'Thun is Weakened

    // Druid
    SPELL_TREE_OF_LIFE = 33891
};

inline constexpr uint32 AQ40_MAP_ID = 531;

// General

// The tracker rescans encounter units within this distance of the boss.
inline constexpr float ENCOUNTER_SCAN_DISTANCE = 220.0f;
inline constexpr uint32 TRACKER_INTERVAL_MS = 250;
inline constexpr uint32 BOT_CACHE_INTERVAL_MS = 100;
// Positioning that shares the bot with its rotation rechecks at this pace, so an unreachable spot can't hold the bot
// every tick.
inline constexpr uint32 POSITION_INTERVAL_MS = 500;
inline constexpr float PATH_STEP_DISTANCE = 3.5f;

// The Prophet Skeram

inline constexpr uint8 SKERAM_CENTER_PLATFORM = 1;
inline constexpr float SKERAM_IMAGE_TELEPORT_DISTANCE = 20.0f;
inline constexpr float SKERAM_PLATFORM_HEIGHT_TOLERANCE = 3.0f;
inline constexpr float SKERAM_TANK_PLATFORM_RADIUS = 12.0f;
inline constexpr float SKERAM_LOCAL_REACH = 12.0f;
// In-game healer spots for the two raised platforms, off the boss's blink destinations.
inline Position const SKERAM_LOW_Y_HEALER_SPOT = {-8335.298f, 2058.8733f, 133.08563f};   // blink 4801
inline Position const SKERAM_HIGH_Y_HEALER_SPOT = {-8352.727f, 2108.1216f, 133.09175f};  // blink 20449

// Bug Trio

inline constexpr uint8 SKULL_ICON = 7;
inline constexpr float TRIO_BOSS_SEPARATION = 18.0f;
inline constexpr float POISON_CLOUD_RADIUS = 12.0f;

// Battleguard Sartura

inline constexpr float WHIRLWIND_SAFE_DISTANCE = 18.0f;
inline constexpr float WHIRLWIND_ESCAPE_STEP = 5.0f;
// A guard's first whirlwind, the earliest threat wipe, comes 6 s after the pull at the soonest.
inline constexpr uint32 SARTURA_OPENING_MS = 6000;

// Viscidus: Toxin (25989) is a 5-yard persistent cloud that never despawns, so the room fills up.
// The raid is pulled from a ramp that descends from the entrance (z -24) to the floor (z -45..-53).

inline constexpr float TOXIN_RADIUS = 5.0f;
inline constexpr float TOXIN_MARGIN = 3.0f;
inline constexpr float VISCIDUS_RAMP_Z = -40.0f;
inline constexpr float VISCIDUS_RAMP_Y = 985.0f;
inline constexpr float VISCIDUS_RING_OFFSET = 18.0f;
inline constexpr float VISCIDUS_RING_TOLERANCE = 6.0f;
// Angle step (rad) when sliding a ring slot off a Toxin cloud; ~4 yd at ring radius.
inline constexpr float VISCIDUS_SPOT_SEARCH_STEP = 0.15f;
// Cloud handling follows Lady Vashj's Toxic Spores (SSCHelpers.h). A bot this close to a cloud counts as in it.
inline constexpr float TOXIN_HAZARD_RADIUS = TOXIN_RADIUS + TOXIN_MARGIN;
// Melee on the boss this close to a cloud move only with the melee toxin action, so reach-melee cannot walk them
// back through a cloud to get to him.
inline constexpr float TOXIN_MELEE_CONTROL_RADIUS = TOXIN_HAZARD_RADIUS + 2.5f;
// Melee spots sit this far inside melee range.
inline constexpr float TOXIN_MELEE_RANGE_INSET = 1.0f;
// Tank breakout search: rings of spots around the tank; a yard walked through a cloud costs this many yards, and a
// spot is held this long before it is picked again.
inline constexpr uint8 TOXIN_TANK_SEARCH_DIRECTIONS = 24;
inline constexpr uint8 TOXIN_TANK_SEARCH_RINGS = 7;
inline constexpr float TOXIN_TANK_SEARCH_RING_SPACING = 5.0f;
inline constexpr float TOXIN_TANK_CLOUD_YARD_COST = 3.0f;
inline constexpr uint8 TOXIN_TANK_PATH_CHECKS = 6;
inline constexpr uint32 TOXIN_TANK_COMMIT_MS = 12 * IN_MILLISECONDS;
// Frost Oil (item 3829): a weapon enchant that procs Frostbolt (spell 205) on hit.
inline constexpr uint32 ENCHANT_FROST_OIL = 26;
inline constexpr uint32 FROST_OIL_DURATION_MS = 30 * MINUTE * IN_MILLISECONDS;
inline Position const VISCIDUS_ROOM_CENTER = {-7992.36f, 908.19f, -52.62f};

// Princess Huhuran

inline constexpr uint32 FANKRISS_SWAP_STACKS = 3;
inline constexpr uint32 HUHURAN_SWAP_STACKS = 5;
// Dispel damage is exactly 3,000 in this server's aura script.
inline constexpr uint32 WYVERN_STING_SAFE_HEALTH = 4500;
inline constexpr float HUHURAN_COOLDOWN_HEALTH_PCT = 30.0f;
// A spread slot whose floor sits more than this above or below Huhuran is on the cave wall.
inline constexpr float HUHURAN_SPREAD_MAX_RISE = 3.0f;

// Twin Emperors

// Edge distance Vek'lor's tank keeps: Arcane Burst triggers within 5 yd; Demoralizing Shout (10 yd) must reach him.
inline constexpr float TWIN_BURST_CLEARANCE = 6.5f;
inline constexpr uint8 TWIN_LOCK_ICON = 4;         // Moon: the warlock who tanks Vek'lor on his own (north) side.
inline constexpr uint8 TWIN_SECOND_LOCK_ICON = 5;  // Square: the warlock for Vek'nilash's (south) side.
// Tanking spots on the room floor, off the pedestals and their stairs. They are ~118 yd apart because the emperors
// heal each other within 60 yd and drift toward each other after swaps.
inline std::array<Position, 2> const TWIN_FLOOR_SPOTS = {Position{-9016.0f, 1262.0f, -112.3f},
                                                         Position{-8900.0f, 1284.0f, -112.3f}};
// Each side's warlock waits ~28 yards behind that side's spot, toward the entrance, with the healers.
inline std::array<Position, 2> const TWIN_LOCK_SPOTS = {Position{-9008.0f, 1288.0f, -112.3f},
                                                        Position{-8920.0f, 1304.0f, -112.3f}};
// 31 yd (32 yd in 3D) from Vek'lor on his pedestal: outside his ~29-yd aggro reach (detection 20 + 3 for the level
// gap + both combat reaches), still inside Searing Pain range edge-to-edge.
inline Position const TWIN_MOON_PREPULL_SPOT = {-8889.6f, 1228.5f, -112.3f};
inline Position const TWIN_ROOM_CENTER = {-8962.0f, 1235.0f, -112.3f};
inline constexpr float TWIN_ROOM_RADIUS = 125.0f;
inline constexpr float TWIN_ROOM_MIN_Z = -118.0f;
inline constexpr float TWIN_ROOM_MAX_Z = -90.0f;
// Teleports come 29-40 s apart (first one at 30 s). Melee leave the landing spots before the window.
inline constexpr uint32 TWIN_SWAP_WINDOW_MS = 27 * IN_MILLISECONDS;
// Healing threat splits onto both emperors, and Vek'nilash cannot be taunted back. For the first seconds of the
// pull healers only save players in real danger while the tanks build threat.
inline constexpr uint32 TWIN_OPENING_HOLD_MS = 8 * IN_MILLISECONDS;
inline constexpr float TWIN_OPENING_HOLD_HEALTH_PCT = 60.0f;
// Center distance the Vek'lor warlock keeps: inside Searing Pain range, outside the bystander ring.
inline constexpr float TWIN_LOCK_DISTANCE = 26.0f;
inline constexpr float TWIN_LOCK_MAX_DISTANCE = 35.0f;
// Center distance everyone but Vek'lor's tank keeps from him, so that tank (at about 14 yd) is clearly the nearest
// player when Vek'nilash lands in his place.
inline constexpr float TWIN_BYSTANDER_DISTANCE = 24.0f;
// One large crit of headroom below the 130% ranged pull threshold on Vek'lor's tank.
inline constexpr float TWIN_CASTER_THREAT_MARGIN = 1500.0f;
inline constexpr float TWIN_TELEPORT_JUMP = 30.0f;
inline constexpr float TWIN_TANK_OFFSET = 4.0f;
inline constexpr float BLIZZARD_SEARCH_RADIUS = 30.0f;
inline constexpr float BLIZZARD_HAZARD_RADIUS = 12.0f;
inline constexpr float BLIZZARD_ESCAPE_DISTANCE = 15.0f;
inline constexpr float EXPLODING_BUG_RADIUS = 15.0f;
inline constexpr float TWIN_BUG_REACH = 30.0f;

// Ouro

inline constexpr float DIRT_MOUND_RADIUS = 12.0f;
inline constexpr float OURO_CONE_RANGE = 40.0f;

// C'Thun: the Eye and C'Thun share this center. Spots are offsets from it (CTHUN_SLOTS).

inline Position const CTHUN_CENTER = {-8578.8f, 1986.2f, 100.7f};
// Raid spots, offsets (x = north, y = west) from the center. Eye Beam jumps to the closest player within ~13 yd
// (10 yd plus both combat reaches) at x1.5 damage, so the 40 spots fit the walkable floor (z 100.7) with no chain
// longer than two at 13.5 yd. Four melee stand on the Eye at N/W/S/E (inside its 17.8-yd melee range); the middle
// ring (30-42 yd) keeps ranged in reach (30-yd spells reach 46.5 yd from its center); healers take the outer ring.
enum class CthunSlotKind : uint8
{
    EyeMelee,
    Middle,
    Outer
};

struct CthunSlotSpot
{
    float x;
    float y;
    CthunSlotKind kind;
};

inline constexpr CthunSlotSpot CTHUN_SLOTS[] = {
    {16.5f, 0.0f, CthunSlotKind::EyeMelee},  {0.0f, 16.5f, CthunSlotKind::EyeMelee},
    {-16.5f, 0.0f, CthunSlotKind::EyeMelee}, {0.0f, -16.5f, CthunSlotKind::EyeMelee},
    {-42.0f, -0.4f, CthunSlotKind::Middle},  {-28.1f, -13.1f, CthunSlotKind::Middle},
    {-28.9f, -30.5f, CthunSlotKind::Middle}, {-14.3f, -26.4f, CthunSlotKind::Middle},
    {-12.5f, -40.1f, CthunSlotKind::Middle}, {2.9f, -34.4f, CthunSlotKind::Middle},
    {16.7f, -38.6f, CthunSlotKind::Middle},  {20.6f, -21.8f, CthunSlotKind::Middle},
    {34.5f, -24.0f, CthunSlotKind::Middle},  {31.8f, -7.6f, CthunSlotKind::Middle},
    {42.0f, 0.1f, CthunSlotKind::Middle},    {26.7f, 13.7f, CthunSlotKind::Middle},
    {33.0f, 26.0f, CthunSlotKind::Middle},   {15.1f, 25.9f, CthunSlotKind::Middle},
    {12.5f, 40.1f, CthunSlotKind::Middle},   {-1.5f, 42.0f, CthunSlotKind::Middle},
    {-14.0f, 39.6f, CthunSlotKind::Middle},  {-20.7f, 21.8f, CthunSlotKind::Middle},
    {-30.7f, 28.7f, CthunSlotKind::Middle},  {-31.2f, 8.6f, CthunSlotKind::Middle},
    {-62.9f, -11.7f, CthunSlotKind::Outer},  {-48.3f, -16.7f, CthunSlotKind::Outer},
    {-45.0f, -30.7f, CthunSlotKind::Outer},  {10.0f, -53.0f, CthunSlotKind::Outer},
    {32.1f, -47.0f, CthunSlotKind::Outer},   {50.1f, -35.0f, CthunSlotKind::Outer},
    {61.5f, -17.7f, CthunSlotKind::Outer},   {55.5f, -5.0f, CthunSlotKind::Outer},
    {43.0f, 15.5f, CthunSlotKind::Outer},    {34.5f, 50.0f, CthunSlotKind::Outer},
    {12.8f, 62.7f, CthunSlotKind::Outer},    {-11.0f, 62.8f, CthunSlotKind::Outer},
    {-32.5f, 51.0f, CthunSlotKind::Outer},   {-45.5f, 33.0f, CthunSlotKind::Outer},
    {-47.0f, 20.6f, CthunSlotKind::Outer},   {-61.0f, 9.9f, CthunSlotKind::Outer},
};
// Melee leave their spot only for a tentacle whose edge is this close to the spot.
inline constexpr float CTHUN_MELEE_LEASH = 10.0f;
// Ranged hit what their 30-yard spells reach from the spot (edge to edge, as GetDistance measures).
inline constexpr float CTHUN_RANGED_REACH = 29.0f;
// Area trigger 4036 (7 yd on the center) spits players out; clients fire it, server bots must send it.
inline constexpr uint32 CTHUN_SPIT_OUT_TRIGGER = 4036;
inline constexpr float CTHUN_SPIT_OUT_RADIUS = 6.5f;
inline constexpr uint32 CTHUN_STOMACH_EXIT_TRIGGER = 4033;
inline constexpr float CTHUN_STOMACH_Z = -40.0f;
// Leave the stomach at 6 stacks of Digestive Acid (9 with a healer inside) or below 55% health: leaving earlier
// lets the Flesh Tentacles live for minutes, leaving later kills bots on the exit pad during its 3-second wait.
inline constexpr uint32 CTHUN_STOMACH_LEAVE_STACKS = 6;
inline constexpr uint32 CTHUN_STOMACH_LEAVE_STACKS_WITH_HEALER = 9;
inline constexpr float CTHUN_STOMACH_LEAVE_HEALTH_PCT = 55.0f;
// Weakened burn: C'Thun can be hit from about 10 yd of his center, although his combat reach is 1 yd and the
// server lets bot strikes land from ~26 yd. Melee stand at 8 yd, outside the 7-yd spit-out circle.
inline constexpr float CTHUN_BURN_MELEE_REACH = 10.0f;
inline constexpr float CTHUN_BURN_MELEE_RADIUS = 8.0f;
// Phase 2: a Giant Claw farther than this from a bot's stack spot is left alone.
inline constexpr float CTHUN_FAR_CLAW = 20.0f;
// The tank marked with Moon walks in alone and pulls from the room's west edge, 76 yd from the Eye and away from
// the raid's path down the entrance slope, so the opening beams can't chain into the arriving raid.
inline constexpr uint8 CTHUN_PULLER_ICON = 4;  // Moon (the Twins warlock mark; that use ends once the Twins are dead)
inline Position const CTHUN_PULL_SPOT = {-8554.8f, 2058.2f, 100.7f};
// The opening beams land at 3, 6 and 9 s and random beams start at about 17 s.
inline constexpr uint32 CTHUN_PULLER_HOLD_MS = 20 * IN_MILLISECONDS;
inline constexpr uint32 CTHUN_PULLER_BEAM_MS = 12 * IN_MILLISECONDS;
// Before the pull the raid trails the puller this far behind (the Eye Beam jumps about 13 yd).
inline constexpr float CTHUN_TRAIL_DISTANCE = 22.0f;
// A trailing bot stays at least this much farther from the Eye than the puller.
inline constexpr float CTHUN_TRAIL_MARGIN = 10.0f;
// The pre-pull walk works only this close to the Eye and above the floor below the room.
inline constexpr float CTHUN_PREPULL_REACH = 140.0f;
inline constexpr float CTHUN_PREPULL_MIN_Z = 90.0f;
// Phase 2 stack: Giant Claws and Giant Eyes spawn under a random player outside the stomach, so they spawn on the
// raid. Inside the Eye Tentacle ring (27.5 yd) to miss their spawn knockback, outside the 7-yd spit-out, in range
// of C'Thun.
inline Position const CTHUN_STACK = {-8592.8f, 1986.2f, 100.4f};
inline constexpr float CTHUN_STACK_RADIUS = 5.0f;
inline constexpr float CTHUN_GIANT_EYE_STACK_DISTANCE = 20.0f;
// Phase 2 ranged chase Eye Tentacles anywhere in the room: their Mind Flay has no range limit.
inline constexpr float CTHUN_TENTACLE_CHASE = 70.0f;
// Phase 2 melee leave the stack for tentacles up to this far from their stack spot.
inline constexpr float CTHUN_STACK_MELEE_LEASH = 30.0f;
// Phase 2 Eye Tentacles farther than this from the stack are out of reach for casters standing in it, and live
// long when every ranged bot picks the nearest tentacle; two ranged bots take only those.
inline constexpr float CTHUN_FAR_TENTACLE = 30.0f;
inline constexpr uint32 CTHUN_TENTACLE_KILLERS = 2;
// Dark Glare band (5 yd) plus a margin, and how many 1-second ticks ahead a bot starts moving.
inline constexpr float GLARE_HALF_WIDTH = 8.0f;
inline constexpr int GLARE_LOOKAHEAD = 3;
// The glare line turns pi/35 per 1-second tick (spell_cthun_dark_glare).
inline constexpr float GLARE_STEP = static_cast<float>(M_PI) / 35.0f;
// A glare dodge stays at least this far from the Eye, steps in this much at a time where the floor runs out, and
// needs floor this far past its goal.
inline constexpr float GLARE_MIN_RADIUS = 15.0f;
inline constexpr float GLARE_STEP_IN = 4.0f;
inline constexpr float GLARE_FLOOR_MARGIN = 3.0f;
inline constexpr uint32 GLARE_DODGE_REFRESH_MS = 500;
// C'Thun's floor ends 48-64 yd from the Eye in places, with a 17-yard drop below it. Ground is searched from just
// above the floor, and counts as floor within this height of it.
inline constexpr float CTHUN_FLOOR_PROBE_HEIGHT = 2.0f;
inline constexpr float CTHUN_FLOOR_TOLERANCE = 3.0f;

// Shared state

// What the mechanic tracker found and assigned for the current encounter. Only the acting tracker writes it.
struct Aq40InstanceState
{
    ObjectGuid tracker;
    uint32 trackerSeen = 0;
    Aq40Encounter encounter = Aq40Encounter::None;
    ObjectGuid group;
    std::optional<uint32> encounterStart;
    GuidVector units;  // Sorted; every live unit of the encounter's entries near the boss.
    ObjectGuid mainTank;
    std::map<ObjectGuid, ObjectGuid> owners;  // Unit -> the tank that holds it.

    // The Prophet Skeram
    std::optional<std::array<Position, 3>> skeramPlatforms;
    std::array<ObjectGuid, 3> skeramPlatformTanks{};
    std::map<ObjectGuid, uint32> skeramMemberPlatform;
    std::map<ObjectGuid, uint32> skeramImagePlatform;
    std::map<ObjectGuid, Position> skeramImagePosition;

    // Princess Huhuran
    GuidVector huhuranSoakers;

    // Twin Emperors
    std::optional<std::array<Position, 2>> twinSides;  // Side 0 is Vek'nilash's spawn side.
    std::map<ObjectGuid, uint32> twinMemberSide;
    std::array<ObjectGuid, 2> twinTanks{};
    std::array<ObjectGuid, 2> twinLocks{};  // Per side: Square = side 0 (Vek'nilash's), Moon = side 1.
    ObjectGuid twinLock;                    // The warlock holding Vek'lor right now.
    ObjectGuid twinWaiter;
    std::optional<std::array<Position, 2>> twinLast;
    std::optional<uint32> twinLastSwap;

    // C'Thun
    ObjectGuid cthunPuller;  // Takes the three opening Eye Beams; stays put while the raid spreads.
    std::map<ObjectGuid, uint32> cthunSpots;
    bool cthunRedeal = false;
    bool glareActive = false;
    float lastGlare = 0.0f;
    float glareDirection = 0.0f;
    GuidVector cthunTentacleKillers;
};

Aq40InstanceState& Aq40State(uint32 instanceId);
bool Aq40ResetInstance(uint32 instanceId);

// Per-bot memory, kept in the bot's "aq40 bot cache" value.
struct Aq40BotCache
{
    uint32 glareDodgeTime = 0;
    Position glareDodge;
    uint32 waitAngleTime = 0;
    float waitAngle = 0.0f;
    uint32 spitAttemptTime = 0;
    uint32 exitAttemptTime = 0;
    // The pet's stance before Twin Emperors made it passive; the core saves the stance with the pet.
    std::optional<ReactStates> petReactBeforeTwins;
};

// General

Aq40Encounter GetCurrentEncounter(Player* bot);
bool IsInEncounter(Player* bot, Aq40Encounter encounter);
bool InStomach(Player* player);
bool Whirling(Unit* unit);
uint32 Stacks(Unit* unit, uint32 spell);
bool HasTrueFulfillment(Unit* unit);
bool IsEncounterBoss(uint32 entry);
bool IsTwinEmperor(Unit* unit);
Position Around(WorldObject const* center, float radius, float angle);
bool Attackable(Player* player, Unit* unit);
bool Passive(Unit* unit);
void StopMoving(PlayerbotAI* botAI);
void StopAttacking(Player* player);
SpellInfo const* SpellFor(PlayerbotAI* botAI, std::string const& name);
bool TryCast(PlayerbotAI* botAI, std::string const& name, Unit* target);
std::string TauntSpell(Player* player);
Aq40BotCache& BotCache(PlayerbotAI* botAI);

std::vector<Player*> GetMembers(Player* bot, bool tanks = false);
Player* GetMember(Player* bot, ObjectGuid guid);
std::vector<Creature*> GetUnits(Player* bot, uint32 entry = 0);
Creature* GetBoss(Player* bot);
Creature* GetCthunBody(Player* bot);
Player* GetTankFor(Player* bot, Unit* unit);
bool IsHeld(Player* bot, Unit* unit);
bool IsAllowedDamage(Player* bot, Unit* unit);
Unit* ComputeAssignedTarget(PlayerbotAI* botAI);
Unit* GetAssignedTarget(PlayerbotAI* botAI);

// Mechanic tracker

// A casting bot's AI does not tick, so the tracker's jobs pass to another bot once the holder falls silent.
bool IsActingTracker(Player* bot);
void TrackEncounter(Player* bot);
void AssignAddTanks(Player* bot);
bool SwapTanksOnStacks(Player* bot, uint32 aura, uint32 stacks);
void SwapFankrissTanks(Player* bot);
void SwapHuhuranTanks(Player* bot);
// False on bosses converted to standard assist, where the assigned-target value and its multipliers stand aside.
bool UsesAssignedTarget(Player* bot);

// The Prophet Skeram

void AssignSkeramPlatforms(Player* bot);
bool GetSkeramPlatform(PlayerbotAI* botAI, Position& platform);
Player* GetMindControlledMember(Player* bot);

// Battleguard Sartura

Creature* GetWhirlwindInRange(Player* bot);
std::vector<Creature*> GetSarturaKillOrder(Player* bot);

// Fankriss the Unyielding

Creature* FindFankrissKillTarget(Player* bot);
// The worm (or, failing that, hatchling) AssignAddTanks gave this tank.
Creature* FindFankrissOwnedSpawn(Player* bot);

// Princess Huhuran

void AssignHuhuranSoakers(Player* bot);
bool IsHuhuranSoaker(Player* bot);
Player* GetWyvernStungTank(Player* bot);

// Viscidus

Position ViscidusRing(Player* player, Creature* boss);
// Melee fighting him: all of them while he is frozen, melee with a frost proc otherwise.
bool IsViscidusRingMelee(PlayerbotAI* botAI);
bool IsNearToxin(Player* player, Position const& spot, float radius);
// The tank keeps clouds this far away, so the boss's center and the melee on the tank's side stand clear too.
float ToxinTankAvoidRadius(Creature* boss);
bool FindViscidusTankClearSpot(Player* bot, Creature* boss, Position& spot);
// A step to the nearest melee spot on the target's edge that is clear of clouds; false when already on one, or when
// every spot is covered (the melee then stay back).
bool GetViscidusMeleeStepClearOfToxin(Player* bot, Unit* target, float& stepX, float& stepY, float& stepZ);
bool GetStepOutOfNearestToxin(Player* bot, float& stepX, float& stepY, float& stepZ);
bool ViscidusShouldBeFrozen(Player* bot);
// A weapon enchant whose on-hit proc deals frost damage: Frost Oil, Frostbrand Weapon, Icy Weapon.
bool HasFrostWeaponProc(Player* bot);
// Melee damage dealers whose swings cannot add to the freeze hold the ring until he is frozen, instead of standing
// in the Toxin at his feet.
bool ViscidusMeleeWaitsForFreeze(PlayerbotAI* botAI);
// Melee that should get Frost Oil from the raid cheat.
bool ViscidusMeleeNeedsFrostOil(PlayerbotAI* botAI);
// Poison Bolt Volley's DoT is a poison; a Cleansing Totem strips it from the shaman's party every 3 s, so shamans keep
// one down for the fight. A Mana Tide Totem in the water slot is left alone.
bool HasViscidusWaterTotem(Player* bot);
bool ViscidusShamanNeedsCleansingTotem(Player* bot);

// Twin Emperors

void AssignTwinRoles(Player* bot);
bool IsTwinPlatformTank(Player* bot, Unit* unit);
bool IsTwinWaiter(Player* bot, Unit* unit);
bool IsTwinLock(Player* bot, Player* player);
bool IsTwinSpareTank(Player* bot, Player* player);
uint32 TwinSideOf(Aq40InstanceState const& state, Position const& pos);
Position TwinLockSpot(Aq40InstanceState const& state, uint32 side);
bool TwinSwapWindow(Player* bot);
bool TwinOpeningHold(Player* bot, Unit* target);
Position TwinUnstuckSpot(Player* bot, Unit* emperor);
Position TwinWaitSpot(PlayerbotAI* botAI, Unit* veklor);
bool IsHoldingVeknilash(Player* bot);
Creature* GetEmperorThatCannotReach(Player* bot);
Player* GetTwinHealFocus(Player* bot);

// C'Thun

void AssignCthunSpots(Player* bot);
void TrackDarkGlare(Player* bot);
void AssignCthunTentacleKillers(Player* bot);
void AssignCthunClawTanks(Player* bot);
void AcceptOuroTank(Player* bot);
bool StomachExitNeeded(Player* bot);
bool GetCthunSpot(Player* bot, Position& spot);
bool IsCthunPhase2(Player* bot);
bool IsCthunWeakened(Player* bot);
bool IsCthunTentacleKiller(Player* bot);
bool CthunMeleeBurn(PlayerbotAI* botAI);

// MoveTo() refuses a destination it believes is still being walked. Forgets the last one once the bot has stopped:
// a knockback may have cancelled it.
void ClearStaleLastMove(PlayerbotAI* botAI);
// The farthest point within 250 yd along the bot's path to the goal. Fails unless the path ends at the goal, or,
// with allowPartial, stops short of it.
bool GetRouteStep(Player* bot, Position goal, bool allowPartial, Position& next);
// A walkable step of up to 7 yd toward the goal, turned aside when the straight line is walled in. With units to
// avoid, the step farthest from them wins. Callers interrupt the bot's cast only after this succeeds, so a stuck
// healer keeps healing.
bool GetEscapeStep(Player* bot, Position const& goal, std::vector<Creature*> const& avoid, Position& step);

// Hazard escapes. Each returns where the bot should step to.

bool GetStomachExitGoal(PlayerbotAI* botAI, Position& goal);
bool GetToxinEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetPoisonCloudEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetDirtMoundEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetExplodingBugEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetDarkGlareEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetOuroConeEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetVeknilashLandingEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetVeklorEscapeGoal(PlayerbotAI* botAI, Position& goal);
bool GetBlizzardEscapeGoal(PlayerbotAI* botAI, Position& goal);

// Encounter positions. Each returns true while the bot is away from its spot.

bool GetSkeramPlatformGoal(PlayerbotAI* botAI, Position& goal);
bool GetTwinTankGoal(PlayerbotAI* botAI, Position& goal);
bool GetTwinWarlockGoal(PlayerbotAI* botAI, Position& goal);
bool GetTwinMeleeSwapGoal(PlayerbotAI* botAI, Position& goal);
bool GetTwinDpsGoal(PlayerbotAI* botAI, Position& goal);
bool GetTwinHealerGoal(PlayerbotAI* botAI, Position& goal);
// The bot's C'Thun position, whichever of the four below applies to its role and the phase.
bool GetCthunPositionGoal(PlayerbotAI* botAI, Position& goal);
bool GetCthunPullerHoldGoal(PlayerbotAI* botAI, Position& goal);
bool GetCthunMeleeBurnGoal(PlayerbotAI* botAI, Position& goal);
bool GetCthunSpotGoal(PlayerbotAI* botAI, Position& goal);
bool GetCthunStackGoal(PlayerbotAI* botAI, Position& goal);
bool GetTrioTankGoal(PlayerbotAI* botAI, Position& goal);
bool GetHuhuranSoakGoal(PlayerbotAI* botAI, Position& goal);
bool GetHuhuranSpreadGoal(PlayerbotAI* botAI, Position& goal);
bool GetViscidusRingGoal(PlayerbotAI* botAI, Position& goal);
bool GetBossRangeGoal(PlayerbotAI* botAI, Position& goal);

// Before the pull

bool GetTwinPrepullSpot(PlayerbotAI* botAI, Position& spot);
bool GetCthunPullerPrepullSpot(PlayerbotAI* botAI, Position& spot);
bool GetCthunTrailSpot(PlayerbotAI* botAI, Position& spot);

}  // namespace Aq40Helpers

#endif
