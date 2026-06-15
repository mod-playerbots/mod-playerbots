/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetMasterAction.h"

#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "BroadcastHelper.h"

Player* SetMasterAction::GetPlayer(Event event)
{
    Player* player = nullptr;
    ObjectGuid guid = event.getObject();

    if (guid)
    {
        player = ObjectAccessor::FindPlayer(guid);
        if (player)
            return player;
    }

    std::string text = event.getParam();

    if (!text.empty())
    {
        if (normalizePlayerName(text))
        {
            player = ObjectAccessor::FindPlayerByName(text.c_str());
            if (player)
                return player;
        }

        return nullptr;
    }

    Player* master = GetMaster();
    if (!master)
        guid = bot->GetTarget();
    else
        guid = master->GetTarget();

    player = ObjectAccessor::FindPlayer(guid);
    if (player)
        return player;

    player = event.getOwner();
    if (player)
        return player;

    return nullptr;
}

bool SetMasterAction::Execute(Event event)
{
    Player* newMaster = GetPlayer(event);
    if (!newMaster)
        return false;

    if (newMaster == bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group || !group->IsMember(newMaster->GetGUID()))
        return false;

    botAI->SetMaster(newMaster);
    botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);

    std::string msg = "Now following " + std::string(newMaster->GetName()) + ".";
    if (group->isRaidGroup())
        botAI->SayToRaid(msg);
    else
        botAI->SayToParty(msg);

    return true;
}

bool SetMasterAction::isUseful()
{
    if (!bot || !botAI)
        return false;

    Player* master = GetMaster();
    if (!master)
        return false;

    return true;
}
