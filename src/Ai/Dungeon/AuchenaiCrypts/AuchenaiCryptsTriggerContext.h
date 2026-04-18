#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
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

        creators["shirrak flee focus fire"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_flee_focus_fire;

        creators["shirrak ranged keep distance"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_ranged_keep_distance;
    }
private:
    // Shirrak the Dead Watcher
    static Trigger* shirrak_tank_position_boss(
        PlayerbotAI* botAI) { return new ShirrakTankPositionBossTrigger(botAI); }

    static Trigger* shirrak_flee_focus_fire(
        PlayerbotAI* botAI) { return new ShirrakFleeFocusFireTrigger(botAI); }

    static Trigger* shirrak_ranged_keep_distance(
        PlayerbotAI* botAI) { return new ShirrakRangedKeepDistanceTrigger(botAI); }
};

#endif
