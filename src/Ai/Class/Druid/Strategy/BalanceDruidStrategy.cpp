/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BalanceDruidStrategy.h"

#include "AiObjectContext.h"
#include "FeralDruidStrategy.h"

class BalanceDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BalanceDruidStrategyActionNodeFactory()
    {
        creators["faerie fire"] = &faerie_fire;
        creators["hibernate"] = &hibernate;
        creators["entangling roots"] = &entangling_roots;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["wrath"] = &wrath;
        creators["starfall"] = &starfall;
        creators["insect swarm"] = &insect_swarm;
        creators["moonfire"] = &moonfire;
        creators["starfire"] = &starfire;
        creators["moonkin form"] = &moonkin_form;
        creators["typhoon"] = &typhoon;
        creators["hurricane"] = &hurricane;
        creators["force of nature"] = &force_of_nature;
        creators["cyclone on cc"] = &cyclone_on_cc;
    }

private:
    static ActionNode* faerie_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "faerie fire",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* hibernate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "hibernate",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ { NextAction("entangling roots") },
            /*C*/ {}
        );
    }

    static ActionNode* entangling_roots([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "entangling roots",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "entangling roots on cc",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "wrath",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* starfall([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "starfall",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* insect_swarm([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "insect swarm",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* moonfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "moonfire",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* starfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "starfire",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* moonkin_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "moonkin form",
            /*P*/ { NextAction("caster form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* typhoon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "typhoon",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* hurricane([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "hurricane",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* force_of_nature([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "force of nature",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* cyclone_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "cyclone on cc",
            /*P*/ { NextAction("moonkin form") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

BalanceDruidStrategy::BalanceDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new BalanceDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

std::vector<NextAction> BalanceDruidStrategy::getDefaultActions()
{
    return {
        NextAction("starfire", 5.4f),
        NextAction("wrath", 5.3f),
    };
}

void BalanceDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    // Debuffs and DoTs
    triggers.push_back(new TriggerNode("faerie fire", { NextAction("faerie fire", 29.5f) }));
    triggers.push_back(new TriggerNode("insect swarm", { NextAction("insect swarm", 18.0f) }));
    triggers.push_back(new TriggerNode("moonfire", { NextAction("moonfire", 17.5f) }));

    // Eclipse procs
    triggers.push_back(new TriggerNode("eclipse (solar)", { NextAction("wrath", 20.0f) }));
    triggers.push_back(new TriggerNode("eclipse (lunar)", { NextAction("starfire", 20.0f) }));

    // Utility/Defensive
    triggers.push_back(new TriggerNode("medium mana", { NextAction("innervate", 29.0f) }));
    triggers.push_back(new TriggerNode("enemy too close for spell", { NextAction("flee", 39.0f) }));
}
