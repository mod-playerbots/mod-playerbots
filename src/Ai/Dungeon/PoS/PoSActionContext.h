/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSACTIONCONTEXT_H
#define _PLAYERBOT_POSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "PoSActions.h"

class WotlkDungeonPoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonPoSActionContext()
        {
            creators["ick and krick"] = &WotlkDungeonPoSActionContext::ick_and_krick;
            creators["tyrannus"] = &WotlkDungeonPoSActionContext::tyrannus;
        }
    private:
        static Action* ick_and_krick(PlayerbotAI* ai) { return new IckAndKrickAction(ai); }
        static Action* tyrannus(PlayerbotAI* ai) { return new TyrannusAction(ai); }
};

#endif
