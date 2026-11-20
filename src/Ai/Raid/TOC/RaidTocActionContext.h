/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCACTIONCONTEXT_H
#define PLAYERBOTS_RAIDTOCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidTocActions.h"

class RaidTocActionContext : public NamedObjectContext<Action>
{
public:
    RaidTocActionContext()
    {
        creators["toc twin valkyr touch"] = &RaidTocActionContext::toc_twin_valkyr_touch;
        creators["toc offheal"] = &RaidTocActionContext::toc_offheal;
        creators["toc icehowl dodge"] = &RaidTocActionContext::toc_icehowl_dodge;
        creators["toc anub submerge move"] = &RaidTocActionContext::toc_anub_submerge_move;
        creators["toc jaraxxus adds"] = &RaidTocActionContext::toc_jaraxxus_adds;
        creators["toc burning bile"] = &RaidTocActionContext::toc_burning_bile;
    }

private:
    static Action* toc_twin_valkyr_touch(PlayerbotAI* botAI) { return new TocTwinValkyrTouchAction(botAI); }
    static Action* toc_offheal(PlayerbotAI* botAI) { return new TocOffhealAction(botAI); }
    static Action* toc_icehowl_dodge(PlayerbotAI* botAI) { return new TocIcehowlDodgeAction(botAI); }
    static Action* toc_anub_submerge_move(PlayerbotAI* botAI) { return new TocAnubSubmergeAction(botAI); }
    static Action* toc_jaraxxus_adds(PlayerbotAI* botAI) { return new TocJaraxxusAddsAction(botAI); }
    static Action* toc_burning_bile(PlayerbotAI* botAI) { return new TocBurningBileAction(botAI); }
};

#endif
