/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENCOUNTERHELPERS_H
#define PLAYERBOTS_ENCOUNTERHELPERS_H

#include "Common.h"
#include "Position.h"
#include <string>
#include <vector>

class Action;
class Player;
class PlayerbotAI;
class Unit;

namespace EncounterHelpers

{

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
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName);
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(Player* bot);
Player* GetGroupAssistTank(Player* bot, uint8 index);
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
bool IsDpsCooldownAction(Player* bot, Action* action);
bool IsTauntAction(Player* bot, Action* action);
bool IsAoeThreatAction(Player* bot, Action* action);

}

#endif
