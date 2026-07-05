#ifndef PLAYERBOTS_EOETRIGGERCONTEXT_H
#define PLAYERBOTS_EOETRIGGERCONTEXT_H

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
