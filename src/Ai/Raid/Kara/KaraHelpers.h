/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAHELPERS_H
#define PLAYERBOTS_KARAHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

class Aura;
class Player;
class PlayerbotAI;
class Unit;

namespace KaraHelpers
{

struct KaraInstanceState;

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Boss ids from karazhan.h, matching DungeonEncounter.dbc's encounterIndex.
enum class KaraBosses : uint32
{
    DATA_ATTUMEN           = 0,
    DATA_MOROES            = 1,
    DATA_MAIDEN            = 2,
    DATA_OPERA_PERFORMANCE = 3,
    DATA_CURATOR           = 4,
    DATA_TERESTIAN         = 5,
    DATA_ARAN              = 6,
    DATA_NETHERSPITE       = 7,
    // DATA_CHESS_EVENT    = 8, // No strategy
    DATA_MALCHEZAAR        = 9,
    DATA_NIGHTBANE         = 10,
};

enum class KaraSpells : uint32
{
    // Trash
    SPELL_WARP_BREACH             = 29919,

    // Maiden of Virtue
    SPELL_REPENTANCE              = 29511,

    // The Big Bad Wolf
    SPELL_LITTLE_RED_RIDING_HOOD  = 30756,
    SPELL_GOBLIN_ROCKET_BOOTS     = 8892,  // the Little Red Riding Hood speed cheat (70% vs. 50%)
    SPELL_FEIGN_DEATH             = 5384,
    SPELL_INTERVENE               = 3411,

    // The Wizard of Oz
    SPELL_BURNING_STRAW           = 31075,

    // The Curator
    SPELL_CURATOR_EVOCATION       = 30254,

    // Terestian Illhoof
    SPELL_BROKEN_PACT             = 30065, // on Terestian from Kil'rek's death to his return
    SPELL_SACRIFICE               = 30115, // on the player until the Demon Chains die

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST       = 30004,
    SPELL_FLAME_WREATH_AURA       = 29946,
    SPELL_BLIZZARD                = 29951,
    SPELL_ARCANE_EXPLOSION        = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF         = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF       = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF        = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_NETHER_EXHAUSTION_RED   = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE  = 38639,
    SPELL_NETHERSPITE_BANISHED    = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                = 30843,
    SPELL_SHADOW_NOVA             = 30852,

    // Nightbane
    SPELL_CHARRED_EARTH           = 30129,
    SPELL_RAIN_OF_BONES           = 37098, // the channel; its ticks are 37091

    // Shaman
    SPELL_TREMOR_TOTEM            = 8143,
    SPELL_GROUNDING_TOTEM         = 8177,

    // Warlock
    SPELL_BANISH_RANK_1           = 710,
    SPELL_BANISH_RANK_2           = 18647,
};

enum class KaraNpcs : uint32
{
    // Trash
    NPC_MANA_WARP                    = 16530,

    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Moroes
    NPC_LADY_KEIRA_BERRYBUCK         = 17007,
    NPC_LADY_CATRIONA_VON_INDI       = 19872,
    NPC_LORD_CRISPIN_FERENCE         = 19873,
    NPC_BARON_RAFE_DREUGER           = 19874,
    NPC_BARONESS_DOROTHEA_MILLSTIPE  = 19875,
    NPC_LORD_ROBIN_DARIS             = 19876,

    // Romulo and Julianne
    NPC_ROMULO                       = 17533,
    NPC_JULIANNE                     = 17534,

    // The Wizard of Oz
    NPC_DOROTHEE                     = 17535,
    NPC_STRAWMAN                     = 17543,
    NPC_ROAR                         = 17546,
    NPC_TINHEAD                      = 17547,
    NPC_TITO                         = 17548,

    // The Curator
    NPC_THE_CURATOR                  = 15691,

    // Terestian Illhoof
    NPC_TERESTIAN_ILLHOOF            = 15688,
    NPC_KILREK                       = 17229,
    NPC_DEMON_CHAINS                 = 17248,
    NPC_FIENDISH_IMP                 = 17267,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    NPC_NETHERSPITE                  = 15689,
    NPC_VOID_ZONE                    = 16697,
    NPC_GREEN_PORTAL                 = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL                  = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                   = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_PRINCE_MALCHEZAAR            = 15690,
    NPC_NETHERSPITE_INFERNAL         = 17646,

