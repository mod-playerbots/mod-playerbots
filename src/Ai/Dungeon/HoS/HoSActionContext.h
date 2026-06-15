#ifndef PLAYERBOTS_HOSACTIONCONTEXT_H
#define PLAYERBOTS_HOSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "HoSActions.h"

class WotlkDungeonHoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonHoSActionContext() {
            creators["shatter spread"] = &WotlkDungeonHoSActionContext::shatter_spread;
            creators["avoid lightning ring"] = &WotlkDungeonHoSActionContext::avoid_lightning_ring;
        }
    private:
        static Action* shatter_spread(PlayerbotAI* ai) { return new ShatterSpreadAction(ai); }
        static Action* avoid_lightning_ring(PlayerbotAI* ai) { return new AvoidLightningRingAction(ai); }
};

#endif
