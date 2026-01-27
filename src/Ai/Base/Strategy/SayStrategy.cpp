/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SayStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"

void SayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction("say::critical health", 99.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction("say::low health", 99.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction("say::low mana", 99.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "tank aoe",
            {
                CreateNextAction("say::taunt", 99.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction("say::aoe", 99.0f)
            }
        )
    );
}