    // Nightbane
    NPC_NIGHTBANE                    = 17225,
    NPC_RESTLESS_SKELETON            = 17261,
};

// Shared

inline constexpr uint32 KARA_MAP_ID = 532;

Unit* GetKaraBoss(Player* bot, KaraBosses boss);
// Barnes spawns the Opera cast non-attackable, and the play's release clears the flag, so an
// actor counts only once released, as the script itself tests.
Unit* GetOperaActor(Player* bot, std::string_view name);

// Shamans drop Tremor Totem while a Spectral Charger, the Big Bad Wolf, Dorothee, or Roar is
// fighting, or Nightbane is on the ground.
bool IsKarazhanFearCasterActive(Player* bot);

// Trash

// A Mana Warp casts Warp Breach at 10% HP. A stun that is on then and lasts past the cast's first
// retry (1.2s later) stops it for good, and so does a stun during its 1s cast. Interrupts do
// nothing. So warps are stunned from 15%, and during the cast if that stun misses or ends too soon.
inline constexpr float MANA_WARP_WARP_BREACH_HEALTH_PCT = 10.0f;
inline constexpr float MANA_WARP_STUN_HEALTH_PCT = 15.0f;
// At or below this, a DPS bot with a stun casts nothing else while targeting a Mana Warp, so its
// global cooldown is free at 15%. Rogues and cat druids first build combo points.
inline constexpr float MANA_WARP_STAND_BY_HEALTH_PCT = 25.0f;
// War Stomp has a 0.5s cast; other stunners leave its warp alone until it lands.
inline constexpr uint32 WAR_STOMP_CAST_MS = 500;
inline constexpr uint32 WAR_STOMP_CLAIM_MS = WAR_STOMP_CAST_MS + 200;
// Stun spell reaches, center to center.
inline constexpr float MELEE_STUN_REACH = 5.0f; // Kidney Shot, Bash, Maim, Concussion Blow
inline constexpr float HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float SHADOWFURY_REACH = 30.0f;
inline constexpr float WAR_STOMP_RADIUS = 8.0f;
// A stunner out of reach moves to this far inside max range.
inline constexpr float MANA_WARP_STUN_APPROACH_MARGIN = 1.0f;
inline constexpr float MANA_WARP_PACKS_RADIUS = 60.0f;
// The nine Mana Warps spawn in four packs within 30 yd of this point.
inline Position const MANA_WARP_PACKS_CENTER = { -11326.0f, -1781.0f, 179.8f };

bool IsNearManaWarpPacks(Player* bot);
float GetManaWarpStunReach(PlayerbotAI* botAI);
Unit* GetManaWarpToStun(PlayerbotAI* botAI);
void ClaimManaWarpForWarStomp(Unit* warp);
Unit* GetManaWarpToStunNext(PlayerbotAI* botAI);

// Attumen the Huntsman

// Shadow Cleave jumps from the tank to two more players within 10 yd in front of Attumen.
inline constexpr float SHADOW_CLEAVE_JUMP_RADIUS = 10.0f;
// Intangible Presence curses every player within 10 yd of Attumen.
inline constexpr float INTANGIBLE_PRESENCE_RADIUS = 10.0f;
inline constexpr float ATTUMEN_HAZARD_CLEARANCE = 1.0f;
inline constexpr uint32 ATTUMEN_MOUNTED_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
// Behind mounted Attumen, center to center. His CombatReach is 2.25 yd; a player's is 1.5, or 1.95
// with Bloodlust. Charge skips a player within 8 yd edge to edge (both reaches added): 11.75 yd
// center to center, 12.2 with Bloodlust. Hunter shots have no stored minimum range, so a shot
// fails only within melee range: 5.08 yd, 5.53 with Bloodlust. Hunters stand between the two.
inline constexpr float ATTUMEN_STACK_DISTANCE = 3.0f;
inline constexpr float ATTUMEN_HUNTER_STACK_DISTANCE = 8.5f;
inline constexpr float ATTUMEN_STACK_TOLERANCE = 0.3f;
inline Position const ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };

bool IsAttumenMounted(Unit* midnight);
Unit* GetMountedAttumen(Player* bot);
std::optional<Position> GetAttumenStackPosition(Player* bot, Unit* attumen);
bool IsAttumenDpsHeld(Player* bot);

// Moroes

// Around Moroes's spawn. Guests can't leave his room; he evades if one does.
inline constexpr float MOROES_GUEST_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 MOROES_GUEST_CACHE_INTERVAL_MS = 1000;

inline Position const MOROES_SPAWN_POSITION = { -10982.700f, -1877.930f, 81.812f };

inline std::vector<uint32> const MOROES_GUESTS_IN_KILL_ORDER = {
    Id(KaraNpcs::NPC_BARONESS_DOROTHEA_MILLSTIPE),
    Id(KaraNpcs::NPC_LADY_CATRIONA_VON_INDI),
    Id(KaraNpcs::NPC_LADY_KEIRA_BERRYBUCK),
    Id(KaraNpcs::NPC_BARON_RAFE_DREUGER),
    Id(KaraNpcs::NPC_LORD_ROBIN_DARIS),
    Id(KaraNpcs::NPC_LORD_CRISPIN_FERENCE),
};

GuidVector FindMoroesGuestGuids(Player* bot);

// Maiden of Virtue

// For both Maiden's distance to her position and the tank's distance to its destination. These are
// checked at different steps and don't have to be identical, but they are for simplicity.
inline constexpr float MAIDEN_OF_VIRTUE_POSITION_TOLERANCE = 2.0f;
// Maiden stops 6.8 yd from a tank she follows, so the tank aims this far past her position.
inline constexpr float MAIDEN_OF_VIRTUE_TANK_LEAD_DISTANCE = 7.0f;
// Holy Ground hits a player within 12 yd of Maiden's center and breaks Repentance.
inline constexpr float HOLY_GROUND_RADIUS = 12.0f;
inline constexpr float MAIDEN_REPENTANCE_BREAK_DISTANCE = HOLY_GROUND_RADIUS - 1.0f;

