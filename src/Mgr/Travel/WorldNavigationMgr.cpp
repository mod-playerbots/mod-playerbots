#include "WorldNavigationMgr.h"
#include <vector>
#include "Log.h"
#include "MapMgr.h"
#include "PlayerbotAIConfig.h"

void WorldNavigationMgr::Init()
{
    if (sPlayerbotAIConfig.enabled)
    {
        PrepareZone2LevelBracket();
        PrepareDestinationCache();
    }
    BuildTaxiGraph();
    ComputeAllPaths();
    LOG_INFO("playerbots", "Playerbots Taxi graph and destination cache built.");
}

Creature* WorldNavigationMgr::GetNearestFlightMaster(Player* bot)
{
    std::map<uint32, WorldPosition>& flightMasterCache =
        (bot->GetTeamId() == TEAM_ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    Creature* nearestFlightMaster = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto const& [entry, pos] : flightMasterCache)
    {
        if (pos.GetMapId() != bot->GetMapId())
            continue;

        float distance = bot->GetExactDist2dSq(pos);
        if (distance > nearestDistance)
            continue;

        Creature* flightMaster = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), entry);
        if (flightMaster)
        {
            nearestDistance = distance;
            nearestFlightMaster = flightMaster;
        }
    }

    return nearestFlightMaster;
}

ObjectGuid WorldNavigationMgr::GetNearestFlightMasterGuid(Player* bot)
{
    Creature* nearestFlightMaster = GetNearestFlightMaster(bot);
    if (!nearestFlightMaster)
        return ObjectGuid::Empty;

    return nearestFlightMaster->GetGUID();
}

std::vector<uint32> WorldNavigationMgr::FindTaxiPath(uint32 fromNode, uint32 toNode)
{
    if (fromNode == toNode)
        return {};

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    TaxiNodesEntry const* endNode = sTaxiNodesStore.LookupEntry(toNode);

    if (!startNode || !endNode || startNode->map_id != endNode->map_id)
        return {};

    auto cacheItr = taxiPathCache.find(fromNode);
    if (cacheItr == taxiPathCache.end())
        return {};

    auto toNodeItr = cacheItr->second.find(toNode);
    if (toNodeItr == cacheItr->second.end())
        return {};

    return toNodeItr->second;
}

std::vector<std::vector<uint32>> WorldNavigationMgr::GetOptimalFlightDestinations(Player* bot)
{
    std::vector<std::vector<uint32>> validDestinations;

    Creature* nearestFlightMaster = GetNearestFlightMaster(bot);
    if (!nearestFlightMaster || bot->GetDistance(nearestFlightMaster) > 500.0f)
        return validDestinations;

    uint32 fromNode = sObjectMgr->GetNearestTaxiNode(nearestFlightMaster->GetPositionX(), nearestFlightMaster->GetPositionY(),
                                            nearestFlightMaster->GetPositionZ(), nearestFlightMaster->GetMapId(),
                                            bot->GetTeamId());
    if (!fromNode)
        return validDestinations;
    std::vector<WorldLocation> candidateLocations;
    if (bot->GetLevel() >= 10 && urand(0, 100) < sPlayerbotAIConfig.probTeleToBankers * 100)
        candidateLocations = GetCityLocations(bot);

    std::vector<WorldLocation> hubLocations = GetTravelHubs(bot);
    candidateLocations.insert(candidateLocations.end(), hubLocations.begin(), hubLocations.end());

    for (auto const& loc : candidateLocations)
    {
        uint32 candidateNode = sObjectMgr->GetNearestTaxiNode(loc.GetPositionX(), loc.GetPositionY(),
                                            loc.GetPositionZ(), loc.GetMapId(),
                                            bot->GetTeamId());
        if (!candidateNode)
            continue;

        std::vector<uint32> path = FindTaxiPath(fromNode, candidateNode);
        if (!path.empty())
            validDestinations.push_back(path);
    }
    return validDestinations;
}

