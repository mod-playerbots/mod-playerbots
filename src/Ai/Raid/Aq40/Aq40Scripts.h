#ifndef _PLAYERBOT_RAIDAQ40SCRIPTS_H_
#define _PLAYERBOT_RAIDAQ40SCRIPTS_H_

#include "ObjectGuid.h"
#include "Position.h"

class Map;
class Player;

namespace Aq40Scripts
{
bool IsTwinTeleportPickupWindow(Player const* bot, uint32 windowMs = 8000, uint32 nowMs = 0);
bool IsTwinBlizzardWindow(Player const* bot, uint32 windowMs = 5000, uint32 nowMs = 0);
bool IsTwinArcaneBurstWindow(Player const* bot, uint32 windowMs = 2500, uint32 nowMs = 0);
bool IsTwinExplodeBugWindow(Player const* bot, uint32 windowMs = 2500, uint32 nowMs = 0);
bool GetTwinExplodeBugSource(Player const* bot, ObjectGuid& sourceGuid, Position& sourcePosition,
                             uint32 windowMs = 2500, uint32 nowMs = 0);
bool HasPersistentTwinState(Player const* bot);
bool ResetTwinState(Player const* bot);
void ResetInstance(uint32 instanceId, Map* map);
}    // namespace Aq40Scripts

void AddSC_Aq40BotScripts();

#endif
