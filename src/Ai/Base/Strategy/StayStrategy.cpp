/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "StayStrategy.h"

#include "Playerbots.h"

void StayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "return to stay position",
            {
                NextAction("return to stay position", ACTION_MOVE)
            }
        )
    );
}

std::vector<NextAction> StayStrategy::getDefaultActions()
{
    return {
        NextAction("stay", 1.0f)
    };
}

void SitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "sit",
            {
                NextAction("sit", 1.5f)
            }
        )
    );
}
