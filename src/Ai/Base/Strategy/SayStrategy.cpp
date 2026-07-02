/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SayStrategy.h"

#include "Playerbots.h"

void SayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("critical health",
                                       { NextAction("say::critical health", 99.0f) }));
    triggers.push_back(
        new TriggerNode("low health", { NextAction("say::low health", 99.0f) }));
    triggers.push_back(
        new TriggerNode("low mana", { NextAction("say::low mana", 99.0f) }));
    triggers.push_back(new TriggerNode("tank aoe", { NextAction("say::taunt", 99.0f) }));
    triggers.push_back(new TriggerNode("medium aoe", { NextAction("say::aoe", 99.0f) }));
}
