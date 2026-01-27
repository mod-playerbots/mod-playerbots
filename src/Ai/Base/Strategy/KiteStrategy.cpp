/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "KiteStrategy.h"

#include "CreateNextAction.h"
#include "MovementActions.h"

KiteStrategy::KiteStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

void KiteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "has aggro",
            {
                CreateNextAction<RunAwayAction>(51.0f)
            }
        )
    );
}
