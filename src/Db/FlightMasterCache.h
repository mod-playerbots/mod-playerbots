/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FLIGHTMASTERCACHE_H
#define _PLAYERBOT_FLIGHTMASTERCACHE_H

#include <vector>

#include "Common.h"

class FlightMasterCache
{
public:
    FlightMasterCache() {}
    virtual ~FlightMasterCache() {}
    static FlightMasterCache* instance()
    {
        static FlightMasterCache instance;
        return &instance;
    }

    void AddFlightMaster(uint32 guid, bool forHorde, bool forAlliance);
    std::vector<uint32> const& GetAllianceFlightMasters() const { return allianceFlightMasterCache; }
    std::vector<uint32> const& GetHordeFlightMasters() const { return hordeFlightMasterCache; }
    void Clear();

private:
    std::vector<uint32> allianceFlightMasterCache;
    std::vector<uint32> hordeFlightMasterCache;
};

#define sFlightMasterCache FlightMasterCache::instance()

#endif
