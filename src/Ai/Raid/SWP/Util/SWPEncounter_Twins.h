/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERTWINS_H
#define PLAYERBOTS_SWPENCOUNTERTWINS_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <array>
#include <ctime>
#include <unordered_map>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct EredarTwinsIncomingConflagrationState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
};

struct EredarTwinsBlazeTargetState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 expireMs = 0;
};

inline constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;
inline constexpr uint8 ALYTHESS_TANK_POSITION_COUNT = 5;

inline Position const ALYTHESS_START_POSITION = { 1819.180f, 625.539f, 33.4038f };
inline std::array const ALYTHESS_TANK_POSITIONS = {
    Position{ 1816.830f, 620.792f, 33.404f },
    Position{ 1824.211f, 625.169f, 33.404f },
    Position{ 1818.701f, 631.196f, 33.404f },
    Position{ 1829.375f, 631.110f, 33.404f },
    Position{ 1830.007f, 620.924f, 33.404f }
};

// Phase 1 positions
inline Position const SACROLASH_TANK_POSITION  =             { 1804.255f, 630.193f, 33.404f };
inline Position const EREDAR_TWINS_P1_RANGED_POSITION =      { 1808.076f, 603.460f, 51.684f };
inline Position const EREDAR_TWINS_MELEE_CONFLAG_POSITION =  { 1812.842f, 611.147f, 33.404f };
inline Position const EREDAR_TWINS_RANGED_CONFLAG_POSITION = { 1801.133f, 584.456f, 50.696f };

// Phase 2 positions
inline Position const EREDAR_TWINS_P2_MELEE_POSITION =       { 1814.327f, 625.645f, 33.404f };
inline Position const EREDAR_TWINS_P2_RANGED_POSITION =      { 1805.587f, 625.653f, 33.404f };

extern std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
	eredarTwinsIncomingConflagrationStates;
extern std::unordered_map<uint32, EredarTwinsBlazeTargetState> eredarTwinsBlazeTargetStates;
extern std::unordered_map<uint32, time_t> eredarTwinsDpsHoldTimer;

Position GetAlythessTankPosition(Unit* alythess, uint8 index);
Position GetEredarTwinsP2MeleePosition(Unit* alythess);
Position GetEredarTwinsP2RangedPosition(Unit* alythess);
bool IsAnySacrolashTank(Player* bot);
bool IsAlythessTank(Player* bot);
bool ShouldHoldTwinThreat(
    Player* bot, Unit* boss, float threatHoldRatio, bool (*isTwinTank)(Player*));
bool IsAlythessTankPositionSafe(Player* bot, Position const& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
void RecordIncomingEredarTwinsConflagrationTarget(Player* target);
Player* GetEredarTwinsConflagrationTarget(Player* bot);
void RecordEredarTwinsBlazeTarget(Player* target);
Player* GetEredarTwinsBlazeTarget(Player* bot);

}

#endif
