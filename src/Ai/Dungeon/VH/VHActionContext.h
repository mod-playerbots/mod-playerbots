/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VHACTIONCONTEXT_H
#define _PLAYERBOT_VHACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "VHActions.h"

class WotlkDungeonVHActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonVHActionContext() {
            creators["attack erekem"] = &WotlkDungeonVHActionContext::attack_erekem;
            creators["attack ichor globule"] = &WotlkDungeonVHActionContext::attack_ichor_globule;
            creators["attack void sentry"] = &WotlkDungeonVHActionContext::attack_void_sentry;
            creators["stop attack"] = &WotlkDungeonVHActionContext::stop_attack;
        }
    private:
        static Action* attack_erekem(PlayerbotAI* ai) { return new AttackErekemAction(ai); }
        static Action* attack_ichor_globule(PlayerbotAI* ai) { return new AttackIchorGlobuleAction(ai); }
        static Action* attack_void_sentry(PlayerbotAI* ai) { return new AttackVoidSentryAction(ai); }
        static Action* stop_attack(PlayerbotAI* ai) { return new StopAttackAction(ai); }
};

#endif
