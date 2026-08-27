/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UPTRIGGERCONTEXT_H
#define PLAYERBOTS_UPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "UPTriggers.h"

class WotlkDungeonUPTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonUPTriggerContext()
        {
            creators["freezing cloud"] = &WotlkDungeonUPTriggerContext::freezing_cloud;
            creators["skadi whirlwind"] = &WotlkDungeonUPTriggerContext::whirlwind;
            creators["ymiron bane"] = &WotlkDungeonUPTriggerContext::bane;
        }
    private:
        static Trigger* freezing_cloud(PlayerbotAI* ai) { return new SkadiFreezingCloudTrigger(ai); }
        static Trigger* whirlwind(PlayerbotAI* ai) { return new SkadiWhirlwindTrigger(ai); }
        static Trigger* bane(PlayerbotAI* ai) { return new YmironBaneTrigger(ai); }
};

#endif
