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

constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;
constexpr uint8 ALYTHESS_TANK_POSITION_COUNT = 5;

struct EredarTwinsIncomingConflagrationState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
};

constexpr uint32 EREDAR_TWINS_INCOMING_CONFLAGRATION_DELAY_MS = 500;

extern Position const SACROLASH_TANK_POSITION;
extern Position const EREDAR_TWINS_P1_RANGED_POSITION;
extern Position const EREDAR_TWINS_RANGED_CONFLAG_POSITION;
extern Position const EREDAR_TWINS_MELEE_CONFLAG_POSITION;

extern std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
	eredarTwinsIncomingConflagrationStates;
extern std::unordered_map<uint32, time_t> eredarTwinsDpsHoldTimer;

Position GetAlythessTankPosition(Unit* alythess, uint8 index);
Position GetEredarTwinsP2MeleeStackPosition(Unit* alythess);
Position GetEredarTwinsP2RangedStackPosition(Unit* alythess);
bool IsAnySacrolashTank(Player* bot);
bool IsAlythessTank(Player* bot);
bool ShouldHoldTwinThreat(
    Player* bot, Unit* boss, float threatHoldRatio, bool (*isTwinTank)(Player*));
bool IsAlythessTankPositionSafe(Player* bot, Position const& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
void RecordEredarTwinsIncomingConflagrationTarget(Player* target, uint32 durationMs = 2000);
Player* GetEredarTwinsConflagrationTarget(Player* bot);

}

#endif
