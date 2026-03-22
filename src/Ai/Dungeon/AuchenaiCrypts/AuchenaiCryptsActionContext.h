#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONCONTEXT_H

#include "AIObjectContext.h"
#include "Action.h"
#include "AuchenaiCryptsActions.h"

class TbcDungeonAuchenaiCryptsActionContext : public NamedObjectContext<Action>
{
    public:
        TbcDungeonAuchenaiCryptsActionContext() : NamedObjectContext<Action>(false, true)
        {
            creators["shirrak tank position boss"] = 
                &TbcDungeonAuchenaiCryptsActionContext::shirrak_tank_position_boss;
            
            creators["flee focus fire"] = 
                &TbcDungeonAuchenaiCryptsActionContext::flee_focus_fire;
        }
    private:
        
        static Action* shirrak_tank_position_boss(
            PlayerbotAI* botAI) { return new ShirrakTankPositionBossAction(botAI); }
        
        static Action* flee_focus_fire(
            PlayerbotAI* botAI) { return new FleeFocusFireAction(botAI); }
};

#endif