const std::vector<WorldLocation> WorldNavigationMgr::GetTeleportLocations(Player* bot)
{
    uint32 level = bot->GetLevel();
    uint8 isAlliance = bot->GetTeamId() == TEAM_ALLIANCE;
    if (sPlayerbotAIConfig.enableNewRpgStrategy)
        return isAlliance ? allianceHubsPerLevelCache[level] : hordeHubsPerLevelCache[level];

    return locsPerLevelCache[level];
}

const std::vector<WorldLocation> WorldNavigationMgr::GetTravelHubs(Player* bot)
{
    std::vector<WorldLocation> locs = bot->GetTeamId() == TEAM_ALLIANCE
                                                 ? allianceHubsPerLevelCache[bot->GetLevel()]
                                                 : hordeHubsPerLevelCache[bot->GetLevel()];
    return locs;
}

std::vector<WorldLocation> WorldNavigationMgr::GetCityLocations(Player* bot)
{
    uint32 level = bot->GetLevel();

    std::vector<WorldLocation> fallbackLocations;
    for (auto& bLoc : bankerLocsPerLevelCache[level])
        fallbackLocations.push_back(bLoc.loc);

    if (!sPlayerbotAIConfig.enableWeightTeleToCityBankers)
        return fallbackLocations;

    TeamId botTeamId = bot->GetTeamId();
    std::unordered_set<CityId> validBankerCities;
    for (auto& loc : bankerLocsPerLevelCache[level])
    {
        auto cityIt = bankerToCity.find(loc.entry);
        if (cityIt == bankerToCity.end())
            continue;

        TeamId cityTeamId = cityIt->second.second;

        if (cityTeamId == botTeamId ||
            (cityTeamId == TEAM_NEUTRAL)
           )
            validBankerCities.insert(cityIt->second.first);
    }
    // Fallback if no valid cities
    if (validBankerCities.empty())
        return fallbackLocations;

    // Apply weights to valid cities
    std::vector<CityId> weightedCities;
    for (CityId city : validBankerCities)
    {
        int weight = GetCityWeight(city);
        if (weight <= 0)
            continue;

        for (int i = 0; i < weight; ++i)
            weightedCities.push_back(city);
    }

    // Fallback if no valid cities
    if (weightedCities.empty())
        return fallbackLocations;

    // Pick a weighted city randomly, then a random banker in that city
    //   then teleport to that banker
    CityId selectedCity = weightedCities[urand(0, weightedCities.size() - 1)];

    auto const& bankers = cityToBankers.at(selectedCity);
    uint32 selectedBankerEntry = bankers[urand(0, bankers.size() - 1)];
    auto locIt = bankerEntryToLocation.find(selectedBankerEntry);
    if (locIt != bankerEntryToLocation.end())
        return { locIt->second };
    // Fallback if something went wrong
    return fallbackLocations;
}

