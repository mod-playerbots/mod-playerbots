/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPSHARED_H
#define PLAYERBOTS_RAMPSHARED_H

#include "Common.h"
#include "Position.h"
#include <type_traits>

using namespace std;

namespace RampShared
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class RampSpells : uint32
{
    SPELL_TREMOR_TOTEM                  = 8143,
    SPELL_FIRE_RESISTANCE_TOTEM_RANK_1  = 8184,
    SPELL_TREACHEROUS_AURA              = 30695,
    SPELL_BANE_OF_TREACHERY             = 37566,
};

inline constexpr uint32 RAMP_MAP_ID = 543;

inline Position const VAZRUDEN_TANK_POSITION   = { -1407.405f, 1744.521f, 81.075f };
inline Position const OMOR_TANK_POSITION = { -1132.252f, 1710.033f, 89.914f };

}

#endif
