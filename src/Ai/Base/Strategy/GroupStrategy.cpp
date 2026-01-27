/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GroupStrategy.h"

#include "CreateNextAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "ResetInstancesAction.h"

void GroupStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<InviteNearbyToGroupAction>(4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<InviteGuildToGroupAction>(4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<LeaveFarAwayAction>(4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "seldom",
            {
                CreateNextAction<ResetInstancesAction>(1.0f)
            }
        )
    );
}
