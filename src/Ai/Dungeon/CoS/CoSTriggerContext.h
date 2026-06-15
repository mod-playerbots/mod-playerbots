/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COSTRIGGERCONTEXT_H
#define _PLAYERBOT_COSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "CoSTriggers.h"

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
