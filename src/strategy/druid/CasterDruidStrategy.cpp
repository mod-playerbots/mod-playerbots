/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CasterDruidStrategy.h"

#include "AiObjectContext.h"
#include "FeralDruidStrategy.h"

class CasterDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterDruidStrategyActionNodeFactory()
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
    }

private:
    static ActionNode* faerie_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* hibernate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("hibernate",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ { new NextAction("entangling roots") },
                              /*C*/ {});
    }

    static ActionNode* entangling_roots([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots on cc",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wrath",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* starfall([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("starfall",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* insect_swarm([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("insect swarm",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* moonfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("moonfire",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* starfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("starfire",
                              /*P*/ { new NextAction("moonkin form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* moonkin_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("moonkin form",
                              /*P*/ { new NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

CasterDruidStrategy::CasterDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CasterDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

std::vector<NextAction*> CasterDruidStrategy::getDefaultActions()
{
    return {
        new NextAction("starfall", ACTION_HIGH + 1.0f),
        new NextAction("force of nature", ACTION_DEFAULT + 1.0f),
        new NextAction("wrath", ACTION_DEFAULT + 0.1f),
    };
}

void CasterDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("eclipse (lunar) cooldown",
                                       { new NextAction("starfire", ACTION_DEFAULT + 0.2f) }));
    triggers.push_back(new TriggerNode("eclipse (solar) cooldown",
                                       { new NextAction("wrath", ACTION_DEFAULT + 0.2f) }));

    triggers.push_back(new TriggerNode(
        "insect swarm", { new NextAction("insect swarm", ACTION_NORMAL + 5) }));
    triggers.push_back(
        new TriggerNode("moonfire", { new NextAction("moonfire", ACTION_NORMAL + 4) }));
    triggers.push_back(
        new TriggerNode("eclipse (solar)", { new NextAction("wrath", ACTION_NORMAL + 6) }));
    triggers.push_back(new TriggerNode("eclipse (lunar)",
                                       { new NextAction("starfire", ACTION_NORMAL + 6) }));
    triggers.push_back(
        new TriggerNode("medium mana", { new NextAction("innervate", ACTION_HIGH + 9) }));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       { new NextAction("flee", ACTION_MOVE + 9) }));
}

void CasterDruidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("hurricane channel check", { new NextAction("cancel channel", ACTION_HIGH + 2) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { new NextAction("hurricane", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "light aoe", { new NextAction("insect swarm on attacker", ACTION_NORMAL + 3),
                                       new NextAction("moonfire on attacker", ACTION_NORMAL + 3)   }));
}

void CasterDruidDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("faerie fire", { new NextAction("faerie fire", ACTION_HIGH) }));
}
