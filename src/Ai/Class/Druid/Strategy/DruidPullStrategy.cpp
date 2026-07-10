/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DruidPullStrategy.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string DruidPullStrategy::GetPullActionName() const
{
    std::string const pullActionName = PullStrategy::GetPullActionName();
    std::string const actionName =
        botAI->HasSpell("faerie fire (feral)") &&
            (botAI->HasStrategy("bear", BOT_STATE_COMBAT) || botAI->HasStrategy("cat", BOT_STATE_COMBAT))
            ? "faerie fire (feral)" : pullActionName;

    Unit* target = GetTarget();
    if (!target)
        return actionName;

    if (!botAI->CanCastSpell(actionName, target) && botAI->CanCastSpell("growl", target))
        return "growl";

    return actionName;
}

std::string DruidPullStrategy::GetPreActionName() const
{
    if (GetPullActionName() == "faerie fire")
        return "";

    return PullStrategy::GetPreActionName();
}
