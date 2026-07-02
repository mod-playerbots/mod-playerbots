/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FollowMasterStrategy.h"

std::vector<NextAction> FollowMasterStrategy::getDefaultActions()
{
    return {
        NextAction("follow", 1.0f)
    };
}

void FollowMasterStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}
