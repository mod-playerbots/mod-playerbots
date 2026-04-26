/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DeathKnightPullStrategy.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string DeathKnightPullStrategy::GetPullActionName() const
{
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    if (!bot || !target ||
        (!botAI->HasStrategy("blood", BOT_STATE_COMBAT) && !botAI->HasStrategy("blood", BOT_STATE_NON_COMBAT)))
    {
        return PullStrategy::GetPullActionName();
    }

    uint32 const deathGripSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "death grip")->Get();
    if (deathGripSpellId && bot->HasSpell(deathGripSpellId) &&
        botAI->CanCastSpell(deathGripSpellId, target))
    {
        return "death grip";
    }

    uint32 const icyTouchSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "icy touch")->Get();
    if (!icyTouchSpellId || !bot->HasSpell(icyTouchSpellId) ||
        !botAI->CanCastSpell(icyTouchSpellId, target))
    {
        uint32 const darkCommandSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "dark command")->Get();
        if (darkCommandSpellId && bot->HasSpell(darkCommandSpellId) &&
            botAI->CanCastSpell(darkCommandSpellId, target))
        {
            return "dark command";
        }
    }

    return PullStrategy::GetPullActionName();
}
