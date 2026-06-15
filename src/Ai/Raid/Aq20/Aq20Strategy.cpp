/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Aq20Strategy.h"

#include "Strategy.h"

void RaidAq20Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("aq20 move to crystal",
            { NextAction("aq20 use crystal", ACTION_RAID) }));

}
