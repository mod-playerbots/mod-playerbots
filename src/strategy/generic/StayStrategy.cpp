/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
