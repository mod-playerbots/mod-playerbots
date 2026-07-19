/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KiteStrategy.h"

#include "Playerbots.h"

KiteStrategy::KiteStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

void KiteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("has aggro", { NextAction("runaway", 51.0f) }));
}
