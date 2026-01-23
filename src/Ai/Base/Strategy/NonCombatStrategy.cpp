/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NonCombatStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "DropQuestAction.h"
#include "CheckMountStateAction.h"
#include "MovementActions.h"
#include "WorldBuffAction.h"
#include "FishingAction.h"
#include "EquipAction.h"

void NonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<CleanQuestLogAction>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "timer",
            {
                CreateNextAction<CheckMountStateAction>(1.0f)
            }
        )
    );
}

void CollisionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "collision",
            {
                CreateNextAction<MoveOutOfCollisionAction>(2.0f)
            }
        )
    );
}

void MountStrategy::InitTriggers(std::vector<TriggerNode*>&)
{
}

void WorldBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "need world buff",
            {
                CreateNextAction<WorldBuffAction>(1.0f)
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
                CreateNextAction<MoveNearWaterAction>(10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                CreateNextAction<FishingAction>(10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<EndMasterFishingAction>(12.0f),
                CreateNextAction<EquipUpgradesAction>(6.0f)
            }
        )
    );
}
