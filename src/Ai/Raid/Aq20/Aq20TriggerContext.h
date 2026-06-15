/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AQ20TRIGGERCONTEXT_H
#define _PLAYERBOT_AQ20TRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "Aq20Triggers.h"

class RaidAq20TriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidAq20TriggerContext()
    {
        creators["aq20 move to crystal"] = &RaidAq20TriggerContext::move_to_crystal;
    }

private:
    static Trigger* move_to_crystal(PlayerbotAI* ai) { return new Aq20MoveToCrystalTrigger(ai); }
};

#endif
