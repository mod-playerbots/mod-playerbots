/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LootNonCombatStrategy.h"

#include "Playerbots.h"

void LootNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("loot available", { new NextAction("loot", 6.0f) }));
    triggers.push_back(
        new TriggerNode("far from loot target", { new NextAction("move to loot", 7.0f) }));
    triggers.push_back(new TriggerNode("can loot", { new NextAction("open loot", 8.0f) }));
    triggers.push_back(new TriggerNode("often", { new NextAction("add all loot", 5.0f) }));
}

void GatherStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("timer", { new NextAction("add gathering loot", 5.0f) }));
}

void RevealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("often", { new NextAction("reveal gathering item", 50.0f) }));
}

void UseBobberStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
     triggers.push_back(
        new TriggerNode("can use fishing bobber", NextAction::array(0, new NextAction("use fishing bobber", 20.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("random", NextAction::array(0, new NextAction("remove bobber strategy", 20.0f), nullptr)));
}
