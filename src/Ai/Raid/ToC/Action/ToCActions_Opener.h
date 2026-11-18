/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_OPENER_H
#define PLAYERBOTS_RAID_TOCACTIONS_OPENER_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidOpener.h"
#include "ToCHelpers_Opener.h"

class ToCOpenerActionContext : public NamedObjectContext<Action>
{
public:
    ToCOpenerActionContext()
    {
        creators["toc opener stop swing"] = &ToCOpenerActionContext::toc_opener_stop_swing;
        creators["toc opener redirect"] = &ToCOpenerActionContext::toc_opener_redirect;
    }

private:
    static Action* toc_opener_stop_swing(PlayerbotAI* botAI)
    {
        return new RaidOpenerStopSwingAction(botAI, "toc opener stop swing", TrialOfTheCrusaderHelpers::TOC_OPENER);
    }
    static Action* toc_opener_redirect(PlayerbotAI* botAI)
    {
        return new RaidOpenerRedirectAction(botAI, "toc opener redirect", TrialOfTheCrusaderHelpers::TOC_OPENER);
    }
};

#endif
