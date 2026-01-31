#ifndef _PLAYERBOT_FLIGHTMASTER_H
#define _PLAYERBOT_FLIGHTMASTER_H

#include "Creature.h"
#include "Player.h"
#include "TravelMgr.h"

class FlightMasterCache
{
public:
    static FlightMasterCache& Instance()
    {
        static FlightMasterCache instance;

        return instance;
    }

    Creature* GetNearestFlightMaster(Player* bot);
    void AddHordeFlightMaster(uint32 entry, WorldPosition pos);
    void AddAllianceFlightMaster(uint32 entry, WorldPosition pos);

private:
    FlightMasterCache() = default;
    ~FlightMasterCache() = default;

    FlightMasterCache(const FlightMasterCache&) = delete;
    FlightMasterCache& operator=(const FlightMasterCache&) = delete;

    FlightMasterCache(FlightMasterCache&&) = delete;
    FlightMasterCache& operator=(FlightMasterCache&&) = delete;

    std::map<uint32, WorldPosition> allianceFlightMasterCache;
    std::map<uint32, WorldPosition> hordeFlightMasterCache;
};

#endif
