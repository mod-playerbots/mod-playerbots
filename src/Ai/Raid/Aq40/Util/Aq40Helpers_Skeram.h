#ifndef _PLAYERBOT_RAIDAQ40HELPERS_SKERAM_H_
#define _PLAYERBOT_RAIDAQ40HELPERS_SKERAM_H_

#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"

namespace Aq40Helpers
{
bool IsSkeramEncounterLive(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers);
GuidVector GetObservedSkeramEncounterUnits(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers);
bool IsSkeramPostBlinkHoldActive(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers);

bool ResetSkeramEncounterState(Player* bot);
bool HasSkeramEncounterState(Player* bot);
void ClearSkeramPostBlinkHold(Player* bot);
}    // namespace Aq40Helpers

#endif
