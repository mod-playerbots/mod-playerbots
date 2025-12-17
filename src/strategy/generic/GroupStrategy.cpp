/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GroupStrategy.h"

#include "Playerbots.h"

void GroupStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", { new NextAction("invite nearby", 4.0f) }));
    triggers.push_back(new TriggerNode("random", { new NextAction("invite guild", 4.0f) }));
    triggers.push_back(new TriggerNode("random", { new NextAction("leave far away", 4.0f) }));
    triggers.push_back(new TriggerNode("seldom", { new NextAction("reset instances", 1.0f) }));
}
