/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DeathKnightPullStrategy.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string DeathKnightPullStrategy::GetPullActionName() const
{
    Unit* target = GetTarget();
    if (!target ||
        (!botAI->HasStrategy("blood", BOT_STATE_COMBAT) && !botAI->HasStrategy("blood", BOT_STATE_NON_COMBAT)))
    {
        return PullStrategy::GetPullActionName();
    }

    if (botAI->CanCastSpell("death grip", target))
        return "death grip";

    if (!botAI->CanCastSpell("icy touch", target) &&
        botAI->CanCastSpell("dark command", target))
    {
        return "dark command";
    }

    return PullStrategy::GetPullActionName();
}
