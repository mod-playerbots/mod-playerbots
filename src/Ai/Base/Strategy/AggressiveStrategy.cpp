/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AggressiveStrategy.h"

#include "ChooseTargetActions.h"
#include "CreateNextAction.h"

void AggressiveStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no target",
            {
                CreateNextAction<AggressiveTargetAction>(4.0f)
            }
        )
    );
}
