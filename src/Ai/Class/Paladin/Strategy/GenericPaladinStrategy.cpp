/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinStrategy.h"

#include "GenericPaladinStrategyActionNodeFactory.h"

GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("critical health", { NextAction("divine shield",
        ACTION_HIGH + 5) }));
    triggers.push_back(
        new TriggerNode("hammer of justice interrupt",
                        { NextAction("hammer of justice", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        { NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        { NextAction("hammer of justice on snare target", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "critical health", { NextAction("lay on hands", ACTION_EMERGENCY) }));
    triggers.push_back(
        new TriggerNode("party member critical health",
                        { NextAction("lay on hands on party", ACTION_EMERGENCY + 1) }));
    triggers.push_back(new TriggerNode(
        "protect party member",
        { NextAction("blessing of protection on party", ACTION_EMERGENCY + 2) }));
    triggers.push_back(
        new TriggerNode("high mana", { NextAction("divine plea", ACTION_HIGH) }));
}

void PaladinCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "cleanse cure disease", { NextAction("cleanse disease", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure disease",
                        { NextAction("cleanse disease on party", ACTION_DISPEL + 1) }));
    triggers.push_back(new TriggerNode(
        "cleanse cure poison", { NextAction("cleanse poison", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure poison",
                        { NextAction("cleanse poison on party", ACTION_DISPEL + 1) }));
    triggers.push_back(new TriggerNode(
        "cleanse cure magic", { NextAction("cleanse magic", ACTION_DISPEL + 2) }));
    triggers.push_back(
        new TriggerNode("cleanse party member cure magic",
                        { NextAction("cleanse magic on party", ACTION_DISPEL + 1) }));
}

void PaladinBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

    // triggers.push_back(new TriggerNode("divine favor", { NextAction("divine favor",
    // ACTION_HIGH + 1) }));
}

void PaladinCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("turn undead", { NextAction("turn undead", ACTION_HIGH + 1) }));
}

void PaladinHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        {
                            NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                            NextAction("holy shock", ACTION_DEFAULT + 0.5f),
                            NextAction("shield of righteousness", ACTION_DEFAULT + 0.4f),
                            NextAction("judgement of light", ACTION_DEFAULT + 0.3f),
                            NextAction("consecration", ACTION_DEFAULT + 0.2f),
                            NextAction("exorcism", ACTION_DEFAULT+ 0.1f),
                            }));
}
