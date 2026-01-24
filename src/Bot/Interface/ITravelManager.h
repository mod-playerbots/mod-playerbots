/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITRAVEL_MANAGER_H
#define _PLAYERBOT_ITRAVEL_MANAGER_H

#include "Common.h"
#include <vector>

class Player;
class WorldLocation;
class WorldPosition;
class Quest;

/**
 * @brief Interface for travel and navigation management
 *
 * This interface abstracts travel-related operations,
 * enabling isolated testing and loose coupling.
 */
class ITravelManager
{
public:
    virtual ~ITravelManager() = default;

    // Teleportation
    virtual void RandomTeleport(Player* bot) = 0;
    virtual void RandomTeleportForLevel(Player* bot) = 0;
    virtual void RandomTeleportForRpg(Player* bot) = 0;

    // Location queries
    virtual uint32 GetZoneLevel(uint16 mapId, float x, float y, float z) = 0;
    virtual std::vector<WorldLocation> GetLocsForLevel(uint8 level) = 0;

    // Quest travel
    virtual bool HasDestination(Player* bot) = 0;
    virtual void SetQuestDestination(Player* bot, Quest const* quest) = 0;
    virtual void ClearDestination(Player* bot) = 0;
};

#endif
