/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HOSTRIGGERCONTEXT_H
#define _PLAYERBOT_HOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "HoSTriggers.h"

class WotlkDungeonHoSTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonHoSTriggerContext()
        {
            creators["ground slam"] = &WotlkDungeonHoSTriggerContext::ground_slam;
            creators["lightning ring"] = &WotlkDungeonHoSTriggerContext::lightning_ring;
        }
    private:
        static Trigger* ground_slam(PlayerbotAI* ai) { return new KrystallusGroundSlamTrigger(ai); }
        static Trigger* lightning_ring(PlayerbotAI* ai) { return new SjonnirLightningRingTrigger(ai); }
};

#endif
