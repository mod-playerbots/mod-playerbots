/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MeleeCombatStrategy.h"

#include "CreateNextAction.h"
#include "ReachTargetActions.h"
#include "MovementActions.h"

void MeleeCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}

void SetBehindCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "not behind target",
            {
                CreateNextAction<SetBehindTargetAction>(ACTION_MOVE + 7.0f)
            }
        )
    );
}
