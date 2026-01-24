/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TravelManagerAdapter.h"

#include "RandomPlayerbotMgr.h"
#include "TravelMgr.h"

void TravelManagerAdapter::RandomTeleport(Player* bot)
{
    // RandomPlayerbotMgr handles random teleportation
    // Note: The actual RandomTeleport method is private in RandomPlayerbotMgr
    // Using RandomTeleportForLevel as the public alternative
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
}

void TravelManagerAdapter::RandomTeleportForLevel(Player* bot)
{
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
}

void TravelManagerAdapter::RandomTeleportForRpg(Player* bot)
{
    sRandomPlayerbotMgr->RandomTeleportForRpg(bot);
}

uint32 TravelManagerAdapter::GetZoneLevel(uint16 /*mapId*/, float /*x*/, float /*y*/, float /*z*/)
{
    // This would need access to RandomPlayerbotMgr's private GetZoneLevel method
    // For now, return 0 - this can be expanded when the method is made accessible
    return 0;
}

std::vector<WorldLocation> TravelManagerAdapter::GetLocsForLevel(uint8 level)
{
    // Access the cached locations from RandomPlayerbotMgr
    auto const& cache = sRandomPlayerbotMgr->locsPerLevelCache;
    auto it = cache.find(level);
    if (it != cache.end())
    {
        return it->second;
    }
    return {};
}

bool TravelManagerAdapter::HasDestination(Player* bot)
{
    // TravelMgr doesn't have a direct "has destination" check
    // This would need to check the bot's travel target
    // For now, return false - this can be expanded based on usage
    (void)bot;  // Suppress unused parameter warning
    return false;
}

void TravelManagerAdapter::SetQuestDestination(Player* bot, Quest const* quest)
{
    // TravelMgr provides getQuestTravelDestinations but doesn't set them directly
    // This would need to interact with the bot's AI to set the travel target
    (void)bot;
    (void)quest;
}

void TravelManagerAdapter::ClearDestination(Player* bot)
{
    // Clear the bot's travel destination
    sTravelMgr->setNullTravelTarget(bot);
}
