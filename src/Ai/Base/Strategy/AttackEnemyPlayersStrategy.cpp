/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AttackEnemyPlayersStrategy.h"

#include "Playerbots.h"
#include "CreateAction.h"
#include "ChooseTargetActions.h"
#include "CreateNextAction.h"

void AttackEnemyPlayersStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "enemy player near",
            {
                CreateNextAction<AttackEnemyPlayerAction>(55.0f)
            }
        )
    );
}
