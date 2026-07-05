/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarriorPullStrategy.h"

#include "PlayerbotAI.h"

std::string WarriorPullStrategy::GetPullActionName() const
{
    Unit* target = GetTarget();
    if (!target)
        return PullStrategy::GetPullActionName();

    if (botAI->CanCastSpell("heroic throw", target))
        return "heroic throw";

    return PullStrategy::GetPullActionName();
}
