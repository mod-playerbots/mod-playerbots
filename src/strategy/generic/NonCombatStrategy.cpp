/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NonCombatStrategy.h"

#include "Playerbots.h"

void NonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", { new NextAction("clean quest log", 1.0f) }));
    triggers.push_back(new TriggerNode("timer", { new NextAction("check mount state", 1.0f) }));
}

void CollisionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("collision", { new NextAction("move out of collision", 2.0f) }));
}

void MountStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
}

void WorldBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "need world buff",
            {
                new NextAction("world buff", 1.0f)
            }
        )
    );
}

void MasterFishingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                new NextAction("move near water" , 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                new NextAction("go fishing" , 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                new NextAction("end master fishing", 12.0f),
                new NextAction("equip upgrades", 6.0f)
            }
        )
    );
}
