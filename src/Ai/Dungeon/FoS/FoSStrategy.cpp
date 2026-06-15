/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FoSStrategy.h"
#include "FoSMultipliers.h"

void WotlkDungeonFoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("move from bronjahm",
        { NextAction("move from bronjahm", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("switch to soul fragment",
        { NextAction("attack corrupted soul fragment", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("bronjahm position",
        { NextAction("bronjahm group position", ACTION_RAID + 1) }));
    triggers.push_back(new TriggerNode("devourer of souls",
        { NextAction("devourer of souls", ACTION_RAID + 1) }));
}

void WotlkDungeonFoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BronjahmMultiplier(botAI));
}
