/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GuildStrategy.h"

#include "Playerbots.h"

void GuildStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("often", { new NextAction("offer petition nearby", 4.0f) }));
    triggers.push_back(
        new TriggerNode("often", { new NextAction("guild manage nearby", 4.0f) }));
    triggers.push_back(
        new TriggerNode("petition signed", { new NextAction("turn in petition", 10.0f) }));
    triggers.push_back(
        new TriggerNode("buy tabard", { new NextAction("buy tabard", 10.0f) }));
    triggers.push_back(
        new TriggerNode("leave large guild", { new NextAction("guild leave", 4.0f) }));
}
