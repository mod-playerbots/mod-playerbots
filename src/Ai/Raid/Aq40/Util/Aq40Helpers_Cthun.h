#ifndef _PLAYERBOT_RAIDAQ40HELPERS_CTHUN_H_
#define _PLAYERBOT_RAIDAQ40HELPERS_CTHUN_H_

#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"

class GameObject;

namespace Aq40Helpers
{
bool IsCthunInStomach(Player* bot, PlayerbotAI* botAI);
uint32 GetCthunPhase2ElapsedMs(PlayerbotAI* botAI, GuidVector const& attackers);
bool IsCthunVulnerableNow(PlayerbotAI* botAI, GuidVector const& attackers);
GameObject* FindLikelyStomachExitPortal(Player* bot, PlayerbotAI* botAI);

bool ResetCthunEncounterState(Player* bot);
bool HasCthunEncounterState(Player* bot);
}    // namespace Aq40Helpers

#endif
