/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GroupStrategy.h"

#include "Playerbots.h"

void GroupStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", { NextAction("invite nearby", 4.0f) }));
    triggers.push_back(new TriggerNode("random", { NextAction("invite guild", 4.0f) }));
    triggers.push_back(new TriggerNode("random", { NextAction("leave far away", 4.0f) }));
    triggers.push_back(new TriggerNode("seldom", { NextAction("reset instances", 1.0f) }));
}
