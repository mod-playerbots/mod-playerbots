/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AKACTIONCONTEXT_H
#define _PLAYERBOT_AKACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "AKActions.h"

class WotlkDungeonOKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonOKActionContext() {
            creators["attack nadox guardian"] = &WotlkDungeonOKActionContext::attack_nadox_guardian;
            creators["attack jedoga volunteer"] = &WotlkDungeonOKActionContext::attack_jedoga_volunteer;
            creators["avoid shadow crash"] = &WotlkDungeonOKActionContext::avoid_shadow_crash;
        }
    private:
        static Action* attack_nadox_guardian(PlayerbotAI* ai) { return new AttackNadoxGuardianAction(ai); }
        static Action* attack_jedoga_volunteer(PlayerbotAI* ai) { return new AttackJedogaVolunteerAction(ai); }
        static Action* avoid_shadow_crash(PlayerbotAI* ai) { return new AvoidShadowCrashAction(ai); }
};

#endif
