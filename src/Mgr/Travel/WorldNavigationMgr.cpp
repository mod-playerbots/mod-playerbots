#include "WorldNavigationMgr.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include "Log.h"
#include "MapMgr.h"
#include "PlayerbotAIConfig.h"

std::vector<std::vector<uint32>> WorldNavigationMgr::GetOptimalDestinations(Player* bot)
{
    std::vector<std::vector<uint32>> validDestinations;
    uint8 level = bot->GetLevel();
    Creature* nearestFlightMaster = GetNearestFlightMaster(bot);
    if (!nearestFlightMaster || bot->GetDistance(nearestFlightMaster) > 500.0f)
        return validDestinations;
    uint32 fromNode = sObjectMgr->GetNearestTaxiNode(nearestFlightMaster->GetPositionX(), nearestFlightMaster->GetPositionY(),
                                            nearestFlightMaster->GetPositionZ(), nearestFlightMaster->GetMapId(),
                                            bot->GetTeamId());
    if (!fromNode)
        return validDestinations;

    std::vector<WorldLocation> candidateLocations =  GetCityLocations(bot);
    std::vector<WorldLocation> hubLocations = GetTravelHubs(bot);
    candidateLocations.insert(candidateLocations.end(), hubLocations.begin(), hubLocations.end());

    std::vector<uint32> candidateToNodes;
    for (auto const& loc : candidateLocations)
    {
        uint32 candidateNode = sObjectMgr->GetNearestTaxiNode(loc.GetPositionX(), loc.GetPositionY(),
                                            loc.GetPositionZ(), loc.GetMapId(),
                                            bot->GetTeamId());
        if (candidateNode)
            candidateToNodes.push_back(candidateNode);
    }

    for (uint32 const node :candidateToNodes)
    {
        std::vector<uint32> path = FindTaxiPath(fromNode, node);
        if (path.empty())
            continue;

            validDestinations.push_back(path);
    }
    return validDestinations;
}

const std::vector<WorldLocation> WorldNavigationMgr::GetTravelHubs(Player* bot)
{
    std::vector<WorldLocation> locs = bot->GetTeamId() == ALLIANCE
                                                 ? allianceHubsPerLevelCache[bot->GetLevel()]
                                                 : hordeHubsPerLevelCache[bot->GetLevel()];
    return locs;
}


void WorldNavigationMgr::Init()
{
    if (sPlayerbotAIConfig->enabled)
    {
        PrepareZone2LevelBracket();
        PrepareTeleportCache();
    }
    BuildTaxiGraph();
    ComputeAllPaths();
    LOG_INFO("playerbots", "Playerbots Taxi graph and cache built.");
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
    for (auto const& [zoneId, bracketPair] : sPlayerbotAIConfig->zoneBrackets)
        zone2LevelBracket[zoneId] = {bracketPair.first, bracketPair.second};
}

