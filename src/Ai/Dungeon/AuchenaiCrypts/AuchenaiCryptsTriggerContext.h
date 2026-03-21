#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSTRIGGERCONTEXT_H

#include "AIObjectContext.h"
#include "TriggerContext.h"
#include "AuchenaiCryptsTriggers.h"

class TbcDungeonAuchenaiCryptsTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        TbcDungeonAuchenaiCryptsTriggerContext()
        {
            creators["shirrak tank position"] = 
                &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_tank_position;

            creators["flee focus fire"] = 
                &TbcDungeonAuchenaiCryptsTriggerContext::flee_focus_fire;
        }
    private:
        static Trigger* shirrak_tank_position(
            PlayerbotAI* botAI) { return new ShirrakTankPositionBossTrigger(botAI); }
            
        static Trigger* flee_focus_fire(
            PlayerbotAI* botAI) { return new FleeFocusFireTrigger(botAI); }
};

#endif
