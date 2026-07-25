/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANTRIGGERCONTEXT_H
#define PLAYERBOTS_ANTRIGGERCONTEXT_H

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
