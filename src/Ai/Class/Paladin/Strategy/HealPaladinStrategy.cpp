/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HealPaladinStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class HealPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
};

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new HealPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> HealPaladinStrategy::getDefaultActions()
{
    return { NextAction("judgement of light", ACTION_DEFAULT) };
}

void HealPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                NextAction("seal of wisdom", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                NextAction("divine illumination", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("divine favor", ACTION_HIGH + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_EMERGENCY + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                NextAction("divine sacrifice", ACTION_CRITICAL_HEAL + 5),
                NextAction("avenging wrath", ACTION_HIGH + 4),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("holy shock on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("divine sacrifice", ACTION_CRITICAL_HEAL + 5),
                NextAction("holy light on party", ACTION_CRITICAL_HEAL + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("holy light on party", ACTION_LIGHT_HEAL + 9),
                NextAction("flash of light on party", ACTION_LIGHT_HEAL + 8)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
        "party member almost full health",
        {
            NextAction("flash of light on party", ACTION_LIGHT_HEAL + 3)
        }
    )
);

    triggers.push_back(
        new TriggerNode(
        "beacon of light on main tank",
        {
            NextAction("beacon of light on main tank", ACTION_CRITICAL_HEAL + 7)
        }
    )
);

    triggers.push_back(
        new TriggerNode(
        "sacred shield on main tank",
        {
            NextAction("sacred shield on main tank", ACTION_CRITICAL_HEAL + 6)
        }
    )
);
}
