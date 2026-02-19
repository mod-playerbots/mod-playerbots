#ifndef _PLAYERBOT_WORLDNAVIGATIONMGR_H
#define _PLAYERBOT_WORLDNAVIGATIONMGR_H

#include "Creature.h"
#include "Player.h"
#include "TravelMgr.h"

enum class CityId : uint8
{
    STORMWIND,
    IRONFORGE,
    DARNASSUS,
    EXODAR,
    ORGRIMMAR,
    UNDERCITY,
    THUNDER_BLUFF,
    SILVERMOON_CITY,
    SHATTRATH_CITY,
    DALARAN
};

// Map of banker entry → city + faction
static const std::unordered_map<uint16, std::pair<CityId, TeamId>> bankerToCity = {
    {2455,  {CityId::STORMWIND,       TEAM_ALLIANCE}}, {2456,  {CityId::STORMWIND,       TEAM_ALLIANCE}}, {2457,  {CityId::STORMWIND,       TEAM_ALLIANCE}},
    {2460,  {CityId::IRONFORGE,       TEAM_ALLIANCE}}, {2461,  {CityId::IRONFORGE,       TEAM_ALLIANCE}}, {5099,  {CityId::IRONFORGE,       TEAM_ALLIANCE}},
    {4155,  {CityId::DARNASSUS,       TEAM_ALLIANCE}}, {4208,  {CityId::DARNASSUS,       TEAM_ALLIANCE}}, {4209,  {CityId::DARNASSUS,       TEAM_ALLIANCE}},
    {17773, {CityId::EXODAR,          TEAM_ALLIANCE}}, {18350, {CityId::EXODAR,          TEAM_ALLIANCE}}, {16710, {CityId::EXODAR,          TEAM_ALLIANCE}},
    {3320,  {CityId::ORGRIMMAR,       TEAM_HORDE}},    {3309,  {CityId::ORGRIMMAR,       TEAM_HORDE}},    {3318,  {CityId::ORGRIMMAR,       TEAM_HORDE}},
    {4549,  {CityId::UNDERCITY,       TEAM_HORDE}},    {2459,  {CityId::UNDERCITY,       TEAM_HORDE}},    {2458,  {CityId::UNDERCITY,       TEAM_HORDE}},    {4550, {CityId::UNDERCITY, TEAM_HORDE}},
    {2996,  {CityId::THUNDER_BLUFF,   TEAM_HORDE}},    {8356,  {CityId::THUNDER_BLUFF,   TEAM_HORDE}},    {8357,  {CityId::THUNDER_BLUFF,   TEAM_HORDE}},
    {17631, {CityId::SILVERMOON_CITY, TEAM_HORDE}},    {17632, {CityId::SILVERMOON_CITY, TEAM_HORDE}},    {17633, {CityId::SILVERMOON_CITY, TEAM_HORDE}},
    {16615, {CityId::SILVERMOON_CITY, TEAM_HORDE}},    {16616, {CityId::SILVERMOON_CITY, TEAM_HORDE}},    {16617, {CityId::SILVERMOON_CITY, TEAM_HORDE}},
    {19246, {CityId::SHATTRATH_CITY,  TEAM_NEUTRAL}},  {19338, {CityId::SHATTRATH_CITY,  TEAM_NEUTRAL}},
    {19034, {CityId::SHATTRATH_CITY,  TEAM_NEUTRAL}},  {19318, {CityId::SHATTRATH_CITY,  TEAM_NEUTRAL}},
    {30604, {CityId::DALARAN,         TEAM_NEUTRAL}},  {30605, {CityId::DALARAN,         TEAM_NEUTRAL}},  {30607, {CityId::DALARAN,         TEAM_NEUTRAL}},
    {28675, {CityId::DALARAN,         TEAM_NEUTRAL}},  {28676, {CityId::DALARAN,         TEAM_NEUTRAL}},  {28677, {CityId::DALARAN,         TEAM_NEUTRAL}}
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
    static WorldNavigationMgr& Instance()
    {
        static WorldNavigationMgr instance;
        return instance;
    }

    void Init();

    Creature* GetNearestFlightMaster(Player* bot);
    ObjectGuid GetNearestFlightMasterGuid(Player* bot);

    std::vector<std::vector<uint32>> GetOptimalFlightDestinations(Player* bot);
    const std::vector<WorldLocation> GetTeleportLocations(Player* bot);
    const std::vector<WorldLocation> GetTravelHubs(Player* bot);
    std::vector<WorldLocation> GetCityLocations(Player* bot);

    std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;
private:

    WorldNavigationMgr() = default;
    ~WorldNavigationMgr() = default;

    WorldNavigationMgr(const WorldNavigationMgr&) = delete;
    WorldNavigationMgr& operator=(const WorldNavigationMgr&) = delete;
    WorldNavigationMgr(WorldNavigationMgr&&) = delete;
    WorldNavigationMgr& operator=(WorldNavigationMgr&&) = delete;

    // Initialization
    void PrepareZone2LevelBracket();
    void PrepareDestinationCache();

    //Taxi Path grapg
    void BuildTaxiGraph();
    void ComputeAllPaths();
    std::unordered_map<uint32, uint32> BFS(uint32 startNode);
    std::vector<uint32> BuildPath(uint32 fromNode, uint32 toNode,
                              const std::unordered_map<uint32, uint32>& parentMap);
    // City Weights
    int GetCityWeight(CityId city);

    //class types needed
    struct LevelBracket
    {
        uint32 low;
        uint32 high;
        bool InsideBracket(uint32 val) const { return val >= low && val <= high; }
    };

    struct BankerLocation
    {
        WorldLocation loc;
        uint32 entry;
    };

    struct CreatureSpawnInfo
    {
        uint32 spawnId;
        uint16 mapId;
        float posX;
        float posY;
        float posZ;
        uint32 areaId;
    };

    struct CreatureLocationCluster
    {
        uint32 areaId;
        float avgX;
        float avgY;
        float avgZ;
        std::vector<CreatureSpawnInfo> spawns;
    };

    struct GrindingSpot
    {
        WorldLocation loc;
        uint32 minLevel;
        uint32 maxLevel;
    };

    //caches
    //Flight master caches
    std::map<uint32, WorldPosition> allianceFlightMasterCache;
    std::map<uint32, WorldPosition> hordeFlightMasterCache;
    //Travel hubs (Starting zones and innkeepers)
    std::map<uint8, std::vector<WorldLocation>> allianceHubsPerLevelCache;
    std::map<uint8, std::vector<WorldLocation>> hordeHubsPerLevelCache;
    //Bankers
    std::map<uint8, std::vector<BankerLocation>> bankerLocsPerLevelCache;
    static inline std::unordered_map<uint32, WorldLocation> bankerEntryToLocation;
    // Areas to Grind
    std::map<uint32, std::vector<GrindingSpot>> grindingSpotsByArea;
    std::map<uint8,  std::vector<GrindingSpot>> grindingSpotsByLevel;
    // TODO Clusters of creatures, to enable searching for specific creatures
    std::unordered_map<uint32, std::vector<CreatureLocationCluster>> creatureSpawnsByTemplate;
    // Zone level bracket lookup
    std::map<uint32, LevelBracket> zone2LevelBracket;
    //taxi paths
    std::map<uint32, std::map<uint32, std::vector<uint32>>> taxiPathCache;
    std::unordered_map<uint32, std::vector<uint32>> taxiGraph;
};

#define sWorldNavigationMgr WorldNavigationMgr::Instance()
#endif