// Maiden's spawn point. The ranged positions are a ring around it.
inline Position const MAIDEN_OF_VIRTUE_POSITION = { -10945.881f, -2103.782f, 92.712f };
inline std::array const MAIDEN_OF_VIRTUE_RANGED_POSITIONS = {
    Position{ -10959.242f, -2119.617f, 92.180f }, // SE (opposite entrance)
    Position{ -10944.495f, -2123.857f, 92.180f }, // E
    Position{ -10966.017f, -2105.288f, 92.175f }, // S
    Position{ -10931.178f, -2116.580f, 92.179f }, // NE
    Position{ -10960.912f, -2090.437f, 92.179f }, // SW
    Position{ -10925.828f, -2102.425f, 92.180f }, // N
    Position{ -10947.590f, -2082.815f, 92.180f }, // W
    Position{ -10933.089f, -2088.502f, 92.180f }, // NW (entrance)
};

Player* GetNearestRepentantHealer(Player* bot, Unit* maiden);

// The Big Bad Wolf

struct BigBadWolfRunPlan
{
    uint8 index = 0;
    int32 direction = 1;
};

// The run turns for the next point when this close to the current one: one step.
inline constexpr float BIG_BAD_WOLF_RUN_TURN_DISTANCE = 3.5f;
// No step for this long means a new Hood or the end of a fear or stun, so the run is replanned.
inline constexpr uint32 BIG_BAD_WOLF_RUN_RESTART_MS = 1000;
inline constexpr int32 BIG_BAD_WOLF_PROTECT_REACTION_MS = 500;
inline constexpr float HAND_OF_PROTECTION_RANGE = 30.0f;
inline constexpr float POWER_WORD_SHIELD_RANGE = 40.0f;
inline constexpr float INTERVENE_MIN_RANGE = 8.0f;
inline constexpr float INTERVENE_MAX_RANGE = 25.0f;
// Hand of Protection (rank 3) lasts 10s, so it waits for the end of Hood or a target in trouble.
// One hit does 6.8-9.6k damage (Hood reduces armor to 0).
inline constexpr int32 HAND_OF_PROTECTION_DURATION_MS = 10 * IN_MILLISECONDS;
inline constexpr float BIG_BAD_WOLF_PROTECT_HEALTH_PCT = 50.0f;
// The front-left corner of the stage, to allow a better starting position for the run.
inline Position const BIG_BAD_WOLF_TANK_POSITION = { -10913.391f, -1773.508f, 90.477f };
// Points near the stage edge, all the way around, clear of trees and in front of the backdrop.
inline std::array const BIG_BAD_WOLF_RUN_POSITIONS = {
    Position{ -10873.346f, -1767.691f, 90.477f },
    Position{ -10873.569f, -1760.625f, 90.477f },
    Position{ -10876.827f, -1753.725f, 90.477f },
    Position{ -10883.397f, -1751.176f, 90.477f },
    Position{ -10893.279f, -1750.877f, 90.477f },
    Position{ -10901.284f, -1750.492f, 90.477f },
    Position{ -10908.019f, -1751.260f, 90.477f },
    Position{ -10912.599f, -1754.285f, 90.477f },
    Position{ -10913.570f, -1761.219f, 90.477f },
    Position{ -10912.652f, -1768.362f, 90.477f },
    Position{ -10908.455f, -1775.869f, 90.477f },
    Position{ -10899.897f, -1780.339f, 90.477f },
    Position{ -10888.966f, -1781.551f, 90.477f },
    Position{ -10879.527f, -1778.071f, 90.477f },
};

bool IsLittleRedRidingHood(Player* bot);
uint8 GetNextBigBadWolfRunIndex(uint8 index, int32 direction);
BigBadWolfRunPlan GetBigBadWolfRunPlan(Player* bot, Unit& wolf, float seconds);
Player* GetBigBadWolfRedHood(Unit* wolf);
Player* GetBigBadWolfTargetToProtect(Unit* wolf);

// Romulo and Julianne

// Once one boss is down, the other must die within 10s, or both revive. When one gets more than
// this much health below the other, the skull target is swapped to the higher health boss.
inline constexpr uint32 ROMULO_AND_JULIANNE_HEALTH_GAP = 25000;

// Wizard of Oz

inline constexpr float SCORCH_RANGE = 30.0f;
// A mage out of Scorch range moves to this far inside it to cast.
inline constexpr float SCORCH_APPROACH_MARGIN = 2.0f;
inline std::array const OZ_TARGETS = {
    "dorothee",
    "tito",
    "roar",
    "strawman",
    "tinhead",
};

// The Curator

