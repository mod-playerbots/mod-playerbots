#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "Action.h"
#include "AuchenaiCryptsActions.h"

class TbcDungeonAuchenaiCryptsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonAuchenaiCryptsActionContext() : NamedObjectContext<Action>(false, true)
    {
        creators["shirrak tank position boss"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_tank_position_boss;

        creators["shirrak flee focus fire"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_flee_focus_fire;

        creators["shirrak ranged keep distance"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_ranged_keep_distance;
    }
private:

    static Action* shirrak_tank_position_boss(
        PlayerbotAI* botAI) { return new ShirrakTankPositionBossAction(botAI); }

    static Action* shirrak_flee_focus_fire(
        PlayerbotAI* botAI) { return new ShirrakFleeFocusFireAction(botAI); }

    static Action* shirrak_ranged_keep_distance(
        PlayerbotAI* botAI) { return new ShirrakRangedKeepDistanceAction(botAI); }
};

#endif
