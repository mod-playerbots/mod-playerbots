/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PassThroughStrategy.h"

#include "Playerbots.h"

void PassThroughStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    for (std::vector<std::string>::iterator i = supported.begin(); i != supported.end(); i++)
        triggers.push_back(
            new TriggerNode(i->c_str(), { NextAction(i->c_str(), relevance) }));
}
