/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LfgStrategy.h"

#include "Playerbots.h"

void LfgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", { new NextAction("lfg join", relevance) }));
    triggers.push_back(
        new TriggerNode("seldom", { new NextAction("lfg leave", relevance) }));
    triggers.push_back(new TriggerNode(
        "unknown dungeon", { new NextAction("give leader in dungeon", relevance) }));
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}
