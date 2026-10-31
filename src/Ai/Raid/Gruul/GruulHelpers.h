/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULHELPERS_H
#define PLAYERBOTS_GRUULHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "SharedDefines.h"
#include <type_traits>
#include <vector>

class Player;
class Unit;

namespace GruulHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class GruulSpells : uint32
{
    // High King Maulgar <Lord of the Ogres>
    SPELL_WHIRLWIND         = 33238,

    // Krosh Firehand
    SPELL_SPELL_SHIELD      = 33054,

    // Hunter
    SPELL_MISDIRECTION      = 34477,
    SPELL_MISDIRECTION_AURA = 35079, // the aura on the hunter

    // Mage
    SPELL_SPELLSTEAL        = 30449,

    // Gruul the Dragonkiller
    SPELL_GROUND_SLAM_1     = 33525,
    SPELL_GROUND_SLAM_2     = 39187,
    SPELL_CAVE_IN           = 36240,
};

enum class GruulNpcs : uint32
{
    // High King Maulgar <Lord of the Ogres>
    NPC_HIGH_KING_MAULGAR  = 18831,
    NPC_KROSH_FIREHAND     = 18832,
    NPC_OLM_THE_SUMMONER   = 18834,
    NPC_KIGGLER_THE_CRAZED = 18835,
    NPC_BLINDEYE_THE_SEER  = 18836,
    NPC_WILD_FEL_STALKER   = 18847,
};

inline constexpr uint32 GRUUL_MAP_ID = 565;

// High King Maulgar

// Ogre combat reaches:
// (1) Maulgar = 3.5y, (2) Olm = 2.2y, (3) Blindeye = 3.525y, (4) Krosh = 2y, (5) Kiggler = 3.3y

inline constexpr EncounterHelpers::ClassRole KROSH_MAGE_TANK_ROLE = {
    .playerClass = CLASS_MAGE,
    .role = { "class_role_krosh_mage_tank", "Krosh mage tank" },
    .candidate = { "class_role_class_mage", "Mage" },
};
inline constexpr EncounterHelpers::ClassRole KIGGLER_MOONKIN_TANK_ROLE = {
    .playerClass = CLASS_DRUID,
    .requiredSpec = DRUID_TAB_BALANCE,
    .role = { "class_role_kiggler_moonkin_tank", "Kiggler moonkin tank" },
    .candidate = { "class_role_class_balance_druid", "Balance Druid" },
};
// 90 yards is about the distance from Maulgar to where the hallway opens into the council room.
inline constexpr float MAULGAR_CLASS_ROLE_ANNOUNCE_DISTANCE = 90.0f;
// 150 yards is short of the instance portal and Gruul's room.
inline constexpr float MAULGAR_CLASS_ROLE_REARM_DISTANCE = 150.0f;

// Hold cooldowns until Blindeye is at this percent health. Also the gate for on-pull Misdirects.
inline constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;
inline constexpr float KIGGLER_ARCANE_EXPLOSION_HAZARD_RADIUS = 30.0f;
// Stays inside the "enemy out of spell" threshold (spellDistance + CONTACT_DISTANCE + both reaches.
// ~34y exact against Kiggler), allowing the boomie to attack Kiggler safely without Nature's Reach.
inline constexpr float KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE =
    KIGGLER_ARCANE_EXPLOSION_HAZARD_RADIUS + 2.0f;
inline constexpr float KROSH_BLAST_WAVE_HAZARD_RADIUS = 15.0f;
inline constexpr float KROSH_BLAST_WAVE_SAFE_DISTANCE = KROSH_BLAST_WAVE_HAZARD_RADIUS + 2.0f;
// Whirlwind's padding is greater than usual. The tl;dr is MoveAway()'s math results in its fallback
// candidates (when running straight away is blocked) moving the bot less than the specified
// distance. Melee dps often ends up between Maulgar and a wall and needing fallback escape angles.
inline constexpr float MAULGAR_WHIRLWIND_HAZARD_RADIUS = 8.0f;
inline constexpr float MAULGAR_WHIRLWIND_SAFE_DISTANCE = MAULGAR_WHIRLWIND_HAZARD_RADIUS + 4.0f;
inline constexpr float MAULGAR_WHIRLWIND_HOLD_DISTANCE = 15.0f;
inline constexpr uint32 WILD_FEL_STALKER_CACHE_INTERVAL_MS = 1000;
inline constexpr float WILD_FEL_STALKER_SEARCH_RADIUS = 50.0f;

inline Position const MAULGAR_SPAWN_POSITION = { 145.786f, 190.979f, -11.542f };
inline Position const MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
inline Position const OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
inline Position const BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
inline Position const KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
inline Position const GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

bool IsMaulgarTank(Player* bot);
bool IsOlmTank(Player* bot);
bool IsBlindeyeTank(Player* bot);
bool UpdateGruulClassRoles(PlayerbotAI* botAI);
bool UpdateMaulgarClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetKroshMageTank(PlayerbotAI* botAI);
bool IsKroshMageTank(PlayerbotAI* botAI);
Player* GetKigglerMoonkinTank(PlayerbotAI* botAI);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI);
GuidVector FindNearbyWildFelStalkerGuids(Player* bot);
std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* botAI);

// Gruul the Dragonkiller

inline constexpr uint32 CAVE_IN_CACHE_INTERVAL_MS = 100;
// Radius is 8y, plus the 1.5y CombatReach the membership check adds for the player, rounded up.
// The pool lasts 15s and is cast as often as every 3.5s, so 5 can be on the ground at once.
inline constexpr float CAVE_IN_RADIUS = 10.0f;
// Distance for the multiplier. As usual, wider than the avoidance to prevent running back in.
inline constexpr float CAVE_IN_CONTROL_RADIUS = CAVE_IN_RADIUS + 5.0f;
inline constexpr float CAVE_IN_SEARCH_RADIUS = CAVE_IN_CONTROL_RADIUS + 2.0f;
// Radius is 20y with 2y of MoveAway padding. Sort of. The details are not really important; I note
// only that damage has a linear relationship with distance.
inline constexpr float GRUUL_SHATTER_SAFE_DISTANCE = 22.0f;

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool);
bool IsNearCaveIn(PlayerbotAI* botAI, float radius);
bool IsInCaveIn(PlayerbotAI* botAI);
bool HasGroundSlam(Player* bot);

// Shared encounter state

struct GruulInstanceState
{
    EncounterHelpers::ClassRoleAssignment kroshMageTank;
    EncounterHelpers::ClassRoleAssignment kigglerMoonkinTank;
};

GruulInstanceState& GruulState(uint32 instanceId);
GruulInstanceState* FindGruulState(uint32 instanceId);

}

#endif
