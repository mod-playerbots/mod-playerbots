/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
