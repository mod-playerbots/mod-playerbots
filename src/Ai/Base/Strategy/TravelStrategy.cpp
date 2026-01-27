/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TravelStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "TravelAction.h"
#include "ChooseTravelTargetAction.h"
#include "MoveToTravelTargetAction.h"

TravelStrategy::TravelStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TravelStrategy::getDefaultActions()
{
    return {
        CreateNextAction<TravelAction>(1.0f)
    };
}

void TravelStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no travel target",
            {
                CreateNextAction<ChooseTravelTargetAction>(6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from travel target",
            {
                CreateNextAction<MoveToTravelTargetAction>(1.0f)
            }
        )
    );
}
