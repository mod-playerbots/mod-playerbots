/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TravelStrategy.h"

#include "Playerbots.h"

TravelStrategy::TravelStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TravelStrategy::getDefaultActions()
{
    return {
        NextAction("travel", 1.0f)
    };
}

void TravelStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no travel target",
            {
                NextAction("choose travel target", 6.f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from travel target",
            {
                NextAction("move to travel target", 1)
            }
        )
    );
}
