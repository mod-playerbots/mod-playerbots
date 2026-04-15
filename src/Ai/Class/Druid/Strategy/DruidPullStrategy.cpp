/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidPullStrategy.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string DruidPullStrategy::GetPullActionName() const
{
    Player* bot = botAI->GetBot();
    std::string actionName = PullStrategy::GetPullActionName();
    if (!bot)
        return actionName;

    uint32 const faerieFireFeralId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "faerie fire (feral)")->Get();
    if (faerieFireFeralId && bot->HasSpell(faerieFireFeralId) &&
        (botAI->HasStrategy("bear", BOT_STATE_COMBAT) || botAI->HasStrategy("cat", BOT_STATE_COMBAT)))
    {
        actionName = "faerie fire (feral)";
    }

    Unit* target = GetTarget();
    uint32 const faerieFireSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", actionName)->Get();
    if (target && (!faerieFireSpellId || !bot->HasSpell(faerieFireSpellId) ||
        !botAI->CanCastSpell(faerieFireSpellId, target)))
    {
        uint32 const growlSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "growl")->Get();
        if (growlSpellId && bot->HasSpell(growlSpellId) && botAI->CanCastSpell(growlSpellId, target))
            return "growl";
    }

    return actionName;
}

std::string DruidPullStrategy::GetPreActionName() const
{
    if (GetPullActionName() == "faerie fire")
        return "";

    return PullStrategy::GetPreActionName();
}
