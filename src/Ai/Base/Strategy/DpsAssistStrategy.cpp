/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DpsAssistStrategy.h"

#include "CreateNextAction.h"
#include "ChooseTargetActions.h"

void DpsAssistStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "not dps target active",
            {
                CreateNextAction<DpsAssistAction>(50.0f)
            }
        )
    );
}

void DpsAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "not dps aoe target active",
            {
                CreateNextAction<DpsAoeAction>(50.0f)
            }
        )
    );
}
