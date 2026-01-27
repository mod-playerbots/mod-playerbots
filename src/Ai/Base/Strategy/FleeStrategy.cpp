/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FleeStrategy.h"

#include "CreateNextAction.h"
#include "MovementActions.h"

void FleeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "panic",
            {
                CreateNextAction<FleeAction>(ACTION_EMERGENCY + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "outnumbered",
            {
                CreateNextAction<FleeAction>(ACTION_EMERGENCY + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<FleeAction>(ACTION_MEDIUM_HEAL)
            }
        )
    );
}

void FleeFromAddsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "has nearest adds",
            {
                CreateNextAction<RunAwayAction>(50.0f)
            }
        )
    );
}
