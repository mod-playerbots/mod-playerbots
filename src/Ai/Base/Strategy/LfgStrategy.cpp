/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgStrategy.h"

#include "Playerbots.h"

void LfgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", { NextAction("lfg join", relevance) }));
    triggers.push_back(
        new TriggerNode("seldom", { NextAction("lfg leave", relevance) }));
    triggers.push_back(new TriggerNode(
        "unknown dungeon", { NextAction("give leader in dungeon", relevance) }));
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}
