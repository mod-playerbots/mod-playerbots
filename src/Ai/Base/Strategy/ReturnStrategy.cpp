/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ReturnStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "PositionAction.h"

void ReturnStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "return",
            {
                CreateNextAction<SetReturnPositionAction>(1.5f),
                CreateNextAction<ReturnAction>(1.0f),
            }
        )
    );
}