void WorldNavigationMgr::PrepareTeleportCache()
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Preparing random teleport caches for {} levels...", maxLevel);

    QueryResult results = WorldDatabase.Query(
        "SELECT "
        "g.map, "
        "position_x, "
        "position_y, "
        "position_z, "
        "t.minlevel, "
        "t.maxlevel "
        "FROM "
        "(SELECT "
        "map, "
        "MIN( c.guid ) guid "
        "FROM "
        "creature c "
        "INNER JOIN creature_template t ON c.id1 = t.entry "
        "WHERE "
        "t.npcflag = 0 "
        "AND t.lootid != 0 "
        "AND t.maxlevel - t.minlevel < 3 "
        "AND map IN ({}) "
        "AND t.entry not in (32820, 24196, 30627, 30617) "
        "AND c.spawntimesecs < 1000 "
        "AND t.faction not in (11, 71, 79, 85, 188, 1575) "
        "AND (t.unit_flags & 256) = 0 "
        "AND (t.unit_flags & 4096) = 0 "
        "AND t.rank = 0 "
        // "AND (t.flags_extra & 32768) = 0 "
        "GROUP BY "
        "map, "
        "ROUND(position_x / 50), "
        "ROUND(position_y / 50), "
        "ROUND(position_z / 50) "
        "HAVING "
        "count(*) >= 2) "
        "AS g "
        "INNER JOIN creature c ON g.guid = c.guid "
        "INNER JOIN creature_template t on c.id1 = t.entry "
        "ORDER BY "
        "t.minlevel;",
        sPlayerbotAIConfig->randomBotMapsAsString.c_str());
    uint32 collected_locs = 0;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].Get<uint16>();
            float x = fields[1].Get<float>();
            float y = fields[2].Get<float>();
            float z = fields[3].Get<float>();
            uint32 min_level = fields[4].Get<uint32>();
            uint32 max_level = fields[5].Get<uint32>();
            uint32 level = (min_level + max_level + 1) / 2;
            WorldLocation loc(mapId, x, y, z, 0);
            collected_locs++;
            for (int32 l = (int32)level - (int32)sPlayerbotAIConfig->randomBotTeleLowerLevel;
                 l <= (int32)level + (int32)sPlayerbotAIConfig->randomBotTeleHigherLevel; l++)
            {
                if (l < 1 || l > maxLevel)
                {
                    continue;
                }
                locsPerLevelCache[(uint8)l].push_back(loc);
            }
        } while (results->NextRow());
    }
    LOG_INFO("playerbots", ">> {} locations for level collected.", collected_locs);

    PrepareZone2LevelBracket();
    LOG_INFO("playerbots", "Preparing innkeepers / flightmasters locations for level...");
    results = WorldDatabase.Query(
        "SELECT "
        "map, "
        "position_x, "
        "position_y, "
        "position_z, "
        "orientation, "
        "t.faction, "
        "t.entry, "
        "t.npcflag, "
        "c.guid "
        "FROM "
        "creature c "
        "INNER JOIN creature_template t on c.id1 = t.entry "
        "WHERE "
        "t.npcflag & 73728 "
        "AND map IN ({}) "
        "ORDER BY "
        "t.minlevel;",
        sPlayerbotAIConfig->randomBotMapsAsString.c_str());
    collected_locs = 0;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].Get<uint16>();
            float x = fields[1].Get<float>();
            float y = fields[2].Get<float>();
            float z = fields[3].Get<float>();
            float orient = fields[4].Get<float>();
            uint32 faction = fields[5].Get<uint32>();
            uint32 tEntry = fields[6].Get<uint32>();
            uint32 tNpcflag = fields[7].Get<uint32>();
            uint32 guid = fields[8].Get<uint32>();

            if (tEntry == 3838 || tEntry == 29480)
                continue;

            const FactionTemplateEntry* entry = sFactionTemplateStore.LookupEntry(faction);

            WorldLocation loc(mapId, x + cos(orient) * 5.0f, y + sin(orient) * 5.0f, z + 0.5f, orient + M_PI);
            collected_locs++;
            Map* map = sMapMgr->FindMap(loc.GetMapId(), 0);
            if (!map)
                continue;
            bool forHorde = !(entry->hostileMask & 4);
            bool forAlliance = !(entry->hostileMask & 2);
            if (tNpcflag & UNIT_NPC_FLAG_FLIGHTMASTER)
            {
                WorldPosition pos(mapId, x, y, z, orient);
                if (forHorde)
                    hordeFlightMasterCache[guid] = pos;

                    if (forAlliance)
                        allianceFlightMasterCache[guid] = pos;
            }
            const AreaTableEntry* area = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
            uint32 zoneId = area->zone ? area->zone : area->ID;
            if (zone2LevelBracket.find(zoneId) == zone2LevelBracket.end())
                continue;
            LevelBracket bracket = zone2LevelBracket[zoneId];
            for (int i = bracket.low; i <= bracket.high; i++)
            {
                if (forHorde)
                    hordeHubsPerLevelCache[i].push_back(loc);

                if (forAlliance)
                    allianceHubsPerLevelCache[i].push_back(loc);
            }

        } while (results->NextRow());
    }

    // add all initial position
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
    LOG_INFO("playerbots", ">> {} innkeepers locations for level collected.", collected_locs);

    results = WorldDatabase.Query(
        "SELECT "
        "map, "
        "position_x, "
        "position_y, "
        "position_z, "
        "orientation, "
        "t.minlevel, "
        "t.entry "
        "FROM "
        "creature c "
        "INNER JOIN creature_template t on c.id1 = t.entry "
        "WHERE "
        "t.npcflag & 131072 "
        "AND t.npcflag != 135298 "
        "AND t.minlevel != 55 "
        "AND t.minlevel != 65 "
        "AND t.faction not in (35, 474, 69, 57) "
        "AND t.entry not in (30606, 30608, 29282) "
        "AND map IN ({}) "
        "ORDER BY "
        "t.minlevel;",
        sPlayerbotAIConfig->randomBotMapsAsString.c_str());
    collected_locs = 0;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].Get<uint16>();
            float x = fields[1].Get<float>();
            float y = fields[2].Get<float>();
            float z = fields[3].Get<float>();
            float orient = fields[4].Get<float>();
            uint32 level = fields[5].Get<uint32>();
            uint32 entry = fields[6].Get<uint32>();
            BankerLocation bLoc;
            bLoc.loc = WorldLocation(mapId, x + cos(orient) * 6.0f, y + sin(orient) * 6.0f, z + 2.0f, orient + M_PI);
            bLoc.entry = entry;
            collected_locs++;
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
        } while (results->NextRow());
    }
    LOG_INFO("playerbots", ">> {} banker locations for level collected.", collected_locs);
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

