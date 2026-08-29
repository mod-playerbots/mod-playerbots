/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULHELPERS_H
#define PLAYERBOTS_GRUULHELPERS_H

#include "Common.h"
#include "Position.h"
#include <type_traits>

class Player;

namespace GruulHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class GruulSpells : uint32
{
    // High King Maulgar
    SPELL_WHIRLWIND     = 33238,

    // Krosh Firehand
    SPELL_SPELL_SHIELD  = 33054,

    // Hunter
    SPELL_MISDIRECTION  = 35079,

    // Mage
    SPELL_SPELLSTEAL    = 30449,

    // Priest
    SPELL_FEAR_WARD     = 6346,

    // Gruul the Dragonkiller
    SPELL_GROUND_SLAM_1 = 33525,
    SPELL_GROUND_SLAM_2 = 39187,
};

enum class GruulNpcs : uint32
{
    NPC_WILD_FEL_STALKER = 18847,
};

inline constexpr uint32 GRUUL_MAP_ID = 565;
inline constexpr float WHIRLWIND_SAFE_DISTANCE = 8.0f;
inline constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;

inline Position const MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
inline Position const OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
inline Position const BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
inline Position const KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
inline Position const GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

bool IsMaulgarTank(Player* bot);
bool IsOlmTank(Player* bot);
bool IsBlindeyeTank(Player* bot);
Player* GetKroshMageTank(Player* bot);
bool IsKroshMageTank(Player* bot);
Player* GetKigglerMoonkinTank(Player* bot);
bool IsKigglerMoonkinTank(Player* bot);
bool HasGroundSlam(Player* bot);

}

#endif
