/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
