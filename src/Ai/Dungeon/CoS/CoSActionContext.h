#ifndef PLAYERBOTS_COSACTIONCONTEXT_H
#define PLAYERBOTS_COSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "CoSActions.h"

class WotlkDungeonCoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonCoSActionContext() {
            creators["explode ghoul spread"] = &WotlkDungeonCoSActionContext::explode_ghoul_spread;
            creators["epoch stack"] = &WotlkDungeonCoSActionContext::epoch_stack;
        }
    private:
        static Action* explode_ghoul_spread(PlayerbotAI* ai) { return new ExplodeGhoulSpreadAction(ai); }
        static Action* epoch_stack(PlayerbotAI* ai) { return new EpochStackAction(ai); }
};

#endif
