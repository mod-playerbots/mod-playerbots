/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinPullStrategy.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string PaladinPullStrategy::GetPullActionName() const
{
    Unit* target = GetTarget();
    if (!target ||
        (!botAI->HasStrategy("tank", BOT_STATE_COMBAT) && !botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT)))
    {
        return PullStrategy::GetPullActionName();
    }

    if (botAI->CanCastSpell("avenger's shield", target))
        return "avenger's shield";

    if (botAI->CanCastSpell("hand of reckoning", target))
        return "hand of reckoning";

    return PullStrategy::GetPullActionName();
}

std::string PaladinPullStrategy::GetPreActionName() const
{
    if (botAI->HasStrategy("tank", BOT_STATE_COMBAT) || botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT))
        return "";

    return PullStrategy::GetPreActionName();
}
