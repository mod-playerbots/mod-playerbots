/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_EOETRIGGERCONTEXT_H
#define _PLAYERBOT_EOETRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "EoETriggers.h"

class RaidEoETriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidEoETriggerContext()
    {
        creators["malygos"] = &RaidEoETriggerContext::malygos;
        creators["power spark"] = &RaidEoETriggerContext::power_spark;
    }

private:
    static Trigger* power_spark(PlayerbotAI* ai) { return new PowerSparkTrigger(ai); }
    static Trigger* malygos(PlayerbotAI* ai) { return new MalygosTrigger(ai); }
};

#endif
