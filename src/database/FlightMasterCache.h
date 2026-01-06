#ifndef _PlAYERBOT_FLIGHTMASTER_H
#define _PlAYERBOT_FLIGHTMASTER_H

#include "Creature.h"
#include "Player.h"

class FlightMasterCache
{
public:
    static FlightMasterCache* Instance()
    {
        static FlightMasterCache instance;
        return &instance;
    }

    void Initialize(); // call once on startup
    Creature* GetNearestFlightMaster(Player* bot);
    void AddHordeFlightMaster(uint32 entry, WorldPosition pos);
    void AddAllianceFlightMaster(uint32 entry, WorldPosition pos);

private:
    std::map<uint32, WorldPosition> allianceFlightMasterCache;
    std::map<uint32, WorldPosition> hordeFlightMasterCache;
};

#define sFlightMaster FlightMasterCache::Instance()
#endif