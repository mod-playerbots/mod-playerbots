/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVEL_MANAGER_ADAPTER_H
#define _PLAYERBOT_TRAVEL_MANAGER_ADAPTER_H

#include "Bot/Interface/ITravelManager.h"

/**
 * @brief Adapter that wraps TravelMgr and RandomPlayerbotMgr behind ITravelManager interface
 *
 * This adapter allows code to use the ITravelManager interface while
 * delegating to the existing TravelMgr and RandomPlayerbotMgr singletons.
 *
 * Note: Some travel operations are split between TravelMgr and RandomPlayerbotMgr
 * in the current architecture. This adapter unifies them behind a single interface.
 */
class TravelManagerAdapter : public ITravelManager
{
public:
    TravelManagerAdapter() = default;
    ~TravelManagerAdapter() override = default;

    // Teleportation (delegates to RandomPlayerbotMgr)
    void RandomTeleport(Player* bot) override;
    void RandomTeleportForLevel(Player* bot) override;
    void RandomTeleportForRpg(Player* bot) override;

    // Location queries
    uint32 GetZoneLevel(uint16 mapId, float x, float y, float z) override;
    std::vector<WorldLocation> GetLocsForLevel(uint8 level) override;

    // Quest travel (delegates to TravelMgr)
    bool HasDestination(Player* bot) override;
    void SetQuestDestination(Player* bot, Quest const* quest) override;
    void ClearDestination(Player* bot) override;
};

#endif
