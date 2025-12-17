/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BattlegroundStrategy.h"

#include "Playerbots.h"

void BGStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", { new NextAction("bg join", relevance)}));
    triggers.push_back(new TriggerNode("bg invite active", { new NextAction("bg status check", relevance)}));
    triggers.push_back(new TriggerNode("timer", { new NextAction("bg strategy check", relevance)}));
}

BGStrategy::BGStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}

void BattlegroundStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg waiting", { new NextAction("bg move to start", ACTION_BG)}));
    triggers.push_back(new TriggerNode("bg active", { new NextAction("bg move to objective", ACTION_BG)}));
    triggers.push_back(new TriggerNode("often", { new NextAction("bg check objective", ACTION_BG + 1)}));
    triggers.push_back(new TriggerNode("dead", { new NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void WarsongStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { new NextAction("bg check flag", ACTION_EMERGENCY )}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { new NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f)}));
    triggers.push_back(new TriggerNode("team flagcarrier near", { new NextAction("bg protect fc", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("often", { new NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { new NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { new NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("player has flag", { new NextAction("bg move to objective", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("timer bg", { new NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void AlteracStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("alliance no snowfall gy", { new NextAction("bg move to objective", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("timer bg", { new NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void ArathiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { new NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { new NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { new NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { new NextAction("bg use buff", ACTION_MOVE)}));
}

void EyeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { new NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { new NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { new NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { new NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { new NextAction("attack enemy flag carrier", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("player has flag",{ new NextAction("bg move to objective", ACTION_EMERGENCY)}));
}

//TODO: Do Priorities
void IsleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { new NextAction("bg check flag", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("timer", { new NextAction("enter vehicle", ACTION_MOVE + 8.0f)}));
    triggers.push_back(new TriggerNode("random", { new NextAction("leave vehicle", ACTION_MOVE + 7.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("hurl boulder", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("fire cannon", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("napalm", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { new NextAction("steam blast", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("ram", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { new NextAction("ram", ACTION_MOVE + 9.1f)}));
    triggers.push_back(new TriggerNode("enemy out of melee", { new NextAction("steam rush", ACTION_MOVE + 9.2f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("incendiary rocket", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("rocket blast", ACTION_MOVE + 9.0f)}));
    // this is bugged: it doesn't work, and stops glaive throw working (which is needed to take down gate)
    // triggers.push_back(new TriggerNode("in vehicle", { new NextAction("blade salvo", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { new NextAction("glaive throw", ACTION_MOVE + 9.0f)}));
}

void ArenaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no possible targets", { new NextAction("arena tactics", ACTION_BG)}));
}
