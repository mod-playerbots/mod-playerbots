/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FERALRUIDSTRATEGY_H
#define _PLAYERBOT_FERALRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"

class PlayerbotAI;

class ShapeshiftDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShapeshiftDruidStrategyActionNodeFactory()
    {
        creators["rejuvenation"] = &rejuvenation;
        creators["regrowth"] = &regrowth;
        creators["healing touch"] = &healing_touch;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
        creators["regrowth on party"] = &regrowth_on_party;
        creators["healing touch on party"] = &healing_touch_on_party;
    }

private:
    static ActionNode* regrowth([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ { NextAction("healing touch") },
                              /*C*/ { NextAction("melee", 10.0f) });
    }

    static ActionNode* rejuvenation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* healing_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ { NextAction("healing touch on party") },
                              /*C*/ { NextAction("melee", 10.0f) });
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

class FeralDruidStrategy : public GenericDruidStrategy
{
protected:
    FeralDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
};

#endif
