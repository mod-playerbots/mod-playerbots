/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FlightMasterCache.h"

void FlightMasterCache::AddFlightMaster(uint32 guid, bool forHorde, bool forAlliance)
{
    if (forHorde)
    {
        hordeFlightMasterCache.push_back(guid);
    }
    if (forAlliance)
    {
        allianceFlightMasterCache.push_back(guid);
    }
}

void FlightMasterCache::Clear()
{
    allianceFlightMasterCache.clear();
    hordeFlightMasterCache.clear();
}
