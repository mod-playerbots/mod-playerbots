/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDruidStrategy.h"

#include "Playerbots.h"

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["caster form"] = &caster_form;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["rebirth"] = &rebirth;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["innervate"] = &innervate;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* caster_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("caster form",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison on party",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* rebirth([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rebirth",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots on cc",
                              /*P*/ { new NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("innervate",
                              /*P*/ {},
                              /*A*/ { new NextAction("mana potion") },
                              /*C*/ {});
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("low health", { new NextAction("barkskin", ACTION_HIGH + 7) }));

    triggers.push_back(new TriggerNode("combat party member dead",
                                       { new NextAction("rebirth", ACTION_HIGH + 9) }));
    triggers.push_back(new TriggerNode("being attacked",
                                       { new NextAction("nature's grasp", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode("new pet", { new NextAction("set pet stance", 60.0f) }));
}

void DruidCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("party member cure poison",
                        { new NextAction("abolish poison on party", ACTION_DISPEL + 1) }));

    triggers.push_back(
        new TriggerNode("party member remove curse",
                        { new NextAction("remove curse on party", ACTION_DISPEL + 7) }));

}

void DruidBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "nature's swiftness", { new NextAction("nature's swiftness", ACTION_HIGH + 9) }));
}

void DruidCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "entangling roots", { new NextAction("entangling roots on cc", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode(
        "entangling roots kite", { new NextAction("entangling roots", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode(
        "hibernate", { new NextAction("hibernate on cc", ACTION_HIGH + 3) }));
}

void DruidHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        {
                            new NextAction("cancel tree form", ACTION_DEFAULT + 0.3f),
                            new NextAction("moonfire", ACTION_DEFAULT + 0.2f),
                            new NextAction("wrath", ACTION_DEFAULT + 0.1f),
                            new NextAction("starfire", ACTION_DEFAULT),
}));

}
