#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H

#include "AIObjectContext.h"
#include "TriggerContext.h"
#include "AuchenaiCryptsTriggers.h"

class TbcDungeonAuchenaiCryptsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    // Shirrak the Dead Watcher
    TbcDungeonAuchenaiCryptsTriggerContext()
    {
        creators["shirrak tank position boss"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_tank_position_boss;

        creators["flee focus fire"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::flee_focus_fire;
    }
private:
    // Shirrak the Dead Watcher
    static Trigger* shirrak_tank_position_boss(
        PlayerbotAI* botAI) { return new ShirrakTankPositionBossTrigger(botAI); }

    static Trigger* flee_focus_fire(
        PlayerbotAI* botAI) { return new FleeFocusFireTrigger(botAI); }
};

#endif
