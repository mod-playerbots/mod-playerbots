/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPvpPursueStrategy.h"

#include "Playerbots.h"

void WorldPvpPursueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("world pvp pursue", { NextAction("move to nearest real player", 6.0f) }));
}
