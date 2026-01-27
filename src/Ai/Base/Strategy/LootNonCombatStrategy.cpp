/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LootNonCombatStrategy.h"

#include "CreateNextAction.h"
#include "LootAction.h"
#include "MovementActions.h"
#include "AddLootAction.h"
#include "RevealGatheringItemAction.h"
#include "FishingAction.h"

void LootNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "loot available",
            {
                CreateNextAction<LootAction>(6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from loot target",
            {
                CreateNextAction<MoveToLootAction>(7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "can loot",
            {
                CreateNextAction<OpenLootAction>(8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<AddAllLootAction>(5.0f)
            }
        )
    );
}

void GatherStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<AddGatheringLootAction>(5.0f)
            }
        )
    );
}

void RevealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<RevealGatheringItemAction>(50.0f)
            }
        )
    );
}

void UseBobberStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
     triggers.push_back(
        new TriggerNode(
            "can use fishing bobber",
            {
                CreateNextAction<UseBobberAction>(20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<RemoveBobberStrategyAction>(20.0f)
            }
        )
    );
}