void WorldNavigationMgr::PrepareZone2LevelBracket()
{
    // Classic WoW - Low - level zones
    zone2LevelBracket[1] = {5, 12};     // Dun Morogh
    zone2LevelBracket[12] = {5, 12};    // Elwynn Forest
    zone2LevelBracket[14] = {5, 12};    // Durotar
    zone2LevelBracket[85] = {5, 12};    // Tirisfal Glades
    zone2LevelBracket[141] = {5, 12};   // Teldrassil
    zone2LevelBracket[215] = {5, 12};   // Mulgore
    zone2LevelBracket[3430] = {5, 12};  // Eversong Woods
    zone2LevelBracket[3524] = {5, 12};  // Azuremyst Isle

    // Classic WoW - Mid - level zones
    zone2LevelBracket[17] = {10, 25};    // Barrens
    zone2LevelBracket[38] = {10, 20};    // Loch Modan
    zone2LevelBracket[40] = {10, 21};    // Westfall
    zone2LevelBracket[130] = {10, 23};   // Silverpine Forest
    zone2LevelBracket[148] = {10, 21};   // Darkshore
    zone2LevelBracket[3433] = {10, 22};  // Ghostlands
    zone2LevelBracket[3525] = {10, 21};  // Bloodmyst Isle

    // Classic WoW - High - level zones
    zone2LevelBracket[10] = {19, 33};   // Deadwind Pass
    zone2LevelBracket[11] = {21, 30};   // Wetlands
    zone2LevelBracket[44] = {16, 28};   // Redridge Mountains
    zone2LevelBracket[267] = {20, 34};  // Hillsbrad Foothills
    zone2LevelBracket[331] = {18, 33};  // Ashenvale
    zone2LevelBracket[400] = {24, 36};  // Thousand Needles
    zone2LevelBracket[406] = {16, 29};  // Stonetalon Mountains

    // Classic WoW - Higher - level zones
    zone2LevelBracket[3] = {36, 46};    // Badlands
    zone2LevelBracket[8] = {36, 46};    // Swamp of Sorrows
    zone2LevelBracket[15] = {35, 46};   // Dustwallow Marsh
    zone2LevelBracket[16] = {45, 52};   // Azshara
    zone2LevelBracket[33] = {32, 47};   // Stranglethorn Vale
    zone2LevelBracket[45] = {30, 42};   // Arathi Highlands
    zone2LevelBracket[47] = {42, 51};   // Hinterlands
    zone2LevelBracket[51] = {45, 51};   // Searing Gorge
    zone2LevelBracket[357] = {40, 52};  // Feralas
    zone2LevelBracket[405] = {30, 41};  // Desolace
    zone2LevelBracket[440] = {41, 52};  // Tanaris

    // Classic WoW - Top - level zones
    zone2LevelBracket[4] = {52, 57};     // Blasted Lands
    zone2LevelBracket[28] = {50, 60};    // Western Plaguelands
    zone2LevelBracket[46] = {51, 60};    // Burning Steppes
    zone2LevelBracket[139] = {54, 62};   // Eastern Plaguelands
    zone2LevelBracket[361] = {47, 57};   // Felwood
    zone2LevelBracket[490] = {49, 56};   // Un'Goro Crater
    zone2LevelBracket[618] = {54, 61};   // Winterspring
    zone2LevelBracket[1377] = {54, 63};  // Silithus

    // The Burning Crusade - Zones
    zone2LevelBracket[3483] = {58, 66};  // Hellfire Peninsula
    zone2LevelBracket[3518] = {64, 70};  // Nagrand
    zone2LevelBracket[3519] = {62, 73};  // Terokkar Forest
    zone2LevelBracket[3520] = {66, 73};  // Shadowmoon Valley
    zone2LevelBracket[3521] = {60, 67};  // Zangarmarsh
    zone2LevelBracket[3522] = {64, 73};  // Blade's Edge Mountains
    zone2LevelBracket[3523] = {67, 73};  // Netherstorm
    zone2LevelBracket[4080] = {68, 73};  // Isle of Quel'Danas

    // Wrath of the Lich King - Zones
    zone2LevelBracket[65] = {71, 77};    // Dragonblight
    zone2LevelBracket[66] = {74, 80};    // Zul'Drak
    zone2LevelBracket[67] = {77, 80};    // Storm Peaks
    zone2LevelBracket[210] = {77, 80};   // Icecrown Glacier
    zone2LevelBracket[394] = {72, 78};   // Grizzly Hills
    zone2LevelBracket[495] = {68, 74};   // Howling Fjord
    zone2LevelBracket[2817] = {77, 80};  // Crystalsong Forest
    zone2LevelBracket[3537] = {68, 75};  // Borean Tundra
    zone2LevelBracket[3711] = {75, 80};  // Sholazar Basin
    zone2LevelBracket[4197] = {79, 80};  // Wintergrasp

    // Override with values from config
    for (auto const& [zoneId, bracketPair] : sPlayerbotAIConfig.zoneBrackets)
        zone2LevelBracket[zoneId] = {bracketPair.first, bracketPair.second};
}

