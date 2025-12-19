/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UnholyDKStrategy.h"

#include "Playerbots.h"

class UnholyDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UnholyDKStrategyActionNodeFactory()
    {
        creators["death strike"] = &death_strike;
        creators["scourge strike"] = &scourge_strike;
        creators["ghoul frenzy"] = &ghoul_frenzy;
        creators["corpse explosion"] = &corpse_explosion;
        creators["icy touch"] = &icy_touch;
    }

private:
    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "death strike",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* ghoul_frenzy([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "ghoul frenzy",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "corpse explosion",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* scourge_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "scourge strike",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "icy touch",
            /*P*/ { NextAction("blood presence") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

UnholyDKStrategy::UnholyDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new UnholyDKStrategyActionNodeFactory());
}

std::vector<NextAction> UnholyDKStrategy::getDefaultActions()
{
    return {
        NextAction("death and decay", ACTION_HIGH + 5),
        NextAction("summon gargoyle", ACTION_DEFAULT + 0.4f),
        NextAction("horn of winter", ACTION_DEFAULT + 0.2f),
        NextAction("death coil", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void UnholyDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "death and decay cooldown",
            {
                NextAction("ghoul frenzy", ACTION_DEFAULT + 0.9f),
                NextAction("scourge strike", ACTION_DEFAULT + 0.8f),
                NextAction("icy touch", ACTION_DEFAULT + 0.7f),
                NextAction("blood strike", ACTION_DEFAULT + 0.6f),
                NextAction("plague strike", ACTION_DEFAULT + 0.5f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "dd cd and no desolation",
            {
                NextAction("blood strike", ACTION_DEFAULT + 0.75f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high frost rune",
            {
                NextAction("icy touch", ACTION_NORMAL + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high blood rune",
            {
                NextAction("blood strike", ACTION_NORMAL + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high unholy rune",
            {
                NextAction("plague strike", ACTION_NORMAL + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode("dd cd and plague strike 3s",
            {
                NextAction("plague strike", ACTION_HIGH + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode("dd cd and icy touch 3s",
            {
                NextAction("icy touch", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode("no rune",
            {
                NextAction("empower rune weapon", ACTION_HIGH + 1)
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
        new TriggerNode("bone shield",
            {
                NextAction("bone shield", ACTION_HIGH + 3)
            }
        )
    );
}

void UnholyDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "loot available",
            {
                NextAction("corpse explosion", ACTION_NORMAL + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("death and decay", ACTION_NORMAL + 3),
                NextAction("corpse explosion", ACTION_NORMAL + 3)
            }
        )
    );
}
