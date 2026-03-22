#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERS_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum class AuchenaiCryptsIDs : uint32
{
    // Shirrak The Dead Watcher
    NPC_FOCUS_FIRE                  = 18374,
};
 
class ShirrakTankPositionBossTrigger : public Trigger
{
public:
    ShirrakTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shirrak tank position boss") {}

    bool IsActive() override;
};

class FleeFocusFireTrigger : public Trigger
{
public:
    FleeFocusFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flee focus fire") {}

    bool IsActive() override;
};

#endif
