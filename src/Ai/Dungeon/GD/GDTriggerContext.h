/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GDTRIGGERCONTEXT_H
#define _PLAYERBOT_GDTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "GDTriggers.h"

class WotlkDungeonGDTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonGDTriggerContext()
        {
            creators["poison nova"] = &WotlkDungeonGDTriggerContext::poison_nova;
            creators["snake wrap"] = &WotlkDungeonGDTriggerContext::snake_wrap;
            creators["whirling slash"] = &WotlkDungeonGDTriggerContext::whirling_slash;
        }
    private:
        static Trigger* poison_nova(PlayerbotAI* ai) { return new SladranPoisonNovaTrigger(ai); }
        static Trigger* snake_wrap(PlayerbotAI* ai) { return new SladranSnakeWrapTrigger(ai); }
        static Trigger* whirling_slash(PlayerbotAI* ai) { return new GaldarahWhirlingSlashTrigger(ai); }
};

#endif
