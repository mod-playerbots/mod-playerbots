/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PassLeadershipToMasterAction.h"

#include "Event.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"

bool PassLeadershipToMasterAction::Execute(Event /*event*/)
{
    if (Player* master = GetMaster())
        if (master && master != bot && bot->GetGroup() && bot->GetGroup()->IsMember(master->GetGUID()))
        {
            auto setLeaderOp = std::make_unique<GroupSetLeaderOperation>(bot->GetGUID(), master->GetGUID());
            PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(setLeaderOp));

            if (!message.empty())
                botAI->TellMasterNoFacing(message);

            if (sRandomPlayerbotMgr.IsRandomBot(bot))
            {
                botAI->ResetStrategies();
                botAI->Reset();
            }

            return true;
        }

    return false;
}

bool PassLeadershipToMasterAction::isUseful()
{
    return botAI->IsAlt() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID());
}

bool GiveLeaderAction::isUseful()
{
    return bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID());
}

bool GiveLeaderAction::Execute(Event event)
{
    Player* target = event.getOwner();
    if (!target || target == bot || !bot->GetGroup() || !bot->GetGroup()->IsMember(target->GetGUID()))
        return false;

    auto setLeaderOp = std::make_unique<GroupSetLeaderOperation>(bot->GetGUID(), target->GetGUID());
    PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(setLeaderOp));

    if (botAI->GetMaster() == bot)
    {
        botAI->SetMaster(target);
        botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
    }

    if (!message.empty())
        botAI->TellMasterNoFacing(message);

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        botAI->ResetStrategies();
        botAI->Reset();
    }

    return true;
}
