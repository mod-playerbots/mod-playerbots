#include "FlightMasterCache.h"

void FlightMasterCache::AddHordeFlightMaster(uint32 entry, WorldPosition pos)
{
    hordeFlightMasterCache[entry] = pos;
}

void FlightMasterCache::AddAllianceFlightMaster(uint32 entry, WorldPosition pos)
{
    allianceFlightMasterCache[entry] = pos;
}

Creature* FlightMasterCache::GetNearestFlightMaster(Player* bot)
{
    std::map<uint32, WorldPosition>& flightMasterCache =
        (bot->GetTeamId() == ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    Creature* nearestFlightMaster = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto const& [entry, pos] : flightMasterCache)
    {
        float distance = bot->GetDistance(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        if (distance < 500.0f && distance < nearestDistance)
        {
            Creature* flightMaster = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), entry);
            if (flightMaster)
            {
                nearestDistance = distance;
                nearestFlightMaster = flightMaster;
            }
        }
    }

    return nearestFlightMaster;
}