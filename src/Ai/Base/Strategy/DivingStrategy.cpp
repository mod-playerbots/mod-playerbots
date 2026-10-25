/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DivingStrategy.h"
#include "Playerbots.h"

void DivingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Emergency: beats looting/combat movement so a drowning bot surfaces first.
    triggers.push_back(new TriggerNode("underwater safety", { NextAction("surface from dive", 95.0f) }));
}
