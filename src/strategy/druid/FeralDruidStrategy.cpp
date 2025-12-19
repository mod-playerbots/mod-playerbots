/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FeralDruidStrategy.h"

#include "Playerbots.h"

class FeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["thorns"] = &thorns;
        creators["omen of clarity"] = &omen_of_clarity;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["prowl"] = &prowl;
    }

private:
    static ActionNode* survival_instincts([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("survival instincts",
                              /*P*/ {},
                              /*A*/ { NextAction("barkskin") },
                              /*C*/ {});
    }

    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* omen_of_clarity([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("omen of clarity",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* prowl([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("prowl",
                              /*P*/ { NextAction("cat form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

FeralDruidStrategy::FeralDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new FeralDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

void FeralDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee", { NextAction("reach melee", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "critical health", { NextAction("survival instincts", ACTION_EMERGENCY + 1) }));
    triggers.push_back(new TriggerNode(
        "omen of clarity", { NextAction("omen of clarity", ACTION_HIGH + 9) }));
    triggers.push_back(new TriggerNode("player has flag",
                                       { NextAction("dash", ACTION_EMERGENCY + 2) }));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       { NextAction("dash", ACTION_EMERGENCY + 2) }));
    triggers.push_back(
        new TriggerNode("berserk", { NextAction("berserk", ACTION_HIGH + 6) }));
}