void WorldNavigationMgr::PrepareDestinationCache()
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    uint32 flightMastersCount = 0;
    uint32 innkeepersCount = 0;
    uint32 bankerCount = 0;

    LOG_INFO("playerbots", "Preparing destination caches for {} levels...", maxLevel);
    // Temporary map to group creatures by entry and area
    std::unordered_map<uint32, std::unordered_map<uint32, std::vector<CreatureSpawnInfo>>> tempCreatureMap;

    for (auto const& [guid, creatureData] : sObjectMgr->GetAllCreatureData())
    {
        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creatureData.id1);
        if (!creatureTemplate)
            continue;

        uint16 mapId = creatureData.mapid;
        if (std::find(sPlayerbotAIConfig.randomBotMaps.begin(), sPlayerbotAIConfig.randomBotMaps.end(), mapId)
                      == sPlayerbotAIConfig.randomBotMaps.end())
            continue;

        float x = creatureData.posX;
        float y = creatureData.posY;
        float z = creatureData.posZ;
        float orient = creatureData.orientation;
        uint32 entry = creatureData.id1;  // The creature spawn ID

        Map* map = sMapMgr->FindMap(mapId, 0);
        if (!map)
            continue;

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
        if (!area)
            continue;

        uint32 areaId = area->zone ? area->zone : area->ID;

        // === GRINDABLE CREATURES ===
        if (creatureTemplate->npcflag == 0 &&
            creatureTemplate->lootid != 0 &&
            creatureTemplate->maxlevel - creatureTemplate->minlevel < 3 &&
            creatureTemplate->Entry != 32820 && creatureTemplate->Entry != 24196 &&
            creatureTemplate->Entry != 30627 && creatureTemplate->Entry != 30617 &&
            creatureData.spawntimesecs < 1000 &&
            creatureTemplate->faction != 11 && creatureTemplate->faction != 71 &&
            creatureTemplate->faction != 79 && creatureTemplate->faction != 85 &&
            creatureTemplate->faction != 188 && creatureTemplate->faction != 1575 &&
            (creatureTemplate->unit_flags & 256) == 0 &&
            (creatureTemplate->unit_flags & 4096) == 0 &&
            creatureTemplate->rank == 0)
        {
            CreatureSpawnInfo spawnInfo{guid, mapId, x, y, z, areaId};
            tempCreatureMap[entry][areaId].push_back(spawnInfo);
        }
        // === FLIGHT MASTERS ===
        else if ((creatureTemplate->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER ||
                  creatureTemplate->npcflag & UNIT_NPC_FLAG_INNKEEPER) &&
                creatureTemplate->Entry != 3838 && creatureTemplate->Entry != 29480)
        {
            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
            bool forHorde = !(factionEntry->hostileMask & 4);
            bool forAlliance = !(factionEntry->hostileMask & 2);

            if (creatureTemplate->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER)
            {
                WorldPosition pos(mapId, x, y, z, orient);
                if (forHorde)
                    hordeFlightMasterCache[guid] = pos;

                if (forAlliance)
                    allianceFlightMasterCache[guid] = pos;
                flightMastersCount++;
            }
            else if (creatureTemplate->npcflag & UNIT_NPC_FLAG_INNKEEPER)
            {
                if (zone2LevelBracket.find(areaId) == zone2LevelBracket.end())
                    continue;

                LevelBracket bracket = zone2LevelBracket[areaId];
                WorldPosition loc(mapId, x + cos(orient) * 5.0f, y + sin(orient) * 5.0f, z + 0.5f, orient + M_PI);
                for (int i = bracket.low; i <= bracket.high; i++)
                {
                    if (forHorde)
                        hordeHubsPerLevelCache[i].push_back(loc);

                    if (forAlliance)
                        allianceHubsPerLevelCache[i].push_back(loc);
                    innkeepersCount++;
                }
            }
        }
        // === BANKERS ===
        else if (creatureTemplate->npcflag & UNIT_NPC_FLAG_BANKER &&
                 creatureTemplate->npcflag != 135298 &&
                 creatureTemplate->minlevel != 55 &&
                 creatureTemplate->minlevel != 65 &&
                 creatureTemplate->faction != 35 && creatureTemplate->faction != 474 &&
                 creatureTemplate->faction != 69 && creatureTemplate->faction != 57 &&
                 creatureTemplate->Entry != 30606 && creatureTemplate->Entry != 30608 &&
                 creatureTemplate->Entry != 29282)
        {
            WorldLocation loc(mapId, x + cos(orient) * 6.0f, y + sin(orient) * 6.0f, z + 2.0f, orient + M_PI);
            BankerLocation bLoc;
            bLoc.loc = WorldLocation(mapId, x + cos(orient) * 6.0f, y + sin(orient) * 6.0f, z + 2.0f, orient + M_PI);
            bLoc.entry = entry;
            uint32 level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;
            for (int32 l = 1; l <= maxLevel; l++)
            {
                // Bots 1-60 go to base game bankers (all have minlevel 30 or 45)
                if (l <=60 && level > 45)
                    continue;

                // Bots 61-70 go to Shattrath bankers (all have minlevel 60 or 70)
                if ((l >=61 && l <=70) && (level < 60 || level > 70))
                    continue;

                // Bots 71+ go to Dalaran bankers (all have minlevel 75)
                if ((l >=71) && level != 75)
                    continue;

                bankerLocsPerLevelCache[(uint8)l].push_back(bLoc);
                bankerEntryToLocation[bLoc.entry] = bLoc.loc;
            }
            bankerCount++;
        }
    }
    // Build the creature spawn clusters from temp map
    for (auto const& [entry, areaClusters] : tempCreatureMap)
    {
        std::vector<CreatureLocationCluster> clusters;

        for (auto const& [areaId, spawns] : areaClusters)
        {
            CreatureLocationCluster cluster;
            cluster.areaId = areaId;
            cluster.spawns = spawns;

            // Calculate average position
            float totalX = 0, totalY = 0, totalZ = 0;
            for (auto const& spawn : spawns)
            {
                totalX += spawn.posX;
                totalY += spawn.posY;
                totalZ += spawn.posZ;
            }
            cluster.avgX = totalX / spawns.size();
            cluster.avgY = totalY / spawns.size();
            cluster.avgZ = totalZ / spawns.size();

            clusters.push_back(cluster);

            CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
            if (creatureTemplate)
            {
                uint8 level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;

                for (auto const& spawn : spawns)
                {
                    GrindingSpot spot{
                        WorldLocation(spawn.mapId, spawn.posX, spawn.posY, spawn.posZ, 0),
                        creatureTemplate->minlevel,
                        creatureTemplate->maxlevel
                    };

                    grindingSpotsByArea[areaId].push_back(spot);
                    grindingSpotsByLevel[level].push_back(spot);
                }
            }
        }

        creatureSpawnsByTemplate[entry] = clusters;
    }
    //Add travel hubs based on player start locations
    for (uint32 i = 1; i < MAX_RACES; i++)
    {
        for (uint32 j = 1; j < MAX_CLASSES; j++)
        {
            PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);

            if (!info)
                continue;

            WorldPosition pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            for (int32 l = 1; l <= 5; l++)
            {
                if ((1 << (i - 1)) & RACEMASK_ALLIANCE)
                    allianceHubsPerLevelCache[(uint8)l].push_back(pos);
                else
                    hordeHubsPerLevelCache[(uint8)l].push_back(pos);
            }
                break;
        }
    }
    LOG_INFO("playerbots", ">> {} flight masters and {} innkeepers and {} banker locations for level collected.", flightMastersCount, innkeepersCount, bankerCount);
}

