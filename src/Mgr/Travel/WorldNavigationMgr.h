#ifndef _PLAYERBOT_WORLDNAVIGATIONMGR_H
#define _PLAYERBOT_WORLDNAVIGATIONMGR_H

#include "Creature.h"
#include "Player.h"
#include "TravelMgr.h"

enum class CityId : uint8 {
    STORMWIND, IRONFORGE, DARNASSUS, EXODAR,
    ORGRIMMAR, UNDERCITY, THUNDER_BLUFF, SILVERMOON_CITY,
    SHATTRATH_CITY, DALARAN
};

enum class FactionId : uint8 { ALLIANCE, HORDE, NEUTRAL };

// Map of banker entry → city + faction
static const std::unordered_map<uint16, std::pair<CityId, FactionId>> bankerToCity = {
    {2455,  {CityId::STORMWIND,       FactionId::ALLIANCE}}, {2456,  {CityId::STORMWIND,       FactionId::ALLIANCE}}, {2457,  {CityId::STORMWIND,       FactionId::ALLIANCE}},
    {2460,  {CityId::IRONFORGE,       FactionId::ALLIANCE}}, {2461,  {CityId::IRONFORGE,       FactionId::ALLIANCE}}, {5099,  {CityId::IRONFORGE,       FactionId::ALLIANCE}},
    {4155,  {CityId::DARNASSUS,       FactionId::ALLIANCE}}, {4208,  {CityId::DARNASSUS,       FactionId::ALLIANCE}}, {4209,  {CityId::DARNASSUS,       FactionId::ALLIANCE}},
    {17773, {CityId::EXODAR,          FactionId::ALLIANCE}}, {18350, {CityId::EXODAR,          FactionId::ALLIANCE}}, {16710, {CityId::EXODAR,          FactionId::ALLIANCE}},
    {3320,  {CityId::ORGRIMMAR,       FactionId::HORDE}},    {3309,  {CityId::ORGRIMMAR,       FactionId::HORDE}},    {3318,  {CityId::ORGRIMMAR,       FactionId::HORDE}},
    {4549,  {CityId::UNDERCITY,       FactionId::HORDE}},    {2459,  {CityId::UNDERCITY,       FactionId::HORDE}},    {2458,  {CityId::UNDERCITY,       FactionId::HORDE}},    {4550, {CityId::UNDERCITY, FactionId::HORDE}},
    {2996,  {CityId::THUNDER_BLUFF,   FactionId::HORDE}},    {8356,  {CityId::THUNDER_BLUFF,   FactionId::HORDE}},    {8357,  {CityId::THUNDER_BLUFF,   FactionId::HORDE}},
    {17631, {CityId::SILVERMOON_CITY, FactionId::HORDE}},    {17632, {CityId::SILVERMOON_CITY, FactionId::HORDE}},    {17633, {CityId::SILVERMOON_CITY, FactionId::HORDE}},
    {16615, {CityId::SILVERMOON_CITY, FactionId::HORDE}},    {16616, {CityId::SILVERMOON_CITY, FactionId::HORDE}},    {16617, {CityId::SILVERMOON_CITY, FactionId::HORDE}},
    {19246, {CityId::SHATTRATH_CITY,  FactionId::NEUTRAL}},  {19338, {CityId::SHATTRATH_CITY,  FactionId::NEUTRAL}},
    {19034, {CityId::SHATTRATH_CITY,  FactionId::NEUTRAL}},  {19318, {CityId::SHATTRATH_CITY,  FactionId::NEUTRAL}},
    {30604, {CityId::DALARAN,         FactionId::NEUTRAL}},  {30605, {CityId::DALARAN,         FactionId::NEUTRAL}},  {30607, {CityId::DALARAN,         FactionId::NEUTRAL}},
    {28675, {CityId::DALARAN,         FactionId::NEUTRAL}},  {28676, {CityId::DALARAN,         FactionId::NEUTRAL}},  {28677, {CityId::DALARAN,         FactionId::NEUTRAL}}
};

// Map of city → available banker entries
static const std::unordered_map<CityId, std::vector<uint16>> cityToBankers = {
    {CityId::STORMWIND,       {2455, 2456, 2457}},
    {CityId::IRONFORGE,       {2460, 2461, 5099}},
    {CityId::DARNASSUS,       {4155, 4208, 4209}},
    {CityId::EXODAR,          {17773, 18350, 16710}},
    {CityId::ORGRIMMAR,       {3320, 3309, 3318}},
    {CityId::UNDERCITY,       {4549, 2459, 2458, 4550}},
    {CityId::THUNDER_BLUFF,   {2996, 8356, 8357}},
    {CityId::SILVERMOON_CITY, {17631, 17632, 17633, 16615, 16616, 16617}},
    {CityId::SHATTRATH_CITY,  {19246, 19338, 19034, 19318}},
    {CityId::DALARAN,         {30604, 30605, 30607, 28675, 28676, 28677, 29530}}
};

class WorldNavigationMgr
{
public:
    static WorldNavigationMgr* Instance()
    {
        static WorldNavigationMgr instance;
        return &instance;
    }

    void Init();

    ObjectGuid GetNearestFlightMaster(Player* bot, bool returnGuid);
    Creature* GetNearestFlightMaster(Player* bot);

    std::vector<std::vector<uint32>> GetOptimalDestinations(Player* bot);
    void PrepareZone2LevelBracket();
    void PrepareTeleportCache();
    const std::vector<WorldLocation> GetTeleportLocations(Player* bot);
    const std::vector<WorldLocation> GetTravelHubs(Player* bot);
    std::vector<WorldLocation> GetCityLocations(Player* bot);

    std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;
private:

    //Taxi Paths
    std::vector<uint32> FindTaxiPath(uint32 fromNode, uint32 toNode);
    void BuildTaxiGraph();
    void ComputeAllPaths();
    std::unordered_map<uint32, uint32> BFS(uint32 start);
    std::vector<uint32> BuildPath(uint32 from, uint32 to,
                              const std::unordered_map<uint32, uint32>& parent);
    //flight master
    std::map<uint32, WorldPosition> allianceFlightMasterCache;
    std::map<uint32, WorldPosition> hordeFlightMasterCache;

    std::map<uint8, std::vector<WorldLocation>> allianceHubsPerLevelCache;
    std::map<uint8, std::vector<WorldLocation>> hordeHubsPerLevelCache;

    int GetCityWeight(CityId city);

    struct LevelBracket
    {
        uint32 low;
        uint32 high;
        bool InsideBracket(uint32 val) { return val >= low && val <= high; }
    };
    std::map<uint32, LevelBracket> zone2LevelBracket;

    struct BankerLocation
    {
        WorldLocation loc;
        uint32 entry;
    };
    std::map<uint8, std::vector<BankerLocation>> bankerLocsPerLevelCache;
    static inline std::unordered_map<uint32, WorldLocation> bankerEntryToLocation;

    //taxi paths
    std::map<uint32, std::map<uint32, std::vector<uint32>>> taxiPathCache;
    std::unordered_map<uint32, std::vector<uint32>> taxiGraph;
};

#define sWorldNavigationMgr WorldNavigationMgr::Instance()
#endif
