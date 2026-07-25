/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReturnStrategy.h"

#include "Playerbots.h"

void ReturnStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("return", { NextAction("set return position", 1.5f),
                                                                   NextAction("return", 1.0f), }));
}
