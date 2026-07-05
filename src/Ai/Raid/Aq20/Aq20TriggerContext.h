#ifndef PLAYERBOTS_AQ20TRIGGERCONTEXT_H
#define PLAYERBOTS_AQ20TRIGGERCONTEXT_H

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