// Arcing Sear jumps 13 yd center-to-center between players. Requiring that great of a spread is
// impractical, so we enforce a smaller spread to at least reduce the jumps.
inline constexpr float THE_CURATOR_RANGED_SPREAD_DISTANCE = 7.0f;
// Toward the far end of the Menagerie, near the Library entrance, in the middle of the corridor.
inline Position const THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };

// Terestian Illhoof

bool IsTerestianSacrificeActive(Player* bot);

// Shade of Aran

// No reach added.
inline constexpr float ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS = 21.0f;
inline constexpr float ARAN_ARCANE_EXPLOSION_CLEARANCE = ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS + 1.0f;
// The pull lands everyone on his spot, so each bot takes its own way out.
inline constexpr float ARAN_PULLED_ONTO_BOSS_DISTANCE = 1.0f;
inline constexpr uint32 ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS = 16;
// No reach added.
inline constexpr float ARAN_COUNTERSPELL_HAZARD_RADIUS = 9.0f;
// Ranged distance from Aran, center to center, outside Counterspell and inside most Blizzards.
inline constexpr float ARAN_RANGED_MIN_DISTANCE = ARAN_COUNTERSPELL_HAZARD_RADIUS + 1.0f;
inline constexpr float ARAN_RANGED_MAX_DISTANCE = 13.0f;
inline constexpr float ARAN_RANGED_BAND_TOLERANCE = 0.5f;
inline constexpr uint32 ARAN_ELEMENTALS_CACHE_INTERVAL_MS = 1000;
// 13 yd + player reach.
inline constexpr float ARAN_BLIZZARD_HAZARD_RADIUS = 14.5f;
inline constexpr float ARAN_BLIZZARD_CLEARANCE = ARAN_BLIZZARD_HAZARD_RADIUS + 0.5f;
inline constexpr float ARAN_ROOM_SEARCH_RADIUS = 60.0f;
// Ranged run here from a patch, measured from the room center. A single Blizzard is actually a
// "train" of connected patches that are added onto the train's head and move clockwise. Blizzard
// stop short of a full lap, so behind its oldest patch is always safe.
inline constexpr float ARAN_BLIZZARD_ESCAPE_DISTANCE = ARAN_RANGED_MIN_DISTANCE + 0.5f;
inline constexpr float ARAN_BLIZZARD_ESCAPE_ANGLE = static_cast<float>(M_PI) * 40.0f / 180.0f;
// 3 yd + player reach. Crossing the ring blows the raid up, and the unfortunate idiot who did it
// is also subjected to a vertical knockback so everybody knows who is at fault.
inline constexpr float ARAN_FLAME_WREATH_RING_HAZARD_RADIUS = 4.5f;
// How close a move can pass by a Flame Wreath ring and be permitted. Intentionally a big buffer.
inline constexpr float ARAN_FLAME_WREATH_REACH_CLEARANCE =
    ARAN_FLAME_WREATH_RING_HAZARD_RADIUS + 5.0f;
// Targets are picked when the cast ends, so everyone has to be standing still by then.
inline constexpr int32 ARAN_FLAME_WREATH_STOP_LEAD_MS = 1500;
inline constexpr int32 ARAN_FLAME_WREATH_RING_DURATION_MS = 20 * IN_MILLISECONDS;
// No one moves this soon after the rings land, which also outlasts the ring value's cache.
inline constexpr int32 ARAN_FLAME_WREATH_SETTLE_MS = 1000;
inline constexpr uint32 ARAN_FLAME_WREATH_RING_CACHE_INTERVAL_MS = 500;

// Aran blinks here before each super spell. His Blizzard circles around it, so this is used to
// calculate the Blizzard escape position.
inline Position const ARAN_ROOM_CENTER = { -11164.8f, -1912.0f, 232.1f };

bool IsAranCastingArcaneExplosion(Unit* aran);
std::optional<Position> GetBlizzardEscapePosition(Player* bot);
bool IsFlameWreathInProgress(PlayerbotAI* botAI, Unit* aran);
bool CanReachTargetDuringFlameWreath(PlayerbotAI* botAI, Unit* aran);
GuidVector FindUnbanishedConjuredElementals(Player* bot);

// Netherspite

enum class NetherspiteBeam : uint8
{
    Red,
    Green,
    Blue,
};

struct NetherspiteBeamData
{
    NetherspiteBeam beam;
    KaraNpcs portal;
    KaraSpells buff;
    KaraSpells exhaustion;
};

struct NetherspiteBeamState
{
    std::optional<Position> portal;
    ObjectGuid blocker;
    ObjectGuid next;
    Position blockerPosition;
    Position nextPosition;
};

struct NetherspiteBeamOffset
{
    float along;
    float off;
};

struct NetherspiteBanishSlot
{
    ObjectGuid guid;
    Position position;
};

