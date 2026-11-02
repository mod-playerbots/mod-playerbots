/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ONYHELPERS_H
#define PLAYERBOTS_ONYHELPERS_H

#include "Common.h"
#include <type_traits>

class Player;
class Unit;

namespace OnyxiaHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class OnyNpcs : uint32
{
    NPC_ONYXIA      = 10184,
    NPC_ONYXIA_40   = 301000,
};

inline constexpr uint32 ONYXIAS_LAIR_MAP_ID = 249;
inline constexpr float ONYXIA_PHASE_THREE_HEALTH_PCT = 40.0f;

// The main tank must lead a bot by this much before it may start an attack on her; a running auto-attack
// stops below half of it
inline constexpr float THREAT_LEAD = 4000.0f;
inline constexpr float THREAT_LEAD_KEEP = THREAT_LEAD / 2;
// Longest hold once she is on the ground, so a main tank who never gets to her cannot hold everyone
inline constexpr uint32 HOLD_LIMIT_MS = 10 * IN_MILLISECONDS;

bool IsOnyxia(Unit* unit);
bool MustHoldPhaseThreeThreat(Player* bot, Unit* onyxia, float requiredLead);

}

#endif
