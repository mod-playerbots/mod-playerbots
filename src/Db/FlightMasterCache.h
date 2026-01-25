#ifndef _PLAYERBOT_FLIGHTMASTER_H
#define _PLAYERBOT_FLIGHTMASTER_H

#include "Creature.h"
#include "Player.h"
#include "TravelMgr.h"

class FlightMasterCache
{
public:
    static FlightMasterCache* Instance()
    {
        static FlightMasterCache instance;
        return &instance;
    }

    Creature* GetNearestFlightMaster(Player* bot);
    void AddHordeFlightMaster(uint32 entry, WorldPosition pos);
    void AddAllianceFlightMaster(uint32 entry, WorldPosition pos);

private:
    std::map<uint32, WorldPosition> allianceFlightMasterCache;
    std::map<uint32, WorldPosition> hordeFlightMasterCache;
};

#define sFlightMasterCache FlightMasterCache::Instance()
#endif
