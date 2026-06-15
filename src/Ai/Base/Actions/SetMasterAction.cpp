/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetMasterAction.h"

#include "Log.h"
#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "BroadcastHelper.h"

bool SetMasterAction::Execute(Event event)
{
    Player* newMaster = event.getOwner();
    Group* group = bot->GetGroup();

    LOG_INFO("playerbots",
        "SetMaster: bot={}, owner={}, master={}, group={}, ownerName={}, masterName={}",
        bot->GetName(),
        newMaster ? newMaster->GetName() : "nullptr",
        GetMaster() ? GetMaster()->GetName() : "nullptr",
        group ? "yes" : "no",
        newMaster ? (newMaster == bot ? "self" : newMaster->GetName()) : "nullptr",
        GetMaster() ? (GetMaster() == bot ? "self" : GetMaster()->GetName()) : "nullptr");

    if (!newMaster)
    {
        LOG_INFO("playerbots", "SetMaster: failed - owner is nullptr");
        return false;
    }

    if (!group || !group->IsMember(newMaster->GetGUID()))
    {
        LOG_INFO("playerbots", "SetMaster: failed - no group or owner not member");
        return false;
    }

    botAI->SetMaster(newMaster);
    botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);

    std::string msg = "Now following " + std::string(newMaster->GetName()) + ".";
    if (group->isRaidGroup())
        botAI->SayToRaid(msg);
    else
        botAI->SayToParty(msg);

    LOG_INFO("playerbots", "SetMaster: success - now following {}", newMaster->GetName());
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
