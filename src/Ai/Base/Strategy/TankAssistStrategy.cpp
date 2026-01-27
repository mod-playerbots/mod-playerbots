/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankAssistStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "ChooseTargetAction.h"

void TankAssistStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "tank assist",
            {
                CreateNextAction<TankAssistAction>(50.0f)
            }
        )
    );
}
