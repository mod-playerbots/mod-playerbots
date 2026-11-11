/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SellJunkStrategy.h"

#include "Playerbots.h"

void SellJunkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("can sell at vendor", { NextAction("sell vendor", ACTION_DEFAULT) }));
}
