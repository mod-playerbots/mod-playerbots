/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_STAGING_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_STAGING_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

// No encounter prefix, so the gate never wraps it: the window is its own gate
class ToCStagingStandTrigger : public Trigger
{
public:
    ToCStagingStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc staging stand") {}
    bool IsActive() override;
};

class ToCStagingTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCStagingTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["toc staging stand"] = &ToCStagingTriggerContext::toc_staging_stand;
    }

private:
    static Trigger* toc_staging_stand(PlayerbotAI* botAI) { return new ToCStagingStandTrigger(botAI); }
};

void AddToCStagingTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
