/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SecurityCheckAction.h"

#include "Event.h"
#include "Playerbots.h"

bool SecurityCheckAction::isUseful()
{
    if (!RandomPlayerbotMgr::instance().IsRandomBot(this->bot))
    {
        return false;
    }

    Player* const master = this->botAI->GetMaster();

    return master != nullptr
        && master->GetSession()->GetSecurity() < SEC_GAMEMASTER
        && !GET_PLAYERBOT_AI(master);
}
bool SecurityCheckAction::Execute(Event)
{
    const Group* const group = bot->GetGroup();

    if (group == nullptr)
    {
        return false;
    }

    const LootMethod method = group->GetLootMethod();
    const ItemQualities threshold = group->GetLootThreshold();

    if (method != MASTER_LOOT && method != FREE_FOR_ALL && threshold <= ITEM_QUALITY_UNCOMMON)
    {
        return false;
    }

    if (botAI->GetGroupLeader()->GetSession()->GetSecurity() != SEC_PLAYER)
    {
        return true;
    }

    uint32_t guildId = this->bot->GetGuildId();

    if (!guildId || guildId != botAI->GetGroupLeader()->GetGuildId())
    {
        this->botAI->TellError("I will play with this loot type only if I'm in your guild :/");
        this->botAI->ChangeStrategy("+passive,+stay", BOT_STATE_NON_COMBAT);
        this->botAI->ChangeStrategy("+passive,+stay", BOT_STATE_COMBAT);
    }

    return true;
}
