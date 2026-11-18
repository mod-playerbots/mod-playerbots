/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_OPENER_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_OPENER_H

#include <vector>

#include "NamedObjectContext.h"
#include "RaidOpener.h"
#include "ToCHelpers_Opener.h"
#include "Trigger.h"

// No encounter prefix, the opener carries its own window.
class ToCOpenerTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCOpenerTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["toc opener swing"] = &ToCOpenerTriggerContext::toc_opener_swing;
        creators["toc opener redirect"] = &ToCOpenerTriggerContext::toc_opener_redirect;
    }

private:
    static Trigger* toc_opener_swing(PlayerbotAI* botAI)
    {
        return new RaidOpenerSwingTrigger(botAI, "toc opener swing", TrialOfTheCrusaderHelpers::TOC_OPENER);
    }
    static Trigger* toc_opener_redirect(PlayerbotAI* botAI)
    {
        return new RaidOpenerRedirectTrigger(botAI, "toc opener redirect", TrialOfTheCrusaderHelpers::TOC_OPENER);
    }
};

void AddToCOpenerTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
