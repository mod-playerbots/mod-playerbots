/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MeleeCombatStrategy.h"

#include "Playerbots.h"

void MeleeCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee", { NextAction("reach melee", ACTION_HIGH + 1) }));
}

void SetBehindCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("not behind target",
                                       { NextAction("set behind", ACTION_MOVE + 7) }));
}
