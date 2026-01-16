/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class ShadowPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShadowPriestStrategyActionNodeFactory()
    {
        creators["mind blast"] = &mind_blast;
        creators["dispersion"] = &dispersion;
        creators["mind flay"] = &mind_flay;
        creators["smite"] = &smite;
    }

private:
    static ActionNode* mind_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mind blast",
                              /*P*/ {},
                              /*A*/ { NextAction("mind flay") },
                              /*C*/ {});
    }

    static ActionNode* mind_flay([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mind flay",
                              /*P*/ {},
                              /*A*/ { NextAction("smite") },
                              /*C*/ {});
    }

    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("smite",
                              /*P*/ {},
                              /*A*/ { NextAction("shoot") },
                              /*C*/ {});
    }

    static ActionNode* dispersion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("dispersion",
                              /*P*/ {},
                              /*A*/ { NextAction("mana potion") },
                              /*C*/ {});
    }
};

#endif
