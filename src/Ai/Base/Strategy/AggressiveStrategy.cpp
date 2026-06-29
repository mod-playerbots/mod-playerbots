/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AggressiveStrategy.h"

#include "Playerbots.h"

void AggressiveStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no target",
            {
                NextAction("aggressive target", 4.0f)
            }
        )
    );
}