void WorldNavigationMgr::BuildTaxiGraph()
{
    taxiGraph.clear();
    std::unordered_map<uint32, std::unordered_set<uint32>> tempGraph;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* path = sTaxiPathStore.LookupEntry(i);
        if (!path)
            continue;

        if (path->to == 0 || path->to == uint32(-1))
            continue;

        tempGraph[path->from].insert(path->to);
        tempGraph[path->to].insert(path->from);
    }
    for (auto const& [node, neighbors] : tempGraph)
        taxiGraph[node] = std::vector<uint32>(neighbors.begin(), neighbors.end());
}

void WorldNavigationMgr::ComputeAllPaths()
{
    std::set<uint32> allNodes;
    for (auto const& [source, neighbors] : taxiGraph)
        allNodes.insert(source);

    for (uint32 source : allNodes)
    {
        auto parentMap = BFS(source);

        for (uint32 target : allNodes)
        {
            if (source == target)
                continue;

            auto path = BuildPath(source, target, parentMap);
            if (!path.empty())
                taxiPathCache[source][target] = path;
        }
    }
}

std::unordered_map<uint32, uint32> WorldNavigationMgr::BFS(uint32 fromNode)
{
    std::queue<uint32> workQueue;
    std::unordered_set<uint32> visited;
    std::unordered_map<uint32, uint32> parentMap;

    workQueue.push(fromNode);
    visited.insert(fromNode);
    parentMap[fromNode] = 0;

    while (!workQueue.empty())
    {
        uint32 current = workQueue.front();
        workQueue.pop();

        for (uint32 next : taxiGraph.at(current))
        {
            if (visited.count(next))
                continue;

            visited.insert(next);
            parentMap[next] = current;
            workQueue.push(next);
        }
    }
    return parentMap;
}

