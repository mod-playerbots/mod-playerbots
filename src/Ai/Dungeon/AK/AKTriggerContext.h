/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AKTRIGGERCONTEXT_H
#define _PLAYERBOT_AKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "AKTriggers.h"

class WotlkDungeonOKTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonOKTriggerContext()
        {
            creators["nadox guardian"] = &WotlkDungeonOKTriggerContext::nadox_guardian;
            creators["jedoga volunteer"] = &WotlkDungeonOKTriggerContext::jedoga_volunteer;
            creators["shadow crash"] = &WotlkDungeonOKTriggerContext::shadow_crash;
        }
    private:
        static Trigger* nadox_guardian(PlayerbotAI* ai) { return new NadoxGuardianTrigger(ai); }
        static Trigger* jedoga_volunteer(PlayerbotAI* ai) { return new JedogaVolunteerTrigger(ai); }
        static Trigger* shadow_crash(PlayerbotAI* ai) { return new ShadowCrashTrigger(ai); }
};

#endif
