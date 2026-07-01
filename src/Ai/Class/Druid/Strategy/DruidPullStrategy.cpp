/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidPullStrategy.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"

constexpr uint32 SPELL_DIRE_BEAR_FORM = 9634;

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

    // Fall back to Bear Form when Dire Bear Form isn't trained yet (< ~level 40).
    if (!botAI->GetBot()->HasSpell(SPELL_DIRE_BEAR_FORM))
        return "bear form";

    return PullStrategy::GetPreActionName();
}
