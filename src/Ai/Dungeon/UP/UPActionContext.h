/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UPACTIONCONTEXT_H
#define _PLAYERBOT_UPACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "UPActions.h"

class WotlkDungeonUPActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonUPActionContext() {
            creators["avoid freezing cloud"] = &WotlkDungeonUPActionContext::avoid_freezing_cloud;
            creators["avoid skadi whirlwind"] = &WotlkDungeonUPActionContext::avoid_whirlwind;
            creators["stop attack"] = &WotlkDungeonUPActionContext::stop_attack;
        }
    private:
        static Action* avoid_freezing_cloud(PlayerbotAI* ai) { return new AvoidFreezingCloudAction(ai); }
        static Action* avoid_whirlwind(PlayerbotAI* ai) { return new AvoidSkadiWhirlwindAction(ai); }
        static Action* stop_attack(PlayerbotAI* ai) { return new DropTargetAction(ai); }
};

#endif
