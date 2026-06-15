/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetMasterAction.h"

#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "BroadcastHelper.h"

bool SetMasterAction::Execute(Event event)
{
    Player* newMaster = event.getOwner();
    Group* group = bot->GetGroup();

    if (!group)
        return false;

    if (!newMaster || newMaster == bot)
    {
        newMaster = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
    }

    if (!newMaster || newMaster == bot || !group->IsMember(newMaster->GetGUID()))
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