std::vector<uint32> WorldNavigationMgr::BuildPath(uint32 fromNode, uint32 toNode,
                              const std::unordered_map<uint32, uint32>& parentMap)
{
    if (!parentMap.count(toNode))
        return {}; // unreachable

    std::vector<uint32> path;
    uint32 current = toNode;
    while (current !=  fromNode)
    {
        path.push_back(current);
        auto it  = parentMap.find(current);
        if (it == parentMap.end() || it->second == 0)
            break;
        current = it->second;
    }

    path.push_back(fromNode);
    std::reverse(path.begin(), path.end());
    return path;
}

int WorldNavigationMgr::GetCityWeight(CityId city)
{
    int weight = 0;
    switch (city)
    {
        case CityId::STORMWIND:       weight = sPlayerbotAIConfig.weightTeleToStormwind; break;
        case CityId::IRONFORGE:       weight = sPlayerbotAIConfig.weightTeleToIronforge; break;
        case CityId::DARNASSUS:       weight = sPlayerbotAIConfig.weightTeleToDarnassus; break;
        case CityId::EXODAR:          weight = sPlayerbotAIConfig.weightTeleToExodar; break;
        case CityId::ORGRIMMAR:       weight = sPlayerbotAIConfig.weightTeleToOrgrimmar; break;
        case CityId::UNDERCITY:       weight = sPlayerbotAIConfig.weightTeleToUndercity; break;
        case CityId::THUNDER_BLUFF:   weight = sPlayerbotAIConfig.weightTeleToThunderBluff; break;
        case CityId::SILVERMOON_CITY: weight = sPlayerbotAIConfig.weightTeleToSilvermoonCity; break;
        case CityId::SHATTRATH_CITY:  weight = sPlayerbotAIConfig.weightTeleToShattrathCity; break;
        case CityId::DALARAN:         weight = sPlayerbotAIConfig.weightTeleToDalaran; break;
        default:              weight = 0; break;
    }
    return weight;
}