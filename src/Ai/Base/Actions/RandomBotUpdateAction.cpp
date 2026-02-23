/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RandomBotUpdateAction.h"

#include "Event.h"
#include "Playerbots.h"

bool RandomBotUpdateAction::Execute(Event /*event*/)
{
    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    if (bot->GetGroup() && botAI->GetGroupLeader())
    {
        PlayerbotAI* groupLeaderBotAI = GET_PLAYERBOT_AI(botAI->GetGroupLeader());
        if (!groupLeaderBotAI || groupLeaderBotAI->IsRealPlayer())
            return true;
    }

    if (botAI->HasPlayerNearby(sPlayerbotAIConfig.grindDistance))
        return true;

    return sRandomPlayerbotMgr.ProcessBot(bot);
}

bool RandomBotUpdateAction::isUseful() { return AI_VALUE(bool, "random bot update"); }
