/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UsePotionsStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "UseItemAction.h"
#include "ActionNode.h"

class UsePotionsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UsePotionsStrategyActionNodeFactory() { creators["healthstone"] = &healthstone; }

private:
    static ActionNode* healthstone(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<UseHealingPotion>(1.0f) },
            /*C*/ {}
        );
    }
};

UsePotionsStrategy::UsePotionsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new UsePotionsStrategyActionNodeFactory());
}

void UsePotionsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<UseItemAction>(ACTION_MEDIUM_HEAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<UseManaPotion>(ACTION_EMERGENCY)
            }
        )
    );
}
