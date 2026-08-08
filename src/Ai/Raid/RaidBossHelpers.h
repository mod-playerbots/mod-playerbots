/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDBOSSHELPERS_H
#define PLAYERBOTS_RAIDBOSSHELPERS_H

#include "AiObject.h"
#include "Unit.h"

bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(PlayerbotAI* botAI, Player* bot);
Player* GetGroupAssistTank(PlayerbotAI* botAI, Player* bot, uint8 index);
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);

#endif
