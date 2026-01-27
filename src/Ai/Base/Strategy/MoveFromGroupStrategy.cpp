/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MoveFromGroupStrategy.h"
#include "PassiveMultiplier.h"
#include "CreateNextAction.h"
#include "MovementActions.h"

std::vector<NextAction> MoveFromGroupStrategy::getDefaultActions()
{
    return {
        CreateNextAction<MoveFromGroupAction>(1.0f)
    };
}

void MoveFromGroupStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PassiveMultiplier(botAI));
}
