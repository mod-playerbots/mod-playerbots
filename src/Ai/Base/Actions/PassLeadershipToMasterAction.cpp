/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PassLeadershipToMasterAction.h"

#include "Event.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"

bool PassLeadershipToMasterAction::Execute(Event)
{
    const Player* const master = this->GetMaster();

    if (master == nullptr || master == this->bot)
    {
        return false;
    }

    const Group* const group = this->bot->GetGroup();

    if (group == nullptr || !this->bot->GetGroup()->IsMember(master->GetGUID()))
    {
        return false;
    }

    std::unique_ptr<GroupSetLeaderOperation> setLeaderOp = std::make_unique<GroupSetLeaderOperation>(this->bot->GetGUID(), master->GetGUID());
    PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(setLeaderOp));

    if (!message.empty())
    {
        this->botAI->TellMasterNoFacing(message);
    }

    if (RandomPlayerbotMgr::instance().IsRandomBot(this->bot))
    {
        this->botAI->ResetStrategies();
        this->botAI->Reset();
    }

    return true;
}

bool PassLeadershipToMasterAction::isUseful()
{
    return this->botAI->IsAlt() && this->bot->GetGroup() && this->bot->GetGroup()->IsLeader(this->bot->GetGUID());
}

bool GiveLeaderAction::isUseful()
{
    return this->botAI->HasActivePlayerMaster() && this->bot->GetGroup() && this->bot->GetGroup()->IsLeader(this->bot->GetGUID());
}
