/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShamanNonCombatStrategy.h"
#include "AiFactory.h"
#include "Strategy.h"
#include "Playerbots.h"

class ShamanNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShamanNonCombatStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["windfury weapon"] = &windfury_weapon;
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["wind shear"] = &wind_shear;
        creators["purge"] = &purge;
    }

private:
    static ActionNode* flametongue_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue weapon",
                              /*P*/ {},
                              /*A*/ { new NextAction("rockbiter weapon") },
                              /*C*/ {});
    }
    static ActionNode* frostbrand_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbrand weapon",
                              /*P*/ {},
                              /*A*/ { new NextAction("flametongue weapon") },
                              /*C*/ {});
    }
    static ActionNode* windfury_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury weapon",
                              /*P*/ {},
                              /*A*/ { new NextAction("flametongue weapon") },
                              /*C*/ {});
    }
    static ActionNode* earthliving_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("earthliving weapon",
                              /*P*/ {},
                              /*A*/ { new NextAction("flametongue weapon") },
                              /*C*/ {});
    }
    static ActionNode* wind_shear(PlayerbotAI*) { return new ActionNode("wind shear", {}, {}, {}); }
    static ActionNode* purge(PlayerbotAI*) { return new ActionNode("purge", {}, {}, {}); }
};

ShamanNonCombatStrategy::ShamanNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new ShamanNonCombatStrategyActionNodeFactory());
}

void ShamanNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // Totemic Recall
    triggers.push_back(new TriggerNode("totemic recall", { new NextAction("totemic recall", 60.0f), }));

    // Healing/Resurrect Triggers
    triggers.push_back(new TriggerNode("party member dead", { new NextAction("ancestral spirit", ACTION_CRITICAL_HEAL + 10), }));
    triggers.push_back(new TriggerNode("party member critical health", {
                                                                   new NextAction("riptide on party", 31.0f),
                                                                   new NextAction("healing wave on party", 30.0f) }));
    triggers.push_back(new TriggerNode("party member low health",{
                                                             new NextAction("riptide on party", 29.0f),
                                                             new NextAction("healing wave on party", 28.0f) }));
    triggers.push_back(new TriggerNode("party member medium health",{
                                                                new NextAction("riptide on party", 27.0f),
                                                                new NextAction("healing wave on party", 26.0f) }));
    triggers.push_back(new TriggerNode("party member almost full health",{
                                                                     new NextAction("riptide on party", 25.0f),
                                                                     new NextAction("lesser healing wave on party", 24.0f) }));
    triggers.push_back(new TriggerNode("group heal setting",{ new NextAction("chain heal on party", 27.0f) }));

    // Cure Triggers
    triggers.push_back(new TriggerNode("cure poison", { new NextAction("cure poison", 21.0f), }));
    triggers.push_back(new TriggerNode("party member cure poison", { new NextAction("cure poison on party", 21.0f), }));
    triggers.push_back(new TriggerNode("cure disease", { new NextAction("cure disease", 31.0f), }));
    triggers.push_back(new TriggerNode("party member cure disease", { new NextAction("cure disease on party", 30.0f), }));

    // Out of Combat Buff Triggers
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(new TriggerNode("main hand weapon no imbue", { new NextAction("flametongue weapon", 22.0f), }));
        triggers.push_back(new TriggerNode("water shield", { new NextAction("water shield", 21.0f), }));
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(new TriggerNode("main hand weapon no imbue", { new NextAction("windfury weapon", 22.0f), }));
        triggers.push_back(new TriggerNode("off hand weapon no imbue", { new NextAction("flametongue weapon", 21.0f), }));
        triggers.push_back(new TriggerNode("lightning shield", { new NextAction("lightning shield", 20.0f), }));
    }
    else if (tab == 2)  // Restoration
    {
        triggers.push_back(new TriggerNode("main hand weapon no imbue",{ new NextAction("earthliving weapon", 22.0f), }));
        triggers.push_back(new TriggerNode("water shield", { new NextAction("water shield", 20.0f), }));
    }

    // Buff Triggers while swimming
    triggers.push_back(new TriggerNode("water breathing", { new NextAction("water breathing", 12.0f), }));
    triggers.push_back(new TriggerNode("water walking", { new NextAction("water walking", 12.0f), }));
    triggers.push_back(new TriggerNode("water breathing on party", { new NextAction("water breathing on party", 11.0f), }));
    triggers.push_back(new TriggerNode("water walking on party", { new NextAction("water walking on party", 11.0f), }));

    // Pet Triggers
    triggers.push_back(new TriggerNode("has pet", { new NextAction("toggle pet spell", 60.0f), }));
    triggers.push_back(new TriggerNode("new pet", { new NextAction("set pet stance", 65.0f), }));
}

void ShamanNonCombatStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}
