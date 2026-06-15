/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AQ20ACTIONCONTEXT_H
#define _PLAYERBOT_AQ20ACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "Aq20Actions.h"

class RaidAq20ActionContext : public NamedObjectContext<Action>
{
public:
    RaidAq20ActionContext()
    {
        creators["aq20 use crystal"] = &RaidAq20ActionContext::use_crystal;
    }

private:
    static Action* use_crystal(PlayerbotAI* ai) { return new Aq20UseCrystalAction(ai); }
};

#endif
