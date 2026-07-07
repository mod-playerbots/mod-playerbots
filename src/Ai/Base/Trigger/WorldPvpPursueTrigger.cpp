/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPvpPursueTrigger.h"

#include "Playerbots.h"

bool WorldPvpPursueTrigger::IsActive()
{
    if (!sRandomPlayerbotMgr.IsWorldPvpBot(bot->GetGUID().GetCounter()))
        return false;

    if (bot->IsInCombat())
        return false;

    if (AI_VALUE(Unit*, "enemy player target"))
        return false;

    ObjectGuid guid = AI_VALUE(ObjectGuid, "nearest real player in zone");
    if (guid.IsEmpty())
        return false;

    Player* target = botAI->GetPlayer(guid);
    if (!target)
        return false;

    return bot->GetDistance(target) > sPlayerbotAIConfig.syncBotsWithPlayerReachDistance;
}
