/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BloodDKStrategy.h"

#include "Playerbots.h"

class BloodDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BloodDKStrategyActionNodeFactory()
    {
        creators["rune strike"] = &rune_strike;
        creators["heart strike"] = &heart_strike;
        creators["death strike"] = &death_strike;
        creators["icy touch"] = &icy_touch;
        creators["dark command"] = &dark_command;
        creators["taunt spell"] = &dark_command;
    }

private:
    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rune strike",
            {
                NextAction("frost presence")
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "icy touch",
            {
                NextAction("frost presence")
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "heart strike",
            {
                NextAction("frost presence")
            },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "death strike",
            {
                NextAction("frost presence")
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* dark_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "dark command",
            {
                NextAction("frost presence")
            },
            /*A*/ {
                NextAction("death grip")
            },
            /*C*/ {}
        );
    }
};

BloodDKStrategy::BloodDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new BloodDKStrategyActionNodeFactory());
}

std::vector<NextAction> BloodDKStrategy::getDefaultActions()
{
    return {
        NextAction("rune strike", ACTION_DEFAULT + 0.8f),
        NextAction("icy touch", ACTION_DEFAULT + 0.7f),
        NextAction("heart strike", ACTION_DEFAULT + 0.6f),
        NextAction("blood strike", ACTION_DEFAULT + 0.5f),
        NextAction("dancing rune weapon", ACTION_DEFAULT + 0.4f),
        NextAction("death coil", ACTION_DEFAULT + 0.3f),
        NextAction("plague strike", ACTION_DEFAULT + 0.2f),
        NextAction("horn of winter", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void BloodDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "rune strike",
            {
                NextAction("rune strike", ACTION_NORMAL + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "blood tap",
            {
                NextAction("blood tap", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                NextAction("dark command", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("army of the dead", ACTION_HIGH + 4),
                NextAction("death strike", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                NextAction("vampiric blood", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "icy touch",
            {
                NextAction("icy touch", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "plague strike",
            {
                NextAction("plague strike", ACTION_HIGH + 2)
            }
        )
    );
}