// The tracker assigns blockers in this order. An unblocked green beam heals Netherspite 4000 a
// second per stack, so if there is a tie, the green beam gets priority.
inline constexpr std::array NETHERSPITE_BEAMS = {
    NetherspiteBeamData{
        NetherspiteBeam::Red, KaraNpcs::NPC_RED_PORTAL, KaraSpells::SPELL_RED_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_RED,
    },
    NetherspiteBeamData{
        NetherspiteBeam::Green, KaraNpcs::NPC_GREEN_PORTAL, KaraSpells::SPELL_GREEN_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN,
    },
    NetherspiteBeamData{
        NetherspiteBeam::Blue, KaraNpcs::NPC_BLUE_PORTAL, KaraSpells::SPELL_BLUE_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_BLUE,
    },
};

// A player blocks a beam within 1.5 yd of the beam and more than 16.5 yd from Netherspite's center.
// The script compares distances to the portal with combat reach subtracted, so Netherspite's reach
// of 18 yd minus a player's 1.5 means the first 16.5 yd of the beam cannot be blocked.
inline constexpr float NETHERSPITE_BEAM_WIDTH = 1.5f;
inline constexpr float NETHERSPITE_BEAM_START = 16.5f;
inline constexpr float NETHERSPITE_BEAM_CLEARANCE = NETHERSPITE_BEAM_WIDTH + 1.5f;
// The red beam dance moves the tank 3 yd sideways.
inline constexpr float NETHERSPITE_RED_DANCE_STEP = 3.0f;
// Beams start 10s into a portal phase. Between 0 and 5s, all non-tanks hold DPS. Between 5s and
// 10s, non-tanks hold DPS unless a tank is the victim. After beams spawn, nobody holds DPS.
inline constexpr uint32 NETHERSPITE_DPS_HOLD_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 NETHERSPITE_BEAMS_START_MS = 10 * IN_MILLISECONDS;
// The portal phase lasts for 60s, but beam swaps are blocked for the final 5s. With a 25-stack
// limit, the second blocker can exceed the limit just before the end of the phase, if a mid-phase
// swap proceeds flawlessly. In that case, the second blocker holds out instead of forcing a late
// swap that would waste exhaustion on another bot that may be needed for the next portal phase.
inline constexpr uint32 NETHERSPITE_LAST_SWAP_MS = 55 * IN_MILLISECONDS;
// The active blue or mana-user green blocker leaves a beam at 25 stacks, while the next blocker
// moves onto the beam at 24 stacks to give it a second to arrive (and finish a cast).
inline constexpr uint8 NETHERSPITE_BEAM_STACK_LIMIT = 25;
inline constexpr uint8 NETHERSPITE_NEXT_STEPS_ON_AT_STACKS = 24;
// Random positions a search tries per tick before giving up until the next tick.
inline constexpr uint8 NETHERSPITE_POSITION_TRIES = 12;
// Blue and green blockers stand 18 yd from Netherspite (hunters 25), moving along the beam if in
// a void zone, but never closer than 17 yd. The red blocker always stands at 18 yd.
inline constexpr float NETHERSPITE_BLOCKER_DISTANCE = 18.0f;
inline constexpr float NETHERSPITE_BLOCKER_MIN_DISTANCE = 17.0f;
inline constexpr float NETHERSPITE_HUNTER_BLOCKER_DISTANCE = 25.0f;
// The next blocker waits 3.5-7 yd to the side of its beam: outside the beam's clearance even if it
// stops short by the tolerance, but close enough to step into the beam's width in less than 1s.
inline constexpr float NETHERSPITE_BEAM_POSITION_TOLERANCE = 0.5f;
inline constexpr float NETHERSPITE_NEXT_MIN_OFF_LINE =
    NETHERSPITE_BEAM_CLEARANCE + NETHERSPITE_BEAM_POSITION_TOLERANCE;
inline constexpr float NETHERSPITE_NEXT_MAX_OFF_LINE = 7.0f;
// The next blocker's waiting position is 17-19.5 yd from Netherspite along its beam (hunters
// 25-27.5), close to the distance it will block from.
inline constexpr float NETHERSPITE_NEXT_ALONG_RANGE = 2.5f;
// To get clear of a void zone, the waiting position can move up to this far past either end of
// NETHERSPITE_NEXT_ALONG_RANGE (but hunters only away from Netherspite).
inline constexpr float NETHERSPITE_NEXT_SHIFT_RANGE = 4.0f;
// See GetNetherspiteStandingArc.
inline constexpr float NETHERSPITE_MAX_STANDING_ARC = static_cast<float>(M_PI) * 170.0f / 180.0f;
// Heals reach 43 yd center to center. NETHERSPITE_RED_DANCE_STEP is 3 yd, so 38 yd accounts for
// that movement plus 2 yd of margin.
inline constexpr float NETHERSPITE_HEALER_TANK_RANGE = 38.0f;
// Standard player run speed, used to determine if a bot can pass between a portal and Netherspite
// at the beginning of a portal phase before the beam spawns (CrossesBeamAfterStart).
inline constexpr float NETHERSPITE_BOT_SPEED = 7.0f;

