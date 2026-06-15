/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ANTRIGGERCONTEXT_H
#define _PLAYERBOT_ANTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "ANTriggers.h"

class WotlkDungeonANTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonANTriggerContext()
        {
            creators["krik'thir web wrap"] = &WotlkDungeonANTriggerContext::krikthir_web_wrap;
            creators["krik'thir watchers"] = &WotlkDungeonANTriggerContext::krikthir_watchers;
            // creators["anub'arak impale"] = &WotlkDungeonANTriggerContext::anubarak_impale;
            creators["anub'arak pound"] = &WotlkDungeonANTriggerContext::anubarak_pound;
        }
    private:
        static Trigger* krikthir_web_wrap(PlayerbotAI* ai) { return new KrikthirWebWrapTrigger(ai); }
        static Trigger* krikthir_watchers(PlayerbotAI* ai) { return new KrikthirWatchersTrigger(ai); }
        // static Trigger* anubarak_impale(PlayerbotAI* ai) { return new AnubarakImpaleTrigger(ai); }
        static Trigger* anubarak_pound(PlayerbotAI* ai) { return new AnubarakPoundTrigger(ai); }
};

#endif
