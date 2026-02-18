#pragma once

#include "AiObjectContext.h"
#include "Trash/GurubashiBatRider/GurubashiBatRiderUnstableConcoctionTrigger.h"

class RaidZGTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZGTriggerContext()
    {
        // Trash
        creators["gurubashi bat rider unstable concoction"] = &RaidZGTriggerContext::gurubashiBatRiderUnstableConcoction;
    }

private:
    // Trash
    static Trigger* gurubashiBatRiderUnstableConcoction(PlayerbotAI* botAI)
    {
        return new GurubashiBatRiderUnstableConcoctionTrigger(botAI);
    }
};