std::vector<WorldLocation> WorldNavigationMgr::GetCityLocations(Player* bot)
{
    std::vector<WorldLocation> locations;
    uint32 level = bot->GetLevel();
    uint8 race = bot->getRace();
    //TODO Remove this check. delegate to decision not this function. Keep scope clean.
    if (level >= 10 && urand(0, 100) < sPlayerbotAIConfig->probTeleToBankers * 100)
    {
        std::vector<WorldLocation> fallbackLocs;
        for (auto& bLoc : bankerLocsPerLevelCache[level])
            fallbackLocs.push_back(bLoc.loc);

        if (!sPlayerbotAIConfig->enableWeightTeleToCityBankers)
            return fallbackLocs;

        // Collect valid cities based on bot faction.
        std::unordered_set<CityId> validBankerCities;
        for (auto& loc : bankerLocsPerLevelCache[level])
        {
            auto cityIt = bankerToCity.find(loc.entry);
            if (cityIt == bankerToCity.end())
                continue;

            CityId cityId = cityIt->second.first;
            FactionId cityFactionId = cityIt->second.second;

            if ((bot->GetTeamId() == ALLIANCE && cityFactionId == FactionId::ALLIANCE) ||
                (!bot->GetTeamId() == ALLIANCE && cityFactionId == FactionId::HORDE) ||
                (cityFactionId == FactionId::NEUTRAL))
            {
                validBankerCities.insert(cityId);
            }
        }

        // Fallback if no valid cities
        if (validBankerCities.empty())
            return fallbackLocs;

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
            return fallbackLocs;

        // Pick a weighted city randomly, then a random banker in that city
        //   then teleport to that banker
        CityId selectedCity = weightedCities[urand(0, weightedCities.size() - 1)];
        auto const& bankers = cityToBankers.at(selectedCity);
        uint32 selectedBankerEntry = bankers[urand(0, bankers.size() - 1)];
        auto locIt = bankerEntryToLocation.find(selectedBankerEntry);
        if (locIt != bankerEntryToLocation.end())
        {
            std::vector<WorldLocation> teleportTarget = { locIt->second };
            return teleportTarget;
        }
    }
    // Fallback if something went wrong
    return locations;
}

