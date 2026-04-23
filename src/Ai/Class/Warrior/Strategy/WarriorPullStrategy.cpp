/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarriorPullStrategy.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"

std::string WarriorPullStrategy::GetPullActionName() const
{
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    if (!bot || !target)
        return PullStrategy::GetPullActionName();

    uint32 const heroicThrowSpellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "heroic throw")->Get();
    if (heroicThrowSpellId && bot->HasSpell(heroicThrowSpellId) &&
        botAI->CanCastSpell(heroicThrowSpellId, target))
    {
        return "heroic throw";
    }

    return PullStrategy::GetPullActionName();
}
