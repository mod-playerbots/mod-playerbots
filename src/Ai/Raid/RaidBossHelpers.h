#ifndef _PLAYERBOT_RAIDBOSSHELPERS_H_
#define _PLAYERBOT_RAIDBOSSHELPERS_H_

#include "AiObject.h"
#include "Unit.h"

void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
void MarkTargetWithSkull(Player* bot, Unit* target);
void MarkTargetWithSquare(Player* bot, Unit* target);
void MarkTargetWithStar(Player* bot, Unit* target);
void MarkTargetWithCircle(Player* bot, Unit* target);
void MarkTargetWithDiamond(Player* bot, Unit* target);
void MarkTargetWithTriangle(Player* bot, Unit* target);
void MarkTargetWithCross(Player* bot, Unit* target);
void MarkTargetWithMoon(Player* bot, Unit* target);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
bool IsMechanicTrackerBot(PlayerbotAI* botAI, Player* bot, uint32 mapId, Player* exclude = nullptr);
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry, bool requireInCombat = false);
Unit* GetNearestPlayerInRadius(Player* bot, float radius);

#endif
