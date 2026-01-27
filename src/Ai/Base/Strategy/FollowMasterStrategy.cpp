/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FollowMasterStrategy.h"

#include "CreateNextAction.h"
#include "FollowActions.h"

std::vector<NextAction> FollowMasterStrategy::getDefaultActions()
{
    return {
        CreateNextAction<FollowAction>(1.0f)
    };
}

void FollowMasterStrategy::InitTriggers(std::vector<TriggerNode*>&)
{
}
