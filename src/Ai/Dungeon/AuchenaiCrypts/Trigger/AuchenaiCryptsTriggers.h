#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERS_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum AuchenaiCryptsIDs
{
    // Shirrak The Dead Watcher
    NPC_FOCUS_FIRE                  = 18374,
    SPELL_FOCUS_CAST                = 32300,
    SPELL_FIERY_BLAST               = 32302,
    SPELL_FOCUS_FIRE_VISUAL         = 32286,
};
 
class ShirrakTankPositionBossTrigger : public Trigger
{
public:
    ShirrakTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shirrak tank position") {}

    bool IsActive() override;

};

class FleeFocusFireTrigger : public Trigger
{
public:
    FleeFocusFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flee focus fire") {}

    bool IsActive() override;
};

#endif
