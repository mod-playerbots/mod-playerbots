/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GrindingStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "NonCombatActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"

std::vector<NextAction> GrindingStrategy::getDefaultActions()
{
    return {
        CreateNextAction<DrinkAction>(4.2f),
        CreateNextAction<EatAction>(4.1f),
    };
}

void GrindingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no target",
            {
                CreateNextAction<AttackAnythingAction>(4.0f)
            }
        )
    );
}

void MoveRandomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<MoveRandomAction>(1.5f)
            }
        )
    );
}
