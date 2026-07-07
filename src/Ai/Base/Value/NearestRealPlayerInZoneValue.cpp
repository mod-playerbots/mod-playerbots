/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NearestRealPlayerInZoneValue.h"

#include "Playerbots.h"

ObjectGuid NearestRealPlayerInZoneValue::Calculate()
{
    uint32 zoneId = sRandomPlayerbotMgr.GetWorldPvpBotZoneId(bot->GetGUID().GetCounter());
    if (!zoneId)
        return ObjectGuid::Empty;

    Player* nearest = nullptr;
    float nearestDistance = 0.0f;
    for (Player* player : sRandomPlayerbotMgr.GetPlayers())
    {
        if (!player->IsInWorld() || player->GetZoneId() != zoneId)
            continue;

        float distance = bot->GetDistance(player);
        if (!nearest || distance < nearestDistance)
        {
            nearest = player;
            nearestDistance = distance;
        }
    }

    return nearest ? nearest->GetGUID() : ObjectGuid::Empty;
}