const std::vector<WorldLocation> WorldNavigationMgr::GetTeleportLocations(Player* bot)
{
    std::vector<WorldLocation> locations;
    uint32 level = bot->GetLevel();
    uint8 isAlliance = bot->GetTeamId() == ALLIANCE;
    if (sPlayerbotAIConfig->enableNewRpgStrategy)
        locations = isAlliance ? allianceHubsPerLevelCache[level] : hordeHubsPerLevelCache[level];
    else
        locations = locsPerLevelCache[level];
    return locations;
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
    std::vector<uint32> path;

    if (!parentMap.count(toNode))
        return path; // unreachable

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

std::vector<uint32> WorldNavigationMgr::FindTaxiPath(uint32 fromNode, uint32 toNode)
{
    std::vector<uint32> path;

    if (fromNode == toNode)
        return path;

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    TaxiNodesEntry const* endNode = sTaxiNodesStore.LookupEntry(toNode);

    if (!startNode || !endNode || startNode->map_id != endNode->map_id)
        return path;

    auto cacheItr = taxiPathCache.find(fromNode);
    if (cacheItr != taxiPathCache.end())
    {
        auto toNodeItr = cacheItr->second.find(toNode);
        if (toNodeItr != cacheItr->second.end())
            return toNodeItr->second;
    }
    return path;
}

Creature* WorldNavigationMgr::GetNearestFlightMaster(Player* bot)
{
    std::map<uint32, WorldPosition>& flightMasterCache =
        (bot->GetTeamId() == ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    Creature* nearestFlightMaster = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto const& [entry, pos] : flightMasterCache)
    {
        if (pos.GetMapId() == bot->GetMapId())
        {
            float distance = bot->GetExactDist2dSq(pos);
            if (distance < nearestDistance)
            {
                Creature* flightMaster = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), entry);
                if (flightMaster)
                {
                    nearestDistance = distance;
                    nearestFlightMaster = flightMaster;
                }
            }
        }
    }

    return nearestFlightMaster;
}

ObjectGuid WorldNavigationMgr::GetNearestFlightMaster(Player* bot, bool returnGuid)
{
    Creature* nearestFlightMaster = GetNearestFlightMaster(bot);
    if (!nearestFlightMaster)
        return ObjectGuid::Empty;

    return nearestFlightMaster->GetGUID();
}

int WorldNavigationMgr::GetCityWeight(CityId city)
{
    int weight = 0;
    switch (city)
    {
        case CityId::STORMWIND:       weight = sPlayerbotAIConfig->weightTeleToStormwind; break;
        case CityId::IRONFORGE:       weight = sPlayerbotAIConfig->weightTeleToIronforge; break;
        case CityId::DARNASSUS:       weight = sPlayerbotAIConfig->weightTeleToDarnassus; break;
        case CityId::EXODAR:          weight = sPlayerbotAIConfig->weightTeleToExodar; break;
        case CityId::ORGRIMMAR:       weight = sPlayerbotAIConfig->weightTeleToOrgrimmar; break;
        case CityId::UNDERCITY:       weight = sPlayerbotAIConfig->weightTeleToUndercity; break;
        case CityId::THUNDER_BLUFF:   weight = sPlayerbotAIConfig->weightTeleToThunderBluff; break;
        case CityId::SILVERMOON_CITY: weight = sPlayerbotAIConfig->weightTeleToSilvermoonCity; break;
        case CityId::SHATTRATH_CITY:  weight = sPlayerbotAIConfig->weightTeleToShattrathCity; break;
        case CityId::DALARAN:         weight = sPlayerbotAIConfig->weightTeleToDalaran; break;
        default:              weight = 0; break;
    }
    return weight;
}