// No reach added.
inline constexpr float NETHERSPITE_VZ_HAZARD_RADIUS = 2.0f;
inline constexpr float NETHERSPITE_VZ_CLEARANCE = NETHERSPITE_VZ_HAZARD_RADIUS + 1.0f;
// This clearance is 0.5 yd short of the ordinary 3 yd void zone clearance because the standard
// blocking position is 18 yd from Netherspite, and 21 yd would be out of melee range (20.8 yd).
inline constexpr float VOID_ZONE_BEAM_POSITION_CLEARANCE = NETHERSPITE_VZ_HAZARD_RADIUS + 0.5f;
inline constexpr float VOID_ZONE_STEP_AROUND_DISTANCE = NETHERSPITE_VZ_CLEARANCE + 0.5f;
// Tried when neither side of the void zone is clear at the normal step-around distance.
inline constexpr float VOID_ZONE_WIDE_STEP_AROUND_DISTANCE = VOID_ZONE_STEP_AROUND_DISTANCE + 2.0f;

// Banish slots: every group member gets a bearing from Netherspite, spread evenly so each
// Netherbreath (a 45° cone) hits only one bot. Slots are 20 yd out (inside his melee range of
// 20.8 yd), except 25 yd for hunters (outside melee range, or Auto Shot fails).
inline constexpr float NETHERSPITE_BANISH_SLOT_RADIUS = 20.0f;
inline constexpr float NETHERSPITE_HUNTER_BANISH_SLOT_RADIUS = 25.0f;
// A bot more than this far off its slot's bearing moves back. Ten slots are 36° apart, and the cone
// reaches 22.5° each side, so two neighbors each 6° off toward each other are still clear.
inline constexpr float NETHERSPITE_BANISH_SLOT_TOLERANCE =
    static_cast<float>(M_PI) * 6.0f / 180.0f;
// A bot more than 5 yd farther from Netherspite than its slot moves back in. Netherbreath knocks
// a bot about 25 yd out along its bearing, potentially out of healing range.
inline constexpr float NETHERSPITE_BANISH_SLOT_RADIUS_TOLERANCE = 5.0f;

bool IsBanishPhase(Unit* netherspite);
bool IsManaUser(Player* player);
bool IsNetherspiteDpsHeld(Player* bot, Unit* netherspite);
bool HasNetherspiteBeamStackLimit(Player* player, NetherspiteBeam beam);
uint8 GetNetherspiteBeamStacks(Player* player, NetherspiteBeamData const& data);
bool AreNetherspitePortalsKnown(KaraInstanceState const& state);
bool IsNetherspiteBlockerOrNext(KaraInstanceState const& state, ObjectGuid guid);
Position const* FindNetherspiteBlockerPosition(KaraInstanceState const& state, ObjectGuid guid);
Position const* FindNetherspiteNextPosition(KaraInstanceState const& state, ObjectGuid guid);
NetherspiteBanishSlot const* FindNetherspiteBanishSlot(
    KaraInstanceState const& state, ObjectGuid guid);
void YellNetherspiteBeamText(Player* player, std::string const& key, std::string const& text);
void YellNetherspiteBeamHandover(Player* leaving, Player* blocking, NetherspiteBeam beam);
NetherspiteBeamOffset GetNetherspiteBeamOffset(
    Unit& netherspite, Position const& portal, float x, float y);
bool IsOnNetherspiteBeam(Unit* netherspite, Position const& portal, Unit* unit);
Position GetPositionOnNetherspiteBeam(Unit& netherspite, Position const& portal, float distance);
bool IsClearOfNetherspiteBeams(
    Unit& netherspite, KaraInstanceState const& state, float x, float y);
bool IsClearOfVoidZones(KaraInstanceState const& state, float x, float y, float clearance);
bool CrossesVoidZone(KaraInstanceState const& state, Position const& from, Position const& to);
float FindNetherspiteBeamPositionDistance(
    Unit& netherspite, Position const& portal, KaraInstanceState const& state, Player* blocker);
float GetNetherspiteInnerSide(
    Unit& netherspite, KaraInstanceState const& state, NetherspiteBeam beam);
bool IsBetweenNetherspiteBeams(
    Player* bot, Unit& netherspite, KaraInstanceState const& state, float x, float y);
bool CanMoveBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state, float x, float y);
std::optional<Position> FindPositionBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state);
std::optional<Position> GetStepAroundVoidZones(
    Player* bot, Unit* netherspite, KaraInstanceState const& state, Position const& destination);

// Prince Malchezaar

// Hellfire and Shadow Nova do not add player reach.
inline constexpr float MALC_HELLFIRE_HAZARD_RADIUS = 20.0f;
inline constexpr float MALC_HELLFIRE_CLEARANCE = MALC_HELLFIRE_HAZARD_RADIUS + 1.0f;
inline constexpr float MALC_SHADOW_NOVA_HAZARD_RADIUS = 30.0f;
inline constexpr float MALC_SHADOW_NOVA_CLEARANCE = MALC_SHADOW_NOVA_HAZARD_RADIUS + 1.0f;
inline constexpr int32 MALC_SHADOW_NOVA_CAST_TIME_MS = 2 * IN_MILLISECONDS;
// Malchezaar's melee range for a player: 4 yd (his reach) + 1.5 yd + 4/3 yd.
inline constexpr float MALCHEZAAR_MELEE_RANGE = 6.83f;
// Malchezaar being this far from an active Infernal means it is safe to stand in melee range.
inline constexpr float MALCHEZAAR_MELEE_CLEARANCE =
    MALC_HELLFIRE_CLEARANCE + MALCHEZAAR_MELEE_RANGE;
