/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FleeStrategy.h"

#include "Playerbots.h"

void FleeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("panic", { NextAction("flee", ACTION_EMERGENCY + 9) }));
    triggers.push_back(
        new TriggerNode("outnumbered", { NextAction("flee", ACTION_EMERGENCY + 9) }));
    triggers.push_back(
        new TriggerNode("critical health", { NextAction("flee", ACTION_MEDIUM_HEAL) }));
}

void FleeFromAddsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("has nearest adds", { NextAction("runaway", 50.0f) }));
}
