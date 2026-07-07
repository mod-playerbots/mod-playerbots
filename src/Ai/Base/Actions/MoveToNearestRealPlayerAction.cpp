/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MoveToNearestRealPlayerAction.h"

#include "Playerbots.h"

Player* MoveToNearestRealPlayerAction::GetPursueTarget()
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "nearest real player in zone");
    if (guid.IsEmpty())
        return nullptr;

    return botAI->GetPlayer(guid);
}

bool MoveToNearestRealPlayerAction::Execute(Event /*event*/)
{
    Player* target = GetPursueTarget();
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig.syncBotsWithPlayerReachDistance);
}

bool MoveToNearestRealPlayerAction::isUseful()
{
    if (!sRandomPlayerbotMgr.IsWorldPvpBot(bot->GetGUID().GetCounter()))
        return false;

    if (bot->IsInCombat())
        return false;

    if (AI_VALUE(Unit*, "enemy player target"))
        return false;

    Player* target = GetPursueTarget();
    if (!target)
        return false;

    return bot->GetDistance(target) > sPlayerbotAIConfig.syncBotsWithPlayerReachDistance;
}
