#ifndef PLAYERBOTS_COSTRIGGERCONTEXT_H
#define PLAYERBOTS_COSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "CoSTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonCoSTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonCoSTriggerContext()
        {
            creators["explode ghoul"] = &WotlkDungeonCoSTriggerContext::explode_ghoul;
            creators["epoch ranged"] = &WotlkDungeonCoSTriggerContext::epoch_ranged;

        }
    private:
        static Trigger* explode_ghoul(PlayerbotAI* ai) { return new ExplodeGhoulTrigger(ai); }
        static Trigger* epoch_ranged(PlayerbotAI* ai) { return new EpochRangedTrigger(ai); }
};

#endif
