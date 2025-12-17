/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPriestStrategy.h"

#include "GenericPriestStrategyActionNodeFactory.h"
#include "HealPriestStrategy.h"
#include "Playerbots.h"

GenericPriestStrategy::GenericPriestStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

void GenericPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("medium threat", { new NextAction("fade", 55.0f) }));
    triggers.push_back(new TriggerNode("critical health", { new NextAction("desperate prayer",
        ACTION_HIGH + 5) }));
    triggers.push_back(new TriggerNode(
        "critical health", { new NextAction("power word: shield", ACTION_NORMAL) }));

    triggers.push_back(
        new TriggerNode("low health", { new NextAction("power word: shield", ACTION_HIGH) }));

    triggers.push_back(
        new TriggerNode("medium mana",
            {
                new NextAction("shadowfiend", ACTION_HIGH + 2),
                new NextAction("inner focus", ACTION_HIGH + 1) }));

    triggers.push_back(
        new TriggerNode("low mana", { new NextAction("hymn of hope", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       { new NextAction("flee", ACTION_MOVE + 9) }));
    triggers.push_back(new TriggerNode("often", { new NextAction("apply oil", 1.0f) }));
    triggers.push_back(new TriggerNode("being attacked",
        { new NextAction("power word: shield", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode("new pet", { new NextAction("set pet stance", 60.0f) }));
}

PriestCureStrategy::PriestCureStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new CurePriestStrategyActionNodeFactory());
}

void PriestCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("dispel magic", { new NextAction("dispel magic", 41.0f) }));
    triggers.push_back(new TriggerNode("dispel magic on party",
                                       { new NextAction("dispel magic on party", 40.0f) }));
    triggers.push_back(
        new TriggerNode("cure disease", { new NextAction("abolish disease", 31.0f) }));
    triggers.push_back(new TriggerNode(
        "party member cure disease", { new NextAction("abolish disease on party", 30.0f) }));
}

void PriestBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("power infusion", { new NextAction("power infusion", 41.0f) }));
    triggers.push_back(new TriggerNode("boost", { new NextAction("shadowfiend", 20.0f) }));
}

void PriestCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shackle undead", { new NextAction("shackle undead", 31.0f) }));
}

void PriestHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        {
                            new NextAction("shadow word: pain", ACTION_DEFAULT + 0.5f),
                            new NextAction("holy fire", ACTION_DEFAULT + 0.4f),
                            new NextAction("smite", ACTION_DEFAULT + 0.3f),
                            new NextAction("mind blast", ACTION_DEFAULT + 0.2f),
                            new NextAction("shoot", ACTION_DEFAULT) }));

    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        {
                            new NextAction("mind sear", ACTION_DEFAULT + 0.5f) }));
}