// Malchezaar ends about this far behind a tank moving him.
inline constexpr float MALCHEZAAR_TRAIL_DISTANCE = 6.0f;
// The model an Infernal has until Infernal Rain lands on it.
inline constexpr uint32 INFERNAL_MODEL_INVISIBLE = 11686;
// Around Malchezaar's spawn. Infernals land on targets within 69 yd of it.
inline constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
inline constexpr float MALCHEZAAR_SEARCH_DISTANCE = 60.0f;
// Bounds the pathfinding cost of each position search.
inline constexpr uint8 MALCHEZAAR_MAX_WALK_CHECKS = 64;
// A smaller arrival distance than is typical, due to the small safety margins used for hazards for
// this fight, which requires usage of an ever-shrinking safe playable space.
inline constexpr float MALCHEZAAR_ARRIVAL_DISTANCE = 0.5f;
inline constexpr uint32 INFERNALS_CACHE_INTERVAL_MS = 200;

inline Position const MALCHEZAAR_SPAWN_POSITION = { -10962.200f, -2018.730f, 275.443f };

bool CanShadowNovaHitEnfeebled(Unit* malchezaar, Aura const& enfeeble);
GuidVector FindInfernalGuids(Player* bot);
std::vector<Position> GetLandedInfernals(Player* bot);
bool IsClearOfInfernals(
    Position const& position, std::vector<Position> const& infernals, float clearance);
bool IsLineClearOfInfernals(
    Position const& from, Position const& to, std::vector<Position> const& infernals);
std::optional<Position> FindNearestMalchezaarSafePosition(
    Player* bot, Position const& origin, std::vector<Position> const& infernals,
    float maxDistance, bool mayCross, float infernalClearance, Unit* novaSource);
std::optional<Position> FindMalchezaarClearPosition(
    PlayerbotAI* botAI, Unit* malchezaar, std::vector<Position> const& infernals);
bool IsClearOfMalchezaarHazards(
    Player* bot, Unit* malchezaar, std::vector<Position> const& infernals,
    Position const& position);
std::optional<Position> FindMalchezaarTankPosition(
    Player* bot, std::vector<Position> const& infernals, float& clearance);

// Nightbane

enum class NightbaneFlightStage : uint8
{
    TakeOff,
    Hover,   // At the flight position, from which Nightbane casts flight phase spells
    Landing,
};
enum class NightbaneRangedTier : uint8
{
    Parapet,
    Band,
    Forward,
};
enum class NightbaneMeleePosition : uint8
{
    ClockwiseFlank,
    CounterclockwiseFlank,
    Hold,
};

