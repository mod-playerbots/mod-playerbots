/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H
#define PLAYERBOTS_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class ShadowPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShadowPriestStrategyActionNodeFactory()
    {
        creators["mind blast"] = &mind_blast;
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

};

#endif
