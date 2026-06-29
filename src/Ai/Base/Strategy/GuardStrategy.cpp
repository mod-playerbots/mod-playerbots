/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuardStrategy.h"

std::vector<NextAction> GuardStrategy::getDefaultActions()
{
    return {
        NextAction("guard", 4.0f)
    };
}

void GuardStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/) {}