// A Z-position is used for flight instead of a flag due to past experience. At one point, AC broke
// unit flags, resulting in Nightbane no longer being marked as flying during the flight phase.
inline constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
inline constexpr float NIGHTBANE_GROUND_Z = 91.473f;
inline constexpr uint32 NIGHTBANE_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
inline constexpr float CHARRED_EARTH_HAZARD_RADIUS = 11.5f; // 10 yd patch + 1.5 yd player reach
inline constexpr float CHARRED_EARTH_CLEARANCE = CHARRED_EARTH_HAZARD_RADIUS + 0.5f;
inline constexpr float CHARRED_EARTH_SEARCH_RADIUS = 80.0f;
inline constexpr uint32 CHARRED_EARTH_CACHE_INTERVAL_MS = 200;
// Each Rain of Bones tick hits everyone within 6 yd of a random point up to 8 yd from its center
// (no combat reach added), so the rain reaches 14 yd.
inline constexpr float RAIN_OF_BONES_HAZARD_RADIUS = 14.0f;
inline constexpr float RAIN_OF_BONES_CLEARANCE = RAIN_OF_BONES_HAZARD_RADIUS + 1.0f;
inline constexpr int32 RAIN_OF_BONES_REACTION_MS = 1000;
// Nightbane's final flight phase starts when his HP drops below 25%. In AC, he is out of LoS in
// flight, so his HP is similar at landing for the final (ground) phase. Using 20% covers variance.
inline constexpr float NIGHTBANE_FINAL_PHASE_HEALTH_PCT = 20.0f;
// The terrace is a ring around the dome. Terrace angles are measured around the dome, running
// counterclockwise from the northwest end to the northeast end.
inline constexpr float TERRACE_INNER_RADIUS = 36.0f; // the dome's wall is at 33.5-35.8 yd
inline constexpr float TERRACE_OUTER_RADIUS = 51.0f;
inline constexpr float TERRACE_ARC_RADIUS = 47.856f;
inline constexpr float TERRACE_ARC_START = 0.2418f;
inline constexpr float TERRACE_ARC_END = 4.9241f;
// The floor runs from 35.8 to 49.4 yd from the dome center at every terrace angle (navmesh).
inline constexpr float TERRACE_FLOOR_INNER_RADIUS = 35.8f;
inline constexpr float TERRACE_FLOOR_OUTER_RADIUS = 49.4f;
// A lane around the dome, on the floor at every angle. The flight stack sits on it, and steps
// around the terrace stay on it or further out. It is 10.5 yd in from the parapet, so skeletons
// appearing up to 10 yd from a Rain of Bones on the stack stay on the terrace.
inline constexpr float TERRACE_INNER_LANE_DIST = 39.0f;
// In flight, Nightbane casts Fireball Barrage at players more than 52 yd from his center (40 yd
// plus both combat reaches), so the flight stack stays within 50 yd of his flight position.
inline constexpr float NIGHTBANE_FLIGHT_STACK_MIN_ANGLE = 1.2566f;
inline constexpr float NIGHTBANE_FLIGHT_STACK_MAX_ANGLE = 3.0020f;
inline constexpr float NIGHTBANE_FLIGHT_STACK_HOME_ANGLE = 2.0944f;
// Melee stand on one of Nightbane's flanks, at an angle from his facing, out of Smoldering Breath,
// Cleave, and Tail Sweep. A Charred Earth on the tank, who stands 3.9-10 yd in front of him, misses
// both flanks. When other patches cover both flanks, melee wait at the hold, outside his melee
// range on his counterclockwise side.
inline constexpr float NIGHTBANE_MELEE_FLANK_ANGLE = 1.8850f; // 108°
inline constexpr float NIGHTBANE_MELEE_FLANK_DIST = 10.5f;
inline constexpr float NIGHTBANE_MELEE_HOLD_ANGLE = 1.7453f; // 100°
inline constexpr float NIGHTBANE_MELEE_HOLD_DIST = 23.0f;
// Ranged distances are calculated to keep melee safe from a Charred Earth that lands on ranged
// positions, while allowing heals (43 yd range) to reach melee on Nightbane's far flank.
inline constexpr float NIGHTBANE_RANGED_MIN_DIST =
    NIGHTBANE_MELEE_FLANK_DIST + CHARRED_EARTH_CLEARANCE;
inline constexpr float NIGHTBANE_RANGED_MAX_DIST = 43.0f - NIGHTBANE_MELEE_FLANK_DIST;

inline Position const NIGHTBANE_FLIGHT_POSITION = { -11160.125f, -1870.683f, 97.739f };
inline Position const TERRACE_DOME_CENTER =       { -11126.015f, -1925.271f, 91.473f };

float GetTerraceAngle(Position const& position);
Position GetTerraceWalkPosition(Position const& from, Position const& destination);
std::optional<Position> FindNightbaneStackPosition(
    float startAngle, std::vector<Position> const& charredEarths, Position const* rainCenter);
std::vector<Position> const& GetCharredEarths(PlayerbotAI* botAI);
bool IsClearOfCharredEarth(Position const& position, std::vector<Position> const& charredEarths);
bool IsOnNightbaneRangedSide(Position const& position, Unit& nightbane);
bool IsInNightbaneRangedSight(Player& bot, Position const& from, Position const& to);
std::optional<Position> FindNightbaneRangedPosition(
    Player& bot, Unit& nightbane, Position const& from, std::vector<Position> const& charredEarths,
    NightbaneRangedTier tier);
Position GetNightbaneMeleePosition(Unit& nightbane, NightbaneMeleePosition position);
bool HoldsNightbaneGroundPosition(PlayerbotAI* botAI, Unit& nightbane);
bool IsOutsideNightbaneTerrace(Unit* unit);
bool IsNightbaneFlying(Unit* nightbane);
bool IsNightbaneFlightPhaseActive(Unit* nightbane);
bool IsNightbaneDpsHeld(Player* bot, Unit* nightbane);
Position const* GetRainOfBonesCenter(Unit* nightbane);

// Shared encounter state

struct KaraInstanceState
{
    std::optional<uint32> attumenDpsHoldTimer;
    EncounterHelpers::InstanceCreatureList moroesGuests;
    std::optional<uint32> netherspitePortalPhaseTimer;
    std::array<NetherspiteBeamState, NETHERSPITE_BEAMS.size()> netherspiteBeams;
    std::vector<Position> netherspiteVoidZones;
    std::vector<NetherspiteBanishSlot> netherspiteBanishSlots;
    EncounterHelpers::InstanceCreatureList malchezaarInfernals;
    std::optional<uint32> nightbaneDpsHoldTimer;
    std::optional<NightbaneFlightStage> nightbaneFlightStage;
    std::optional<Position> nightbaneFlightStackPosition;
    std::optional<Position> nightbaneRainOfBonesCenter;
    std::optional<NightbaneMeleePosition> nightbaneMeleePosition;
    std::optional<Position> nightbaneRangedPosition;
};

KaraInstanceState& KaraState(uint32 instanceId);
KaraInstanceState const* FindKaraState(uint32 instanceId);
bool KaraResetInstance(uint32 instanceId);

}

#endif
