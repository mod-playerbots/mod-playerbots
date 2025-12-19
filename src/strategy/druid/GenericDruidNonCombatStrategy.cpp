/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDruidNonCombatStrategy.h"

#include "Playerbots.h"
#include "AiFactory.h"

class GenericDruidNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidNonCombatStrategyActionNodeFactory()
    {
        creators["thorns"] = &thorns;
        creators["thorns on party"] = &thorns_on_party;
        creators["mark of the wild"] = &mark_of_the_wild;
        creators["mark of the wild on party"] = &mark_of_the_wild_on_party;
        // creators["innervate"] = &innervate;
        creators["regrowth_on_party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["revive"] = &revive;
    }

private:
    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* thorns_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* mark_of_the_wild([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mark of the wild",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* mark_of_the_wild_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mark of the wild on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse on party",
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
    static ActionNode* revive([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("revive",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("mark of the wild", { NextAction("mark of the wild", 14.0f) }));
    triggers.push_back(new TriggerNode("party member cure poison", { NextAction("abolish poison on party", 20.0f) }));
    triggers.push_back(new TriggerNode("party member dead", { NextAction("revive", ACTION_CRITICAL_HEAL + 10) }));

    triggers.push_back(new TriggerNode("often", { NextAction("apply oil", 1.0f) }));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        {
                                          NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 7),
                                          NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 6),
                                          NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 5),
                                          }));

    triggers.push_back(
        new TriggerNode("party member low health",
                        {
                                          NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 5),
                                          NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 4),
                                          NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 3),
                                          }));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        { NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 3),
                                          NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 2),
                                          NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 1),
                                          }));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        { NextAction("wild growth on party", ACTION_LIGHT_HEAL + 3), NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 2) }));

    triggers.push_back(
        new TriggerNode("party member remove curse",
                        { NextAction("remove curse on party", ACTION_DISPEL + 7) }));
    triggers.push_back(
        new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));

    triggers.push_back(new TriggerNode("party member critical health", {
                       NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 7),
                       NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 6),
                       NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 5),
                       }));
    triggers.push_back(new TriggerNode("party member low health", {
                       NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 5),
                       NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 4),
                       NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 3),
                       }));
    triggers.push_back(new TriggerNode("party member medium health", {
                       NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 3),
                       NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 2),
                       NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 1),
                       }));
    triggers.push_back(new TriggerNode("party member almost full health", {
                       NextAction("wild growth on party", ACTION_LIGHT_HEAL + 3),
                       NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 2),
                       }));
    triggers.push_back(new TriggerNode("party member remove curse", {
                       NextAction("remove curse on party", ACTION_DISPEL + 7),
                       }));

    int specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (specTab == 0 || specTab == 2) // Balance or Restoration
        triggers.push_back(new TriggerNode("often", { NextAction("apply oil", 1.0f) }));
    if (specTab == 1) // Feral
        triggers.push_back(new TriggerNode("often", { NextAction("apply stone", 1.0f) }));

}

GenericDruidBuffStrategy::GenericDruidBuffStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("mark of the wild on party", {
                       NextAction("mark of the wild on party", 13.0f),
                       }));
    triggers.push_back(new TriggerNode("thorns on main tank", {
                       NextAction("thorns on main tank", 11.0f),
                       }));
    triggers.push_back(new TriggerNode("thorns", {
                       NextAction("thorns", 10.0f),
                       }));
}
