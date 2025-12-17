/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinStrategy.h"

#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"

GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("critical health", { new NextAction("divine shield",
        ACTION_HIGH + 5) }));
    triggers.push_back(
        new TriggerNode("hammer of justice interrupt",
                        { new NextAction("hammer of justice", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        { new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        { new NextAction("hammer of justice on snare target", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "critical health", { new NextAction("lay on hands", ACTION_EMERGENCY) }));
    triggers.push_back(
        new TriggerNode("party member critical health",
                        { new NextAction("lay on hands on party", ACTION_EMERGENCY + 1) }));
    triggers.push_back(new TriggerNode(
        "protect party member",
        { new NextAction("blessing of protection on party", ACTION_EMERGENCY + 2) }));
    triggers.push_back(
        new TriggerNode("high mana", { new NextAction("divine plea", ACTION_HIGH) }));
}

void PaladinCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "cleanse cure disease", { new NextAction("cleanse disease", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure disease",
                        { new NextAction("cleanse disease on party", ACTION_DISPEL + 1) }));
    triggers.push_back(new TriggerNode(
        "cleanse cure poison", { new NextAction("cleanse poison", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure poison",
                        { new NextAction("cleanse poison on party", ACTION_DISPEL + 1) }));
    triggers.push_back(new TriggerNode(
        "cleanse cure magic", { new NextAction("cleanse magic", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure magic",
                        { new NextAction("cleanse magic on party", ACTION_DISPEL + 1) }));
}

void PaladinBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

    // triggers.push_back(new TriggerNode("divine favor", { new NextAction("divine favor",
    // ACTION_HIGH + 1) }));
}

void PaladinCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("turn undead", { new NextAction("turn undead", ACTION_HIGH + 1) }));
}

void PaladinHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        {
                            new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                            new NextAction("holy shock", ACTION_DEFAULT + 0.5f),
                            new NextAction("shield of righteousness", ACTION_DEFAULT + 0.4f),
                            new NextAction("judgement of light", ACTION_DEFAULT + 0.3f),
                            new NextAction("consecration", ACTION_DEFAULT + 0.2f),
                            new NextAction("exorcism", ACTION_DEFAULT+ 0.1f),
                            }));
}
