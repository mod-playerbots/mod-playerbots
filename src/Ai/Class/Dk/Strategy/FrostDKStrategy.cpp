/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostDKStrategy.h"

#include "Playerbots.h"

class FrostDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostDKStrategyActionNodeFactory()
    {
        creators["icy touch"] = &icy_touch;
        creators["obliterate"] = &obliterate;
        creators["howling blast"] = &howling_blast;
        creators["frost strike"] = &frost_strike;
        creators["rune strike"] = &rune_strike;
        creators["unbreakable armor"] = &unbreakable_armor;
    }

private:
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "icy touch",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* obliterate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "obliterate",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rune strike",
            /*P*/ { NextAction("blood presence") },
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }

    static ActionNode* frost_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "frost strike",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* howling_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "howling blast",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* unbreakable_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "unbreakable armor",
            /*P*/ { NextAction("blood tap") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

FrostDKStrategy::FrostDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new FrostDKStrategyActionNodeFactory());
}

std::vector<NextAction> FrostDKStrategy::getDefaultActions()
{
    return {
        NextAction("obliterate", ACTION_DEFAULT + 0.7f),
        NextAction("frost strike", ACTION_DEFAULT + 0.4f),
        NextAction("empower rune weapon", ACTION_DEFAULT + 0.3f),
        NextAction("horn of winter", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void FrostDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "unbreakable armor",
            {
                NextAction("unbreakable armor", ACTION_DEFAULT + 0.6f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "freezing fog",
            {
                NextAction("howling blast", ACTION_DEFAULT + 0.5f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high blood rune",
            {
                NextAction("blood strike", ACTION_DEFAULT + 0.2f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "army of the dead",
            {
                NextAction("army of the dead", ACTION_HIGH + 6)
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

void FrostDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("howling blast", ACTION_HIGH + 4)
            }
        )
    );
}
