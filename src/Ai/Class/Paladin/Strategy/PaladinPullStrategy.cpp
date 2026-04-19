/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinPullStrategy.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string PaladinPullStrategy::GetPullActionName() const
{
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    if (!bot || !target ||
        (!botAI->HasStrategy("tank", BOT_STATE_COMBAT) && !botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT)))
    {
        return PullStrategy::GetPullActionName();
    }

    uint32 const avengersShieldSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "avenger's shield")->Get();
    if (avengersShieldSpellId && bot->HasSpell(avengersShieldSpellId) &&
        botAI->CanCastSpell(avengersShieldSpellId, target))
    {
        return "avenger's shield";
    }

    uint32 const handOfReckoningSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "hand of reckoning")->Get();
    if (handOfReckoningSpellId && bot->HasSpell(handOfReckoningSpellId) &&
        botAI->CanCastSpell(handOfReckoningSpellId, target))
    {
        return "hand of reckoning";
    }

    return PullStrategy::GetPullActionName();
}

std::string PaladinPullStrategy::GetPreActionName() const
{
    if (botAI->HasStrategy("tank", BOT_STATE_COMBAT) || botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT))
        return "";

    return PullStrategy::GetPreActionName();
}
