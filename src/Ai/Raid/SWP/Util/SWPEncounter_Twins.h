/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERTWINS_H
#define PLAYERBOTS_SWPENCOUNTERTWINS_H

#include <array>
#include <ctime>
#include <unordered_map>

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"

class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
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

extern const Position SACROLASH_TANK_POSITION;
extern const Position EREDAR_TWINS_P1_RANGED_POSITION;
extern const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION;
extern const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION;

extern std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
	eredarTwinsIncomingConflagrationStates;
extern std::unordered_map<uint32, time_t> eredarTwinsDpsHoldTimer;

Position GetAlythessTankPosition(Unit* alythess, uint8 index);
Position GetEredarTwinsP2MeleeStackPosition(Unit* alythess);
Position GetEredarTwinsP2RangedStackPosition(Unit* alythess);
bool IsAnySacrolashTank(PlayerbotAI* botAI, Player* bot);
bool IsAlythessTank(PlayerbotAI* botAI, Player* bot);
bool ShouldHoldSacrolashThreat(PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash);
bool ShouldHoldAlythessThreat(PlayerbotAI* botAI, Player* bot, Unit* alythess);
bool IsAlythessTankPositionSafe(Player* bot, const Position& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
void RecordEredarTwinsIncomingConflagrationTarget(Player* target, uint32 durationMs = 2000);
Player* GetEredarTwinsConflagrationTarget(Player* bot);

}

#endif
