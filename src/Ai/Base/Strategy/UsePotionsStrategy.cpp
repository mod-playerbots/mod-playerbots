/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UsePotionsStrategy.h"

class UsePotionsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UsePotionsStrategyActionNodeFactory() { creators["healthstone"] = &healthstone; }

private:
    static ActionNode* healthstone(PlayerbotAI* botAI)
    {
        return new ActionNode("healthstone",
                              /*P*/ {},
                              /*A*/ { NextAction("healing potion") },
                              /*C*/ {});
    }
};

UsePotionsStrategy::UsePotionsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new UsePotionsStrategyActionNodeFactory());
}

void UsePotionsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health", { NextAction("healthstone", ACTION_MEDIUM_HEAL + 1) }));
    triggers.push_back(
        new TriggerNode("low mana", { NextAction("mana potion", ACTION_EMERGENCY) }));
}
