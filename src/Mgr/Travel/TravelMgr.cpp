/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TravelMgr.h"
#include "QuestLevelRules.h"
#include "AreaDefines.h"
#include "CellImpl.h"
#include "ChatHelper.h"
#include "Corpse.h"
#include "Creature.h"
#include "GameEventMgr.h"
#include "Log.h"
#include "Map.h"
#include "MapMgr.h"
#include "ModelIgnoreFlags.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "RaceMgr.h"
#include "TransportMgr.h"
#include "TravelNode.h"

#include <iomanip>
#include <numeric>
#include <unordered_set>

namespace
{
    constexpr uint32 PATHING_CREATURE_ENTRY = 1;
}

// Navigation data

struct Capital
{
    uint32              zoneId;
    TeamId              team;
    char const*         name;
    std::vector<uint16> bankers;
};

static const std::vector<Capital> capitals = {
    { AREA_STORMWIND_CITY,  TEAM_ALLIANCE, "Stormwind",     {2455, 2456, 2457} },
    { AREA_IRONFORGE,       TEAM_ALLIANCE, "Ironforge",     {2460, 2461, 5099} },
    { AREA_DARNASSUS,       TEAM_ALLIANCE, "Darnassus",     {4155, 4208, 4209} },
    { AREA_THE_EXODAR,      TEAM_ALLIANCE, "Exodar",        {17773, 18350, 16710} },
    { AREA_ORGRIMMAR,       TEAM_HORDE,    "Orgrimmar",     {3320, 3309, 3318} },
    { AREA_UNDERCITY,       TEAM_HORDE,    "Undercity",     {4549, 2459, 2458, 4550} },
    { AREA_THUNDER_BLUFF,   TEAM_HORDE,    "Thunder Bluff", {2996, 8356, 8357} },
    { AREA_SILVERMOON_CITY, TEAM_HORDE,    "Silvermoon",    {17631, 17632, 17633, 16615, 16616, 16617} },
    { AREA_SHATTRATH_CITY,  TEAM_NEUTRAL,  "Shattrath",     {19246, 19338, 19034, 19318} },
    { AREA_DALARAN,         TEAM_NEUTRAL,  "Dalaran",       {30604, 30605, 30607, 28675, 28676, 28677, 29530} }
};

static Capital const* FindCapitalByZone(uint32 zoneId)
{
    for (Capital const& capital : capitals)
        if (capital.zoneId == zoneId)
            return &capital;
    return nullptr;
}

static Capital const* FindCapitalByBanker(uint16 bankerEntry)
{
    for (Capital const& capital : capitals)
        for (uint16 bankerId : capital.bankers)
            if (bankerId == bankerEntry)
                return &capital;
    return nullptr;
}

static int GetCityWeight(uint32 zoneId)
{
    switch (zoneId)
    {
        case AREA_STORMWIND_CITY:  return sPlayerbotAIConfig.WeightTeleToStormwind;
        case AREA_IRONFORGE:       return sPlayerbotAIConfig.WeightTeleToIronforge;
        case AREA_DARNASSUS:       return sPlayerbotAIConfig.WeightTeleToDarnassus;
        case AREA_THE_EXODAR:      return sPlayerbotAIConfig.WeightTeleToExodar;
        case AREA_ORGRIMMAR:       return sPlayerbotAIConfig.WeightTeleToOrgrimmar;
        case AREA_UNDERCITY:       return sPlayerbotAIConfig.WeightTeleToUndercity;
        case AREA_THUNDER_BLUFF:   return sPlayerbotAIConfig.WeightTeleToThunderBluff;
        case AREA_SILVERMOON_CITY: return sPlayerbotAIConfig.WeightTeleToSilvermoonCity;
        case AREA_SHATTRATH_CITY:  return sPlayerbotAIConfig.WeightTeleToShattrathCity;
        case AREA_DALARAN:         return sPlayerbotAIConfig.WeightTeleToDalaran;
    }
    return 0;
}

WorldPosition::WorldPosition(std::string const str)
{
    std::vector<std::string> tokens = split(str, '|');
    if (tokens.size() == 5)
    {
        try
        {
            m_mapId = std::stoi(tokens[0]);
            m_positionX = std::stof(tokens[1]);
            m_positionY = std::stof(tokens[2]);
            m_positionZ = std::stof(tokens[3]);
            m_orientation = std::stof(tokens[4]);
        }
        catch (std::exception const&)
        {
            m_mapId = 0;
            m_positionX = 0.0f;
            m_positionY = 0.0f;
            m_positionZ = 0.0f;
            m_orientation = 0.0f;
        }
    }
}

WorldPosition::WorldPosition(uint32 mapId, Position const& pos)
    : WorldLocation(mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation())
{
}

WorldPosition::WorldPosition(WorldObject const* wo)
{
    if (wo)
    {
        set(WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(),
                          wo->GetOrientation()));
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition*> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(*list.front());
    else if (conType == WP_RANDOM)
        set(*list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0]->GetMapId(), 0, 0, 0, 0),
                            [size](WorldLocation i, WorldPosition* j)
                            {
                                i.m_positionX += j->GetPositionX() / size;
                                i.m_positionY += j->GetPositionY() / size;
                                i.m_positionZ += j->GetPositionZ() / size;
                                i.NormalizeOrientation(i.m_orientation += j->GetOrientation() / size);
                                return i;
                            }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(*pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(list.front());
    else if (conType == WP_RANDOM)
        set(list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0].GetMapId(), 0, 0, 0, 0),
                            [size](WorldLocation i, WorldPosition& j)
                            {
                                i.m_positionX += j.GetPositionX() / size;
                                i.m_positionY += j.GetPositionY() / size;
                                i.m_positionZ += j.GetPositionZ() / size;
                                i.NormalizeOrientation(i.m_orientation += j.GetOrientation() / size);
                                return i;
                            }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(uint32 mapid, GridCoord grid)
    : WorldLocation(mapid, (int32(grid.x_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET,
                    (int32(grid.y_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET, 0, 0)
{
}

WorldPosition::WorldPosition(uint32 mapid, CellCoord cell)
    : WorldLocation(
          mapid, (int32(cell.x_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET,
          (int32(cell.y_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, 0, 0)
{
}

void WorldPosition::set(WorldLocation const& pos) { WorldRelocate(pos); }

void WorldPosition::set(WorldPosition const& pos)
{
    WorldRelocate(pos.m_mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
}

void WorldPosition::set(WorldObject const* pos)
{
    WorldRelocate(pos->GetMapId(), pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), pos->GetOrientation());
}

void WorldPosition::setMapId(uint32 id)
{
    m_mapId = id;
}

void WorldPosition::setX(float x) { m_positionX = x; }

void WorldPosition::setY(float y) { m_positionY = y; }

void WorldPosition::setZ(float z) { m_positionZ = z; }

void WorldPosition::setO(float o) { m_orientation = o; }

WorldPosition::operator bool() const
{
    return GetMapId() != 0 || GetPositionX() != 0 || GetPositionY() != 0 || GetPositionZ() != 0;
}

bool operator==(WorldPosition const& p1, WorldPosition const& p2)
{
    return p1.GetMapId() == p2.GetMapId() && p2.GetPositionX() == p1.GetPositionX() &&
           p2.GetPositionY() == p1.GetPositionY() && p2.GetPositionZ() == p1.GetPositionZ() &&
           p2.GetOrientation() == p1.GetOrientation();
}

bool operator!=(WorldPosition const& p1, WorldPosition const& p2) { return !(p1 == p2); }

WorldPosition& WorldPosition::operator+=(WorldPosition const& p1)
{
    m_positionX += p1.GetPositionX();
    m_positionY += p1.GetPositionY();
    m_positionZ += p1.GetPositionZ();
    return *this;
}

WorldPosition& WorldPosition::operator-=(WorldPosition const& p1)
{
    m_positionX -= p1.GetPositionX();
    m_positionY -= p1.GetPositionY();
    m_positionZ -= p1.GetPositionZ();
    return *this;
}

bool WorldPosition::isOverworld()
{
    return GetMapId() == 0 || GetMapId() == 1 || GetMapId() == 530 || GetMapId() == 571;
}

bool WorldPosition::isInWater()
{
    return getMap() ? getMap()->IsInWater(PHASEMASK_NORMAL, GetPositionX(), GetPositionY(), GetPositionZ(),
                                          DEFAULT_COLLISION_HEIGHT)
                    : false;
};

bool WorldPosition::isUnderWater()
{
    return getMap() ? getMap()->IsUnderWater(PHASEMASK_NORMAL, GetPositionX(), GetPositionY(), GetPositionZ(),
                                             DEFAULT_COLLISION_HEIGHT)
                    : false;
};

bool WorldPosition::isDarkWater()
{
    Map* map = getMap();
    if (!map)
        return false;

    LiquidData const& liquidData = map->GetLiquidData(PHASEMASK_NORMAL, GetPositionX(), GetPositionY(),
                                                      GetPositionZ(), DEFAULT_COLLISION_HEIGHT,
                                                      MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN);

    return (liquidData.Status & MAP_LIQUID_STATUS_SWIMMING) != 0 &&
           (liquidData.Flags & MAP_LIQUID_TYPE_DARK_WATER) != 0;
};

bool WorldPosition::setAtWaterSurface()
{
    if (!isInWater() && !isUnderWater())
        return false;
    Map* map = getMap();
    if (!map)
        return false;
    // Returns the water level when liquid is present; falls back to
    // ground level otherwise. Our isInWater/isUnderWater preconditions
    // ensure liquid exists, so the +0.5y nudge lands the point on top
    // of the surface (matches the reference's surface snap).
    float const level = map->GetWaterOrGroundLevel(PHASEMASK_NORMAL,
                                                   GetPositionX(),
                                                   GetPositionY(),
                                                   GetPositionZ());
    if (level <= INVALID_HEIGHT)
        return false;
    setZ(level + 0.5f);
    return true;
}

bool WorldPosition::IsValid()
{
    return !(GetMapId() == MAPID_INVALID && GetPositionX() == 0 && GetPositionY() == 0 && GetPositionZ() == 0);
}

WorldPosition WorldPosition::relPoint(WorldPosition* center)
{
    return WorldPosition(GetMapId(), GetPositionX() - center->GetPositionX(), GetPositionY() - center->GetPositionY(),
                         GetPositionZ() - center->GetPositionZ(), GetOrientation());
}

WorldPosition WorldPosition::offset(WorldPosition* center)
{
    return WorldPosition(GetMapId(), GetPositionX() + center->GetPositionX(), GetPositionY() + center->GetPositionY(),
                         GetPositionZ() + center->GetPositionZ(), GetOrientation());
}

float WorldPosition::size()
{
    return GetExactDist(0.0f, 0.0f, 0.0f);
}

float WorldPosition::distance(WorldPosition* center)
{
    if (GetMapId() == center->GetMapId())
        return GetExactDist(center->GetPositionX(), center->GetPositionY(), center->GetPositionZ());

    // this -> mapTransfer | mapTransfer -> center
    return TravelMgr::instance().mapTransDistance(*this, *center);
};

float WorldPosition::fDist(WorldPosition* center)
{
    if (GetMapId() == center->GetMapId())
        return GetExactDist2d(center->GetPositionX(), center->GetPositionY());

    // this -> mapTransfer | mapTransfer -> center
    return TravelMgr::instance().fastMapTransDistance(*this, *center);
};

float mapTransfer::fDist(WorldPosition start, WorldPosition end)
{
    return start.fDist(pointFrom) + portalLength + pointTo.fDist(end);
}

// When moving from this along list return last point that falls within range.
// Distance is move distance along path.
WorldPosition WorldPosition::lastInRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    float startDist = 0.0f;

    // Enter the path at the closest point.
    for (auto& p : list)
    {
        float curDist = distance(p);
        if (startDist < curDist || p == list.front())
            startDist = curDist + 0.1f;
    }

    float totalDist = 0.0f;

    // Follow the path from the last nearest point
    // Return last point in range.
    for (auto& p : list)
    {
        float curDist = distance(p);

        if (totalDist > 0)  // We have started the path. Keep counting.
            totalDist += p.distance(std::prev(&p, 1));

        if (curDist == startDist)  // Start the path here.
            totalDist = startDist;

        if (minDist > 0 && totalDist < minDist)
            continue;

        if (maxDist > 0 && totalDist > maxDist)
            continue;  // We do not break here because the path may loop back and have a second startDist point.

        rPoint = p;
    }

    return rPoint;
};

// Todo: remove or adjust to above standard.
WorldPosition WorldPosition::firstOutRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    for (auto& p : list)
    {
        if (minDist > 0 && distance(p) < minDist)
            return p;

        if (maxDist > 0 && distance(p) > maxDist)
            return p;

        rPoint = p;
    }

    return rPoint;
}

// Returns true if (on the x-y plane) the position is inside the three points.
bool WorldPosition::isInside(WorldPosition* p1, WorldPosition* p2, WorldPosition* p3)
{
    if (GetMapId() != p1->GetMapId() || GetMapId() != p2->GetMapId() || GetMapId() != p3->GetMapId())
        return false;

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = mSign(p1, p2);
    d2 = mSign(p2, p3);
    d3 = mSign(p3, p1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

MapEntry const* WorldPosition::getMapEntry() { return sMapStore.LookupEntry(GetMapId()); };

uint32 WorldPosition::getInstanceId()
{
    if (Map* map = sMapMgr->FindBaseMap(GetMapId()))
        return map->GetInstanceId();

    return 0;
}

Map* WorldPosition::getMap()
{
    return sMapMgr->FindMap(GetMapId(), getMapEntry()->Instanceable() ? getInstanceId() : 0);
}

float WorldPosition::getHeight()  // remove const - whipowill
{
    return getMap()->GetHeight(GetPositionX(), GetPositionY(), GetPositionZ());
}

G3D::Vector3 WorldPosition::getVector3() { return G3D::Vector3(GetPositionX(), GetPositionY(), GetPositionZ()); }

std::string const WorldPosition::print()
{
    std::ostringstream out;
    out << GetMapId() << std::fixed << std::setprecision(2);
    out << ';' << GetPositionX();
    out << ';' << GetPositionY();
    out << ';' << GetPositionZ();
    out << ';' << GetOrientation();

    return out.str();
}

std::string const WorldPosition::to_string()
{
    std::stringstream out;
    out << GetMapId() << '|';
    out << GetPositionX() << '|';
    out << GetPositionY() << '|';
    out << GetPositionZ() << '|';
    out << GetOrientation();
    return out.str();
}

std::vector<std::string> WorldPosition::split(std::string const& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void WorldPosition::printWKT(std::vector<WorldPosition> points, std::ostringstream& out, uint32 dim, bool loop)
{
    switch (dim)
    {
        case 0:
            if (points.size() == 1)
                out << "\"POINT(";
            else
                out << "\"MULTIPOINT(";
            break;
        case 1:
            out << "\"LINESTRING(";
            break;
        case 2:
            out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p.getDisplayX() << " " << p.getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front().getDisplayX() << " " << points.front().getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

WorldPosition WorldPosition::getDisplayLocation()
{
    WorldPosition pos = TravelNodeMap::instance().getMapOffset(GetMapId());
    return offset(const_cast<WorldPosition*>(&pos));
}

uint16 WorldPosition::getAreaId()
{
    return sMapMgr->GetAreaId(PHASEMASK_NORMAL, GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ());
}

AreaTableEntry const* WorldPosition::getArea()
{
    uint16 areaId = getAreaId();
    if (!areaId)
        return nullptr;

    return sAreaTableStore.LookupEntry(areaId);
}

std::string const WorldPosition::getAreaName(bool fullName, bool zoneName)
{
    if (!isOverworld())
    {
        MapEntry const* map = sMapStore.LookupEntry(GetMapId());
        if (map)
            return map->name[0];
    }

    AreaTableEntry const* area = getArea();
    if (!area)
        return "";

    std::string areaName = area->area_name[0];

    if (fullName)
    {
        uint16 zoneId = area->zone;

        while (zoneId > 0)
        {
            AreaTableEntry const* parentArea = sAreaTableStore.LookupEntry(zoneId);
            if (!parentArea)
                break;

            std::string const subAreaName = parentArea->area_name[0];

            if (zoneName)
                areaName = subAreaName;
            else
                areaName = subAreaName + " " + areaName;

            zoneId = parentArea->zone;
        }
    }

    return areaName;
}

std::set<Transport*> WorldPosition::getTransports(uint32 /*entry*/)
{
    /*
    if (!entry)
        return getMap()->m_transports;
    else
    {
    */
    std::set<Transport*> transports;
    /*
    for (auto transport : getMap()->m_transports)
        if (transport->GetEntry() == entry)
            transports.insert(transport);

    return transports;
}
*/
    return transports;
}

std::vector<GridCoord> WorldPosition::getGridCoord(WorldPosition secondPos)
{
    std::vector<GridCoord> retVec;

    int lx = std::min(getGridCoord().x_coord, secondPos.getGridCoord().x_coord);
    int ly = std::min(getGridCoord().y_coord, secondPos.getGridCoord().y_coord);
    int ux = std::max(getGridCoord().x_coord, secondPos.getGridCoord().x_coord);
    int uy = std::max(getGridCoord().y_coord, secondPos.getGridCoord().y_coord);

    int32 border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(GridCoord(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromGridCoord(GridCoord gridCoord)
{
    std::vector<WorldPosition> retVec;
    GridCoord g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridCoord;

        if (d == 1 || d == 2)
            g.inc_x(1);

        if (d == 2 || d == 3)
            g.inc_y(1);

        retVec.push_back(WorldPosition(GetMapId(), g));
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromCellCoord(CellCoord cellcoord)
{
    std::vector<WorldPosition> retVec;
    CellCoord p;

    for (uint32 d = 0; d < 4; d++)
    {
        p = cellcoord;

        if (d == 1 || d == 2)
            p.inc_x(1);

        if (d == 2 || d == 3)
            p.inc_y(1);

        retVec.push_back(WorldPosition(GetMapId(), p));
    }
    return retVec;
}

std::vector<WorldPosition> WorldPosition::gridFromCellCoord(CellCoord cellCoord)
{
    Cell c(cellCoord);

    return fromGridCoord(GridCoord(c.GridX(), c.GridY()));
}

std::vector<WorldPosition> WorldPosition::fromPointsArray(std::vector<G3D::Vector3> path)
{
    std::vector<WorldPosition> retVec;
    for (auto p : path)
        retVec.push_back(WorldPosition(GetMapId(), p.x, p.y, p.z, GetOrientation()));

    return retVec;
}

Unit* TravelMgr::GetPathingCreature(uint32 mapId)
{
    auto it = pathingCreatures.find(mapId);
    if (it != pathingCreatures.end())
        return it->second;

    Map* map = sMapMgr->CreateBaseMap(mapId);
    if (!map)
        return nullptr;

    for (uint32 x = 0; x < MAX_NUMBER_OF_GRIDS; ++x)
        for (uint32 y = 0; y < MAX_NUMBER_OF_GRIDS; ++y)
            map->EnsureGridCreated(GridCoord(x, y));

    Creature* creature = new Creature();
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL,
                        PATHING_CREATURE_ENTRY, 0, 0.0f, 0.0f, 0.0f, 0.0f))
    {
        delete creature;
        return nullptr;
    }

    pathingCreatures[mapId] = creature;
    return creature;
}

void TravelMgr::ReleasePathingCreatures()
{
    for (auto& entry : pathingCreatures)
        delete entry.second;

    pathingCreatures.clear();
}

// A single pathfinding attempt from one position to another. Returns pathfinding status and path.
std::vector<WorldPosition> WorldPosition::getPathStepFrom(WorldPosition startPos, PathGenerator& path)
{
    // Explicit-start overload. Without this, the chain begins from the
    // unit's current position every step and never advances.
    if (!path.CalculatePath(startPos.GetPositionX(), startPos.GetPositionY(), startPos.GetPositionZ(),
                            GetPositionX(), GetPositionY(), GetPositionZ(), false))
        return {};

    Movement::PointsArray points = path.GetPath();
    PathType type = path.GetPathType();

    // PathType is a bitmask. AC's PathGenerator returns
    // NORMAL | NOT_USING_PATH when start/end poly is INVALID_POLYREF
    // (BuildShortcut produces a 2-point straight line through whatever's
    // in the way). Reject those to avoid silently dispatching a
    // geometry-ignoring shortcut.
    //
    // AC also ORs NOPATH/SHORT onto the still-set NORMAL bit after
    // replacing the geometry with a 2-point BuildShortcut straight line
    // (PathGenerator.cpp point-cap overrun -> NORMAL|NOPATH, point-limit
    // hit -> NORMAL|SHORT). Reject those composite results too, otherwise
    // we accept a poisoned shortcut that ignores geometry.
    if (!(type & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE)) ||
        (type & (PATHFIND_NOT_USING_PATH | PATHFIND_NOPATH | PATHFIND_SHORT)))
        return {};

    std::vector<WorldPosition> retvec = fromPointsArray(points);
    if (retvec.empty())
        return {};

    // Underwater path-extension. When PATHFIND_INCOMPLETE ends within
    // 50y of dest and both endpoints are underwater with LOS, extend
    // by one 5y step (or straight to dest if <5y). Lets bots traverse
    // navmesh-poor water volumes.
    if (type & PATHFIND_INCOMPLETE)
    {
        WorldPosition end = *this;
        WorldPosition lastPoint = retvec.back();
        float dist = lastPoint.distance(&end);

        if (dist < 50.0f && lastPoint.isUnderWater() && end.isUnderWater())
        {
            Map* m = end.getMap();
            bool inLos = m && m->isInLineOfSight(
                lastPoint.GetPositionX(), lastPoint.GetPositionY(), lastPoint.GetPositionZ() + 1.0f,
                end.GetPositionX(), end.GetPositionY(), end.GetPositionZ() + 1.0f,
                PHASEMASK_NORMAL, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
            if (inLos)
            {
                if (dist < 5.0f)
                    retvec.push_back(end);
                else
                {
                    float dx = end.GetPositionX() - lastPoint.GetPositionX();
                    float dy = end.GetPositionY() - lastPoint.GetPositionY();
                    float dz = end.GetPositionZ() - lastPoint.GetPositionZ();
                    float scale = 5.0f / dist;
                    retvec.emplace_back(end.GetMapId(),
                                        lastPoint.GetPositionX() + dx * scale,
                                        lastPoint.GetPositionY() + dy * scale,
                                        lastPoint.GetPositionZ() + dz * scale);
                }
            }
        }
    }

    return retvec;
}

bool WorldPosition::cropPathTo(std::vector<WorldPosition>& path, float maxDistance)
{
    if (path.empty())
        return false;

    auto bestPos = std::min_element(path.begin(), path.end(),
                                    [this](WorldPosition i, WorldPosition j)
                                    { return this->sqDistance(i) < this->sqDistance(j); });

    bool insRange = this->sqDistance(*bestPos) <= maxDistance * maxDistance;

    if (bestPos == path.end())
        return insRange;

    path.erase(std::next(bestPos), path.end());

    return insRange;
}

// A sequential series of pathfinding attempts. Returns the complete path and if the patfinder eventually found a way to
// the destination.
std::vector<WorldPosition> WorldPosition::getPathFromPath(std::vector<WorldPosition> startPath, Unit* bot,
                                                          uint8 maxAttempt)
{
    // We start at the end of the last path.
    WorldPosition currentPos = startPath.back();

    // No pathfinding across maps.
    if (GetMapId() != currentPos.GetMapId())
        return {};

    std::vector<WorldPosition> subPath, fullPath = startPath;

    Unit* pathUnit = bot ? bot : TravelMgr::instance().GetPathingCreature(GetMapId());
    if (!pathUnit)
        return fullPath;

    PathGenerator path(pathUnit);
    // A bot gets its runtime filter from CreateFilter. The temp creature that
    // builds travel node links would walk lava and steep ground, and stored
    // paths are splined raw, so it gets the bot filter with steep excluded.
    if (!bot)
    {
        path.SetExcludeFlags(NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
        path.SetNavTerrainCost(NAV_WATER, 5.0f);
    }

    // Limit the pathfinding attempts
    for (uint32 i = 0; i < maxAttempt; i++)
    {
        // Reset cached poly state from the previous step so each call
        // is a fresh A* (otherwise the prefix-recycling at
        // PathGenerator.cpp BuildPolyPath snaps the start to the
        // cached corridor, bending the chain).
        path.Clear();

        subPath = getPathStepFrom(currentPos, path);

        if (subPath.empty() || currentPos.distance(&subPath.back()) < sPlayerbotAIConfig.TargetPosRecalcDistance)
            break;

        fullPath.insert(fullPath.end(), std::next(subPath.begin(), 1), subPath.end());

        if (isPathTo(subPath))
            break;

        currentPos = subPath.back();
    }

    return fullPath;
}

bool WorldPosition::GetReachableRandomPointOnGround(Player* bot, float radius, bool randomRange)
{
    radius *= randomRange ? rand_norm() : 1.f;
    float angle = rand_norm() * static_cast<float>(2 * M_PI);
    setX(GetPositionX() + radius * cosf(angle));
    setY(GetPositionY() + radius * sinf(angle));

    float x = GetPositionX();
    float y = GetPositionY();
    float z = GetPositionZ();
    bool canReach = getMap()->CanReachPositionAndGetValidCoords(bot, x, y, z);
    setX(x);
    setY(y);
    setZ(z);

    return canReach;
}

uint32 WorldPosition::getUnitsAggro(GuidVector& units, Player* bot)
{
    units.erase(std::remove_if(units.begin(), units.end(),
                               [this, bot](ObjectGuid guid)
                               {
                                   Creature* creature = ObjectAccessor::GetCreature(*bot, guid);
                                   if (!creature)
                                       return true;

                                   return sqDistance(WorldPosition(creature)) >
                                          creature->GetAttackDistance(bot) * creature->GetAttackDistance(bot);
                               }),
                units.end());

    return units.size();
}

void FindPointCreatureData::operator()(CreatureData const& creatureData)
{
    if (!entry || creatureData.id == entry)
        if ((!point || creatureData.mapid == point.GetMapId()) &&
            (!radius || point.sqDistance(WorldPosition(creatureData.mapid, creatureData.posX, creatureData.posY,
                                                       creatureData.posZ)) < radius * radius))
        {
            data.push_back(&creatureData);
        }
}

void FindPointGameObjectData::operator()(GameObjectData const& gameobjectData)
{
    if (!entry || gameobjectData.id == entry)
        if ((!point || gameobjectData.mapid == point.GetMapId()) &&
            (!radius || point.sqDistance(WorldPosition(gameobjectData.mapid, gameobjectData.posX, gameobjectData.posY,
                                                       gameobjectData.posZ)) < radius * radius))
        {
            data.push_back(&gameobjectData);
        }
}

std::vector<CreatureData const*> WorldPosition::getCreaturesNear(float radius, uint32 entry)
{
    FindPointCreatureData worker(*this, radius, entry);
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
        worker(itr.second);

    return worker.GetResult();
}

std::vector<GameObjectData const*> WorldPosition::getGameObjectsNear(float radius, uint32 entry)
{
    FindPointGameObjectData worker(*this, radius, entry);
    for (auto const& itr : sObjectMgr->GetAllGOData())
        worker(itr.second);

    return worker.GetResult();
}

CreatureTemplate const* GuidPosition::GetCreatureTemplate()
{
    return IsCreature() ? sObjectMgr->GetCreatureTemplate(GetEntry()) : nullptr;
}

GameObjectTemplate const* GuidPosition::GetGameObjectTemplate()
{
    return IsGameObject() ? sObjectMgr->GetGameObjectTemplate(GetEntry()) : nullptr;
}

WorldObject* GuidPosition::GetWorldObject()
{
    if (!*this)
        return nullptr;

    switch (GetHigh())
    {
        case HighGuid::Player:
            return GetPlayer();
        case HighGuid::Transport:
        case HighGuid::Mo_Transport:
        case HighGuid::GameObject:
            return GetGameObject();
        case HighGuid::Vehicle:
        case HighGuid::Unit:
            return GetCreature();
        case HighGuid::Pet:
            return getMap()->GetPet(*this);
        case HighGuid::DynamicObject:
            return getMap()->GetDynamicObject(*this);
        case HighGuid::Corpse:
            return getMap()->GetCorpse(*this);
        default:
            return nullptr;
    }

    return nullptr;
}

GameObject* GuidPosition::GetGameObject()
{
    if (!*this)
        return nullptr;

    if (loadedFromDB)
        return ObjectAccessor::GetSpawnedGameObjectByDBGUID(GetMapId(), GetCounter());

    return getMap()->GetGameObject(*this); // fallback
}

Unit* GuidPosition::GetUnit()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return GetPlayer();

    if (IsPet())
        return getMap()->GetPet(*this);

    return GetCreature();
}

Creature* GuidPosition::GetCreature()
{
    if (!*this)
        return nullptr;

    if (loadedFromDB)
        return ObjectAccessor::GetSpawnedCreatureByDBGUID(GetMapId(), GetCounter());

    return getMap()->GetCreature(*this); // fallback
}

Player* GuidPosition::GetPlayer()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return ObjectAccessor::FindPlayer(*this);

    return nullptr;
}

bool GuidPosition::HasNpcFlag(NPCFlags flag) { return IsCreature() && GetCreatureTemplate()->npcflag & flag; }

bool GuidPosition::IsCreatureOrGOAccessible()
{
    Map* map = getMap();
    if (!map || !map->IsGridLoaded(GetPositionX(), GetPositionY()))
        return false;

    if (IsCreature())
    {
        Creature* creature = GetCreature();
        if (creature && creature->IsInWorld() && creature->IsAlive())
            return true;
    }
    else if (IsGameObject())
    {
        GameObject* go = GetGameObject();
        if (go && go->IsInWorld())
            return true;
    }

    return false;
}

GuidPosition::GuidPosition(WorldObject* wo) : ObjectGuid(wo->GetGUID()), WorldPosition(wo), loadedFromDB(false) {}

GuidPosition::GuidPosition(CreatureData const& creData)
    : ObjectGuid(HighGuid::Unit, creData.id, creData.spawnId),
      WorldPosition(creData.mapid, creData.posX, creData.posY, creData.posZ, creData.orientation)
{
    loadedFromDB = true;
}

GuidPosition::GuidPosition(GameObjectData const& goData)
    : ObjectGuid(HighGuid::GameObject, goData.id, goData.spawnId),
      WorldPosition(goData.mapid, goData.posX, goData.posY, goData.posZ, goData.orientation)
{
    loadedFromDB = true;
}

TravelDestination::~TravelDestination()
{
    for (WorldPosition* point : points)
        delete point;

    points.clear();
}

std::vector<WorldPosition*> TravelDestination::getPoints(bool ignoreFull)
{
    if (ignoreFull)
        return points;

    uint32 max = maxVisitorsPerPoint;
    if (!max)
        return points;

    std::vector<WorldPosition*> retVec;
    std::copy_if(points.begin(), points.end(), std::back_inserter(retVec),
                 [max](WorldPosition* p) { return p->getVisitors() < max; });
    return retVec;
}

WorldPosition* TravelDestination::nearestPoint(WorldPosition* pos)
{
    return *std::min_element(points.begin(), points.end(),
                             [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
}

std::vector<WorldPosition*> TravelDestination::touchingPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points;
    for (auto& point : points)
    {
        float dist = pos->distance(point);
        if (!dist)
            continue;

        if (dist > radiusMax * 2)
            continue;

        ret_points.push_back(point);
    }

    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::sortedPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points = points;
    std::sort(ret_points.begin(), ret_points.end(),
              [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::nextPoint(WorldPosition* pos, bool ignoreFull)
{
    return TravelMgr::instance().getNextPoint(pos, ignoreFull ? points : getPoints());
}

bool TravelDestination::isFull(bool ignoreFull)
{
    if (!ignoreFull && maxVisitors > 0 && visitors >= maxVisitors)
        return true;

    if (maxVisitorsPerPoint > 0)
        if (getPoints(ignoreFull).empty())
            return true;

    return false;
}

std::string const QuestTravelDestination::getTitle() { return ChatHelper::FormatQuest(questTemplate); }

bool QuestRelationTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (botAI && !botAI->HasStrategy("rpg quest", BOT_STATE_NON_COMBAT))
        return false;

    if (relation == 0)
    {
        if ((int32)questTemplate->GetQuestLevel() >= (int32)bot->GetLevel() + (int32)5)
            return false;

        // skip for now this quest
        if (getPoints().front()->GetMapId() != bot->GetMapId())
            return false;

        if (!bot->GetMap()->GetEntry()->IsWorldMap() || !bot->CanTakeQuest(questTemplate, false))
            return false;

        //uint32 dialogStatus = TravelMgr::instance().getDialogStatus(bot, entry, questTemplate); //not used, shadowed by the next declaration, line marked for removal.

        if (AI_VALUE(bool, "can fight equal"))
        {
            if (AI_VALUE(uint8, "free quest log slots") < 5)
                return false;

            //None has yellow exclamation mark.
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest npc::" + std::to_string(entry)))
                if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry) + "need quest objective::" + std::to_string(questId))) //Noone can do this quest for a usefull reward.
                    return false;
        }
        else
        {
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry))) //Noone can pick up this quest for money.
                return false;

            if (AI_VALUE(uint8, "free quest log slots") < 10)
                return false;
        }

        // Do not try to pick up dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) &&
            !AI_VALUE(bool, "can fight boss"))
            return false;
    }
    else
    {
        if (!AI_VALUE2(bool, "group or", "following party,near leader,can turn in quest npc::" + std::to_string(entry)))
            return false;

        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(&pos)->isOverworld())
                return false;
        }
    }

    return true;
}

std::string const QuestRelationTravelDestination::getTitle()
{
    std::ostringstream out;

    if (relation == 0)
        out << "questgiver";
    else
        out << "questtaker";

    out << " " << ChatHelper::FormatWorldEntry(entry);
    return out.str();
}

bool QuestObjectiveTravelDestination::isActive(Player* bot)
{
    if (questTemplate->GetQuestLevel() > bot->GetLevel() + 1)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (questTemplate->GetQuestLevel() + 5 > bot->GetLevel() && !AI_VALUE(bool, "can fight equal"))
        return false;

    // Check mob level
    if (getEntry() > 0)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(getEntry());
        if (cInfo && (int)cInfo->maxlevel - (int)bot->GetLevel() > 4)
            return false;

        // Do not try to hand-in dungeon/elite quests in instances without a group.
        if (cInfo->rank > CREATURE_ELITE_NORMAL)
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(const_cast<WorldPosition*>(&pos))->isOverworld() &&
                !AI_VALUE(bool, "can fight boss"))
                return false;

            if (!AI_VALUE(bool, "can fight elite"))
                return false;
        }
    }

    if (questTemplate->GetType() == QUEST_TYPE_ELITE && !AI_VALUE(bool, "can fight elite"))
        return false;

    if (!TravelMgr::instance().getObjectiveStatus(bot, questTemplate, objective))
        return false;

    WorldPosition botPos(bot);

    if (getEntry() > 0 && !isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == uint32(getEntry()) && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    return true;
}

std::string const QuestObjectiveTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "objective " << objective;

    if (itemId)
        out << " loot " << ChatHelper::FormatItem(sObjectMgr->GetItemTemplate(itemId), 0, 0) << " from";
    else
        out << " to kill";

    out << " " << ChatHelper::FormatWorldEntry(entry);
    return out.str();
}

bool RpgTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    bool isUsefull = false;

    if (cInfo->npcflag & UNIT_NPC_FLAG_VENDOR)
        if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
            isUsefull = true;

    if (cInfo->npcflag & UNIT_NPC_FLAG_REPAIR)
        if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
            isUsefull = true;

    if (!isUsefull)
        return false;

    // Once the target rpged with it is added to the ignore list. We can now move on.
    GuidSet& ignoreList = GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<GuidSet&>("ignore rpg target")->Get();

    for (ObjectGuid const guid : ignoreList)
    {
        if (guid.GetEntry() == uint32(getEntry()))
            return false;

    }

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction > REP_NEUTRAL;
}

CreatureTemplate const* RpgTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const RpgTravelDestination::getTitle()
{
    std::ostringstream out;

    if (entry > 0)
        out << "rpg npc ";

    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

bool ExploreTravelDestination::isActive(Player* bot)
{
    AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);

    if (area->area_level && (uint32)area->area_level > bot->GetLevel() && bot->GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if (area->exploreFlag == 0xffff)
        return false;

    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    return !(currFields & val);
}

// std::string const ExploreTravelDestination::getTitle()
//{
//     return points[0]->getAreaName();
// };

bool GrindTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "should get money"))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f;   //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f;  //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);

    if ((int32)cInfo->maxlevel > maxLevel)  //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->maxlevel < minLevel)  //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (!cInfo->mingold)
        return false;

    if (cInfo->rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction < REP_NEUTRAL;
}

CreatureTemplate const* GrindTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const GrindTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "grind mob ";

    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

bool BossTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "can fight boss"))
        return false;

    CreatureTemplate const* cInfo = getCreatureTemplate();

    /*
    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = botLevel + 3.0;

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = botLevel - 10;

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;
    */

    if ((int32)cInfo->maxlevel > bot->GetLevel() + 3)
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(), factionEntry);

    if (reaction >= REP_NEUTRAL)
        return false;

    WorldPosition botPos(bot);
    if (!isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == uint32(getEntry()) && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade", getEntry()))
        return false;

    return true;
}

CreatureTemplate const* BossTravelDestination::getCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const BossTravelDestination::getTitle()
{
    std::ostringstream out;
    out << "boss mob ";
    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

TravelTarget::~TravelTarget()
{
    if (!tDestination)
        return;

    releaseVisitors();
    // TravelMgr::instance().botTargets.erase(std::remove(TravelMgr::instance().botTargets.begin(), TravelMgr::instance().botTargets.end(), this),
    // TravelMgr::instance().botTargets.end());
}

void TravelTarget::setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1)
{
    releaseVisitors();

    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    addVisitors();

    setStatus(TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::copyTarget(TravelTarget* target)
{
    setTarget(target->tDestination, target->wPosition);
    groupCopy = target->isGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::addVisitors()
{
    if (!visitor)
    {
        wPosition->addVisitor();
        tDestination->addVisitor();
    }

    visitor = true;
}

void TravelTarget::releaseVisitors()
{
    if (visitor)
    {
        if (tDestination)
            tDestination->remVisitor();
        if (wPosition)
            wPosition->remVisitor();
    }

    visitor = false;
}

float TravelTarget::distance(Player* bot)
{
    WorldPosition pos(bot);
    return wPosition->distance(&pos);
}

WorldPosition* TravelTarget::getPosition() { return wPosition; }

TravelDestination* TravelTarget::getDestination() { return tDestination; }

void TravelTarget::setStatus(TravelStatus status)
{
    m_status = status;
    startTime = getMSTime();

    switch (m_status)
    {
        case TRAVEL_STATUS_NONE:
        case TRAVEL_STATUS_PREPARE:
        case TRAVEL_STATUS_EXPIRED:
            statusTime = 1;
            break;
        case TRAVEL_STATUS_TRAVEL:
            statusTime = getMaxTravelTime() * 2 + sPlayerbotAIConfig.MaxWaitForMove;
            break;
        case TRAVEL_STATUS_WORK:
            statusTime = tDestination->getExpireDelay();
            break;
        case TRAVEL_STATUS_COOLDOWN:
            statusTime = tDestination->getCooldownDelay();
        default:
            break;
    }
}

bool TravelTarget::isActive()
{
    if (m_status == TRAVEL_STATUS_NONE || m_status == TRAVEL_STATUS_EXPIRED || m_status == TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && isTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < getMSTime()))
    {
        setStatus(TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TRAVEL_STATUS_COOLDOWN)
        return true;

    if (isTraveling())
        return true;

    if (isWorking())
        return true;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
};

uint32 TravelTarget::getMaxTravelTime()
{
    return (IN_MILLISECONDS * distance(bot)) / bot->GetSpeed(MOVE_RUN);
}

bool TravelTarget::isTraveling()
{
    if (m_status != TRAVEL_STATUS_TRAVEL)
        return false;

    if (!tDestination->isActive(bot) && !forced)  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->isIn(&pos, radius);

    if (HasArrived)
    {
        setStatus(TRAVEL_STATUS_WORK);
        return false;
    }

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(TravelMgr::instance().nullTravelDestination, TravelMgr::instance().nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isWorking()
{
    if (m_status != TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    /*
    bool HasLeft = tDestination->isOut(&pos);

    if (HasLeft)
    {
        setStatus(TRAVEL_STATUS_TRAVEL);
        return false;
    }
    */

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(TravelMgr::instance().nullTravelDestination, TravelMgr::instance().nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isPreparing()
{
    if (m_status != TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::getTravelState()
{
    if (!tDestination || tDestination->getName() == "NullTravelDestination")
        return TRAVEL_STATE_IDLE;

    if (tDestination->getName() == "QuestRelationTravelDestination")
    {
        if (((QuestRelationTravelDestination*)tDestination)->getRelation() == 0)
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (tDestination->getName() == "QuestObjectiveTravelDestination")
    {
        if (isTraveling() || isPreparing())
            return TRAVEL_STATE_TRAVEL_DO_QUEST;

        if (isWorking())
            return TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (tDestination->getName() == "RpgTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (tDestination->getName() == "ExploreTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TRAVEL_STATE_IDLE;
}

void TravelMgr::Clear()
{
    std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        TravelMgr::setNullTravelTarget(itr->second);

    for (auto& quest : quests)
    {
        for (auto& dest : quest.second->questGivers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questTakers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questObjectives)
        {
            delete dest;
        }
    }

    questGivers.clear();
    quests.clear();
}

uint32 TravelMgr::getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationBounds rbounds;   // QuestRelations (quest-giver)
    QuestRelationBounds irbounds;  // InvolvedRelations (quest-finisher)

    uint32 questId = pQuest->GetQuestId();

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelationBounds(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelationBounds(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelations::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(questId)) ||
            (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelations::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if (status == QUEST_STATUS_NONE)  // For all other cases the mark is handled either at some place else, or with
                                          // involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->IsQuestRewarded(questId)))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 ||
                             pPlayer->GetLevel() <= pPlayer->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

// Selects a random WorldPosition from a list. Use a distance weighted distribution.
std::vector<WorldPosition*> TravelMgr::getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points,
                                                    uint32 /*amount*/)
{
    std::vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);

        return retVec;
    }

    retVec = points;

    std::vector<uint32> weights;

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    // std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return
    // 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    // List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights),
                   [center](WorldPosition* point)
                   { return static_cast<uint32>(200000.f / (1.f + point->distance(center))); });

    Acore::Containers::RandomShuffle(retVec);

    std::vector<float> dists;

    // Total sum of all those weights.
    /*
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    //Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        //Pick a random number in that range.
        uint32 rnd = urand(0, sum);

        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] && (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) ==
    retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }*/

    return retVec;
}

std::vector<WorldPosition> TravelMgr::getNextPoint(WorldPosition center, std::vector<WorldPosition> points,
                                                   uint32 amount)
{
    std::vector<WorldPosition> retVec;

    if (points.size() == 1)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    std::vector<uint32> weights;

    std::transform(points.begin(), points.end(), std::back_inserter(weights),
                   [center](WorldPosition point)
                   { return 1 + 1000 * static_cast<uint32>(exp(-1.f * pow(point.distance(center) / 400.f, 2.f))); });

    // Total sum of all those weights.
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    // Pick a random number in that range.
    uint32 rnd = urand(0, sum);

    // Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] &&
                (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) == retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }

    // Peiru: Crash failsafe - if the retVec is still empty but points exist, return first point
    if (retVec.empty() && points.size() > 0)
        retVec.push_back(points[0]);

    if (!retVec.empty())
        return retVec;

    assert(!"No valid point found.");

    return retVec;
}

QuestStatusData* TravelMgr::getQuestStatus(Player* bot, uint32 questId) { return &bot->getQuestStatusMap()[questId]; }

bool TravelMgr::getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective)
{
    uint32 questId = pQuest->GetQuestId();
    if (!bot->IsActiveQuest(questId))
        return false;

    if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
        return false;

    QuestStatusData* questStatus = TravelMgr::instance().getQuestStatus(bot, questId);

    uint32 reqCount = pQuest->RequiredItemCount[objective];
    uint32 hasCount = questStatus->ItemCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    reqCount = pQuest->RequiredNpcOrGoCount[objective];
    hasCount = questStatus->CreatureOrGOCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    return false;
}

std::vector<TravelDestination*> TravelMgr::getQuestTravelDestinations(Player* bot, int32 questId, bool ignoreFull,
                                                                      bool ignoreInactive, float maxDistance,
                                                                      bool ignoreObjectives)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    if (!questId)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
        for (auto& quest : quests)
        {
            for (auto& dest : quest.second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : quest.second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }
    else if (questId == -1)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (dest->isFull(ignoreFull))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
    }
    else
    {
        auto i = quests.find(questId);

        if (i != quests.end())
        {
            for (auto& dest : i->second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (dest->isFull(ignoreFull))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : i->second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (dest->isFull(ignoreFull))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getRpgTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                    float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : rpgNpcs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getExploreTravelDestinations(Player* bot, bool ignoreFull,
                                                                        bool ignoreInactive)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : exploreLocs)
    {
        if (!ignoreInactive && !dest.second->isActive(bot))
            continue;

        if (dest.second->isFull(ignoreFull))
            continue;

        retTravelLocations.push_back(dest.second);
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getGrindTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                      float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : grindMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

void TravelMgr::setNullTravelTarget(Player* player)
{
    if (!player)
        return;

    PlayerbotAI* playerBotAI = GET_PLAYERBOT_AI(player);
    if (!playerBotAI)
        return;

    TravelTarget* target = playerBotAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    if (target)
        target->setTarget(TravelMgr::instance().nullTravelDestination, TravelMgr::instance().nullWorldPosition, true);
}

void TravelMgr::addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.GetMapId();
    uint32 eMap = end.GetMapId();

    if (sMap == eMap)
        return;

    // Calculate shortcuts.
    if (makeShortcuts)
    {
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& mapTransfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap)  // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = mapTransDistance(*mapTransfer.getPointFrom(), start) + portalDistance;
                    if (mapTransDistance(*mapTransfer.getPointFrom(), end) > newDistToEnd)
                        addMapTransfer(*mapTransfer.getPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap)  // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + mapTransDistance(end, *mapTransfer.getPointTo());
                    if (mapTransDistance(start, *mapTransfer.getPointTo()) > newDistToEnd)
                        addMapTransfer(start, *mapTransfer.getPointTo(), newDistToEnd, false);
                }
            }
        }
    }

    // Add actual transfer.
    auto mapTransfers = mapTransfersMap.find(std::make_pair(start.GetMapId(), end.GetMapId()));

    if (mapTransfers == mapTransfersMap.end())
        mapTransfersMap.insert({{sMap, eMap}, {mapTransfer(start, end, portalDistance)}});
    else
        mapTransfers->second.push_back(mapTransfer(start, end, portalDistance));
};

void TravelMgr::loadMapTransfers()
{
    for (auto& node : TravelNodeMap::instance().getNodes())
    {
        for (auto& link : *node->getLinks())
        {
            addMapTransfer(*node->getPosition(), *link.first->getPosition(), link.second->getDistance());
        }
    }
}

float TravelMgr::mapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.GetMapId();
    uint32 eMap = end.GetMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({sMap, eMap});
    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.distance(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

float TravelMgr::fastMapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.GetMapId();
    uint32 eMap = end.GetMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({sMap, eMap});

    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.fDist(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

QuestTravelDestination::QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1)
    : TravelDestination(radiusMin1, radiusMax1)
{
    questId = questId1;
    questTemplate = sObjectMgr->GetQuestTemplate(questId);
}

bool QuestTravelDestination::isActive(Player* bot) { return bot->IsActiveQuest(questId); }

bool QuestObjectiveTravelDestination::isCreature() { return GetQuestTemplate()->RequiredNpcOrGo[objective] > 0; }

uint32 QuestObjectiveTravelDestination::ReqCreature()
{
    return isCreature() ? GetQuestTemplate()->RequiredNpcOrGo[objective] : 0;
}

uint32 QuestObjectiveTravelDestination::ReqGOId()
{
    return !isCreature() ? abs(GetQuestTemplate()->RequiredNpcOrGo[objective]) : 0;
}

uint32 QuestObjectiveTravelDestination::ReqCount() { return GetQuestTemplate()->RequiredNpcOrGoCount[objective]; }

void TravelMgr::printGrid(uint32 mapId, int x, int y, std::string const type)
{
    std::string const fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.HasLog(fileName))
    {
        WorldPosition p = WorldPosition(mapId, 0, 0, 0, 0);

        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr();
        out << "+00, " << 0 << 0 << x << "," << y << ", " << type << ",";
        p.printWKT(p.fromGridCoord(GridCoord(x, y)), out, 1, true);
        sPlayerbotAIConfig.Log(fileName, out.str().c_str());
    }
}

void TravelMgr::printObj(WorldObject* obj, std::string const type)
{
    std::string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.HasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());

        std::vector<WorldPosition> vcell, vgrid;
        vcell = p.fromCellCoord(p.getCellCoord());
        vgrid = p.gridFromCellCoord(p.getCellCoord());

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vcell, out, 1, true);
            sPlayerbotAIConfig.Log(fileName, out.str().c_str());
        }

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vgrid, out, 1, true);
            sPlayerbotAIConfig.Log(fileName, out.str().c_str());
        }
    }

    fileName = "unload_obj.csv";

    if (sPlayerbotAIConfig.HasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT({p}, out, 0);
            sPlayerbotAIConfig.Log(fileName, out.str().c_str());
        }
    }
}

void TravelMgr::Init()
{
    if (sPlayerbotAIConfig.Enabled)
    {
        if (!destinationCachesReady)
        {
            PrepareZone2LevelBracket();
            PrepareDestinationCache();

            if (sPlayerbotAIConfig.ProbTeleToQuestGivers > 0.0f)
                PrepareQuestGiverTeleportIndex();
            else
                questGiverTeleportIndex.clear();

            destinationCachesReady = true;
        }
    }
    sTravelNodeMap.Init();
}

TravelMgr::FlightMasterInfo const* TravelMgr::GetNearestFlightMasterInfo(Player* bot) const
{
    auto const& flightMasterCache =
        (bot->GetTeamId() == TEAM_ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    FlightMasterInfo const* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto const& [dbGuid, info] : flightMasterCache)
    {
        if (info.pos.GetMapId() != bot->GetMapId())
            continue;

        float distance = bot->GetExactDist2dSq(info.pos);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = &info;
        }
    }

    return nearest;
}

std::vector<uint32> TravelMgr::GetFlightNodesInZone(uint32 zoneId, TeamId team, uint32 excludeNode) const
{
    auto const& cache = (team == TEAM_ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;
    std::unordered_set<uint32> seen;
    std::vector<uint32> result;
    for (auto const& [entry, info] : cache)
    {
        if (info.zoneId != zoneId || info.taxiNodeId == 0 || info.taxiNodeId == excludeNode)
            continue;
        if (seen.insert(info.taxiNodeId).second)
            result.push_back(info.taxiNodeId);
    }
    return result;
}

std::vector<std::vector<uint32>> TravelMgr::GetOptimalFlightDestinations(Player* bot)
{
    std::vector<std::vector<uint32>> validDestinations;

    FlightMasterInfo const* nearestFlightMaster = GetNearestFlightMasterInfo(bot);
    if (!nearestFlightMaster)
        return validDestinations;

    uint32 fromNode = nearestFlightMaster->taxiNodeId;
    if (!fromNode)
        return validDestinations;

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    if (!startNode)
        return validDestinations;

    uint32 botLevel = bot->GetLevel();

    // Bots already in a capital shouldn't have another capital picked as a
    // flight destination — that just shuffles them between cities.
    bool botInCapital = false;
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetZoneId()))
        botInCapital = (area->flags & AREA_FLAG_CAPITAL) != 0;

    std::vector<uint32> candidateZones;
    if (botLevel >= 10 && !botInCapital &&
        urand(0, 100) < sPlayerbotAIConfig.ProbTeleToBankers * 100)
    {
        TeamId botTeam = bot->GetTeamId();
        for (Capital const& capital : capitals)
        {
            if (capital.team != TEAM_NEUTRAL && capital.team != botTeam)
                continue;
            candidateZones.push_back(capital.zoneId);
        }
    }
    if (candidateZones.empty())
    {
        for (auto const& [zoneId, bracket] : zone2LevelBracket)
        {
            if (botLevel < bracket.low || botLevel > bracket.high)
                continue;
            if (GetFlightNodesInZone(zoneId, bot->GetTeamId(), fromNode).empty())
                continue;
            candidateZones.push_back(zoneId);
        }
    }

    if (candidateZones.empty())
        return validDestinations;

    while (!candidateZones.empty())
    {
        uint32 zoneIndex = urand(0, candidateZones.size() - 1);
        uint32 pickedZone = candidateZones[zoneIndex];

        std::vector<uint32> usableNodes = GetFlightNodesInZone(pickedZone, bot->GetTeamId(), fromNode);

        if (!usableNodes.empty())
        {
            uint32 pickedNode = usableNodes[urand(0, usableNodes.size() - 1)];
            std::vector<uint32> path = sTravelNodeMap.FindTaxiPath(fromNode, pickedNode);
            if (!path.empty())
            {
                validDestinations.push_back(std::move(path));
                return validDestinations;
            }
        }

        candidateZones.erase(candidateZones.begin() + zoneIndex);
    }

    return validDestinations;
}

const std::vector<WorldLocation> TravelMgr::GetTeleportLocations(Player* bot)
{
    uint32 level = bot->GetLevel();
    std::map<uint8, std::vector<WorldLocation>> const& cache =
        !sPlayerbotAIConfig.EnableNewRpgStrategy ? locsPerLevelCache
        : bot->GetTeamId() == TEAM_ALLIANCE      ? allianceHubsPerLevelCache
                                                 : hordeHubsPerLevelCache;

    // Called from map threads: find() must not insert.
    auto itr = cache.find(static_cast<uint8>(level));
    return itr != cache.end() ? itr->second : std::vector<WorldLocation>();
}

const std::vector<WorldLocation> TravelMgr::GetTravelHubs(Player* bot)
{
    std::vector<WorldLocation> locs = bot->GetTeamId() == TEAM_ALLIANCE
                                                 ? allianceHubsPerLevelCache[bot->GetLevel()]
                                                 : hordeHubsPerLevelCache[bot->GetLevel()];
    return locs;
}

std::vector<WorldLocation> TravelMgr::GetCityLocations(Player* bot)
{
    uint32 level = bot->GetLevel();

    std::vector<WorldLocation> fallbackLocations;
    for (auto& bLoc : bankerLocsPerLevelCache[level])
        fallbackLocations.push_back(bLoc.loc);

    if (!sPlayerbotAIConfig.EnableWeightTeleToCityBankers)
        return fallbackLocations;

    TeamId botTeamId = bot->GetTeamId();
    std::unordered_set<uint32> validBankerCities;
    for (auto& loc : bankerLocsPerLevelCache[level])
    {
        Capital const* capital = FindCapitalByBanker(loc.entry);
        if (!capital)
            continue;

        TeamId cityTeamId = capital->team;

        if (cityTeamId == botTeamId ||
            (cityTeamId == TEAM_NEUTRAL)
           )
            validBankerCities.insert(capital->zoneId);
    }
    // Fallback if no valid cities
    if (validBankerCities.empty())
        return fallbackLocations;

    // Apply weights to valid cities
    std::vector<uint32> weightedCities;
    for (uint32 zoneId : validBankerCities)
    {
        int weight = GetCityWeight(zoneId);
        if (weight <= 0)
            continue;

        for (int i = 0; i < weight; ++i)
            weightedCities.push_back(zoneId);
    }

    // Fallback if no valid cities
    if (weightedCities.empty())
        return fallbackLocations;

    // Pick a weighted city randomly, then a random banker in that city
    uint32 selectedCity = weightedCities[urand(0, weightedCities.size() - 1)];
    Capital const* selectedCapital = FindCapitalByZone(selectedCity);
    if (!selectedCapital)
        return fallbackLocations;
    auto const& bankers = selectedCapital->bankers;
    uint32 selectedBankerEntry = bankers[urand(0, bankers.size() - 1)];
    auto locIt = bankerEntryToLocation.find(selectedBankerEntry);
    if (locIt != bankerEntryToLocation.end())
        return { locIt->second };
    // Fallback if something went wrong
    return fallbackLocations;
}

std::vector<WorldLocation> TravelMgr::GetValidQuestGiverLocations(Player* bot)
{
    std::vector<WorldLocation> out;

    uint32 level = std::min<uint32>(bot->GetLevel(), sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
    auto levelIt = questGiverTeleportIndex.find(level);
    if (levelIt == questGiverTeleportIndex.end())
        return out;

    auto teamIt = levelIt->second.find(bot->GetTeamId());
    if (teamIt == levelIt->second.end())
        return out;

    int32 botLevel = (int32)bot->GetLevel();
    uint32 raceMask = bot->getRaceMask();
    uint32 classMask = bot->getClassMask();
    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);

    for (auto const& cand : teamIt->second)
    {
        int32 effQuestLevel = (cand.questLevel > 0) ? cand.questLevel : (int32)botLevel;
        if (botLevel + QUEST_MAX_LEVELS_ABOVE_BOT < effQuestLevel)
            continue;
        // A negative hide-diff disables the low-level filter (core convention).
        if (lowLevelDiff >= 0 && botLevel > effQuestLevel + lowLevelDiff)
            continue;
        if (cand.allowableRaces && !(cand.allowableRaces & raceMask))
            continue;
        if (cand.requiredClasses && !(cand.requiredClasses & classMask))
            continue;

        auto const* quest = sObjectMgr->GetQuestTemplate(cand.questId);
        if (!quest)
            continue;
        if (bot->GetQuestStatus(cand.questId) != QUEST_STATUS_NONE)
            continue;
        if (!bot->CanTakeQuest(quest, false) || !bot->CanAddQuest(quest, false))
            continue;

        // The team bucket only guarantees the right side: neutral and friendly spawns are both
        // valid, hostile ones are skipped.
        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(cand.giverEntry);
        if (creatureTemplate)
        {
            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
            if (factionEntry &&
                Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(), factionEntry) <= REP_UNFRIENDLY)
                continue;
        }

        out.push_back(cand.loc);
    }

    return out;
}

void TravelMgr::PrepareZone2LevelBracket()
{
    // Classic WoW - starter zones
    zone2LevelBracket[AREA_DUN_MOROGH]        = {5, 12};
    zone2LevelBracket[AREA_ELWYNN_FOREST]     = {5, 12};
    zone2LevelBracket[AREA_DUROTAR]           = {5, 12};
    zone2LevelBracket[AREA_TIRISFAL_GLADES]   = {5, 12};
    zone2LevelBracket[AREA_TELDRASSIL]        = {5, 12};
    zone2LevelBracket[AREA_MULGORE]           = {5, 12};
    zone2LevelBracket[AREA_EVERSONG_WOODS]    = {5, 12};
    zone2LevelBracket[AREA_AZUREMYST_ISLE]    = {5, 12};

    // Classic WoW - low level zones
    zone2LevelBracket[AREA_THE_BARRENS]       = {10, 25};
    zone2LevelBracket[AREA_LOCH_MODAN]        = {10, 20};
    zone2LevelBracket[AREA_WESTFALL]          = {10, 21};
    zone2LevelBracket[AREA_SILVERPINE_FOREST] = {10, 23};
    zone2LevelBracket[AREA_DARKSHORE]         = {10, 21};
    zone2LevelBracket[AREA_GHOSTLANDS]        = {10, 22};
    zone2LevelBracket[AREA_BLOODMYST_ISLE]    = {10, 21};

    // Classic WoW - mid-level zones
    zone2LevelBracket[AREA_DUSKWOOD]             = {19, 33};
    zone2LevelBracket[AREA_WETLANDS]             = {21, 30};
    zone2LevelBracket[AREA_REDRIDGE_MOUNTAINS]   = {16, 28};
    zone2LevelBracket[AREA_HILLSBRAD_FOOTHILLS]  = {20, 34};
    zone2LevelBracket[AREA_ASHENVALE]            = {18, 33};
    zone2LevelBracket[AREA_THOUSAND_NEEDLES]     = {24, 36};
    zone2LevelBracket[AREA_STONETALON_MOUNTAINS] = {16, 29};

    // Classic WoW - 30-52 zones
    zone2LevelBracket[AREA_BADLANDS]            = {36, 46};
    zone2LevelBracket[AREA_SWAMP_OF_SORROWS]    = {36, 46};
    zone2LevelBracket[AREA_DUSTWALLOW_MARSH]    = {35, 46};
    zone2LevelBracket[AREA_AZSHARA]             = {45, 52};
    zone2LevelBracket[AREA_STRANGLETHORN_VALE]  = {32, 47};
    zone2LevelBracket[AREA_ARATHI_HIGHLANDS]    = {30, 42};
    zone2LevelBracket[AREA_THE_HINTERLANDS]     = {42, 51};
    zone2LevelBracket[AREA_SEARING_GORGE]       = {45, 51};
    zone2LevelBracket[AREA_FERALAS]             = {40, 52};
    zone2LevelBracket[AREA_DESOLACE]            = {30, 41};
    zone2LevelBracket[AREA_TANARIS]             = {41, 52};

    // Classic WoW - top level zones
    zone2LevelBracket[AREA_BLASTED_LANDS]        = {52, 57};
    zone2LevelBracket[AREA_WESTERN_PLAGUELANDS]  = {50, 60};
    zone2LevelBracket[AREA_BURNING_STEPPES]      = {51, 60};
    zone2LevelBracket[AREA_EASTERN_PLAGUELANDS]  = {54, 62};
    zone2LevelBracket[361]                       = {47, 57}; // Felwood (no AREA_ define)
    zone2LevelBracket[490]                       = {49, 56}; // Un'Goro Crater (no AREA_ define)
    zone2LevelBracket[AREA_WINTERSPRING]         = {54, 61};
    zone2LevelBracket[AREA_SILITHUS]             = {54, 63};

    // The Burning Crusade zones
    zone2LevelBracket[AREA_HELLFIRE_PENINSULA]    = {58, 66};
    zone2LevelBracket[AREA_NAGRAND]               = {64, 70};
    zone2LevelBracket[AREA_TEROKKAR_FOREST]       = {62, 73};
    zone2LevelBracket[AREA_SHADOWMOON_VALLEY]     = {66, 73};
    zone2LevelBracket[AREA_ZANGARMARSH]           = {60, 67};
    zone2LevelBracket[AREA_BLADES_EDGE_MOUNTAINS] = {64, 73};
    zone2LevelBracket[AREA_NETHERSTORM]           = {67, 73};
    zone2LevelBracket[AREA_ISLE_OF_QUEL_DANAS]    = {68, 73};

    // Wrath of the Lich King zones
    zone2LevelBracket[AREA_DRAGONBLIGHT]        = {71, 77};
    zone2LevelBracket[AREA_ZUL_DRAK]            = {74, 80};
    zone2LevelBracket[AREA_THE_STORM_PEAKS]     = {77, 80};
    zone2LevelBracket[210]                      = {77, 80}; // Icecrown Glacier (no AREA_ define)
    zone2LevelBracket[AREA_GRIZZLY_HILLS]       = {72, 78};
    zone2LevelBracket[AREA_HOWLING_FJORD]       = {68, 74};
    zone2LevelBracket[AREA_CRYSTALSONG_FOREST]  = {77, 80};
    zone2LevelBracket[AREA_BOREAN_TUNDRA]       = {68, 75};
    zone2LevelBracket[AREA_SHOLAZAR_BASIN]      = {75, 80};
    zone2LevelBracket[AREA_WINTERGRASP]         = {79, 80};

    // Override with values from config
    for (auto const& [zoneId, bracketPair] : sPlayerbotAIConfig.ZoneBrackets)
        zone2LevelBracket[zoneId] = {bracketPair.first, bracketPair.second};
}

void TravelMgr::PrepareDestinationCache()
{
    allianceFlightMasterCache.clear();
    hordeFlightMasterCache.clear();
    allianceHubsPerLevelCache.clear();
    hordeHubsPerLevelCache.clear();
    bankerLocsPerLevelCache.clear();
    bankerEntryToLocation.clear();
    locsPerLevelCache.clear();
    creatureSpawnsByTemplate.clear();

    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    uint32 flightMastersCount = 0;
    uint32 innkeepersCount = 0;
    uint32 bankerCount = 0;

    LOG_INFO("playerbots", "Preparing destination caches for {} levels...", maxLevel);
    // Temporary map to group creatures by entry and area
    std::map<std::tuple<uint16, int32, int32, int32>, std::vector<CreatureData>> tempLocsCache;
    std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> tempCreatureCache;
    for (auto const& [guid, creatureData] : sObjectMgr->GetAllCreatureData())
    {
        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creatureData.id);
        if (!creatureTemplate)
            continue;

        uint16 mapId = creatureData.mapid;
        if (std::find(sPlayerbotAIConfig.RandomBotMaps.begin(), sPlayerbotAIConfig.RandomBotMaps.end(), mapId)
                      == sPlayerbotAIConfig.RandomBotMaps.end())
            continue;

        float x = creatureData.posX;
        float y = creatureData.posY;
        float z = creatureData.posZ;
        float orient = creatureData.orientation;
        uint32 templateEntry = creatureData.id;

        Map* map = sMapMgr->FindMap(mapId, 0);
        if (!map)
            continue;

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
        if (!area)
            continue;

        uint32 areaId = area->zone ? area->zone : area->ID;

        // CREATURES
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
            int32 roundX = static_cast<int32>(std::lround(x / 50.0f));
            int32 roundY = static_cast<int32>(std::lround(y / 50.0f));
            int32 roundZ = static_cast<int32>(std::lround(z / 50.0f));
            tempLocsCache[std::make_tuple(mapId, roundX, roundY, roundZ)].push_back(creatureData);
            tempCreatureCache[templateEntry][areaId].push_back(WorldLocation(mapId, x, y, z));
        }
        // FLIGHT MASTERS
        // Entry 29480 is Grimwing (Storm Peaks) — has FLIGHTMASTER flag but
        // isn't a real usable flight master; skip it.
        else if ((creatureTemplate->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER ||
                  creatureTemplate->npcflag & UNIT_NPC_FLAG_INNKEEPER) &&
                creatureTemplate->Entry != 29480)
        {
            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
            bool forHorde = !(factionEntry->hostileMask & 4);
            bool forAlliance = !(factionEntry->hostileMask & 2);

            if (creatureTemplate->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER)
            {
                WorldPosition pos(mapId, x, y, z, orient);
                if (forHorde)
                {
                    FlightMasterInfo info;
                    info.pos = pos;
                    info.zoneId = areaId;
                    info.taxiNodeId = sObjectMgr->GetNearestTaxiNode(x, y, z, mapId, TEAM_HORDE);
                    info.templateEntry = templateEntry;
                    info.dbGuid = guid;
                    hordeFlightMasterCache[guid] = info;
                }

                if (forAlliance)
                {
                    FlightMasterInfo info;
                    info.pos = pos;
                    info.zoneId = areaId;
                    info.taxiNodeId = sObjectMgr->GetNearestTaxiNode(x, y, z, mapId, TEAM_ALLIANCE);
                    info.templateEntry = templateEntry;
                    info.dbGuid = guid;
                    allianceFlightMasterCache[guid] = info;
                }
                flightMastersCount++;

                // Zones that have flight masters but no innkeepers — use flight master as hub
                static const std::set<uint32> zonesWithoutInnkeeper = {
                    AREA_BLASTED_LANDS,
                    AREA_AZSHARA,
                    AREA_WESTERN_PLAGUELANDS,
                    AREA_BURNING_STEPPES,
                    AREA_SEARING_GORGE,
                    361,  // Felwood (47-57)
                    490,  // Un'Goro Crater (49-56)
                    AREA_CRYSTALSONG_FOREST,
                    AREA_WINTERGRASP
                };
                if (zonesWithoutInnkeeper.count(areaId))
                {
                    LevelBracket bracket = zone2LevelBracket[areaId];
                    WorldPosition loc(mapId, x + cos(orient) * 5.0f, y + sin(orient) * 5.0f, z + 0.5f, orient + M_PI);
                    for (uint32 i = bracket.low; i <= bracket.high; i++)
                    {
                        if (forHorde)
                            hordeHubsPerLevelCache[i].push_back(loc);
                        if (forAlliance)
                            allianceHubsPerLevelCache[i].push_back(loc);
                    }
                }
            }
            else if (creatureTemplate->npcflag & UNIT_NPC_FLAG_INNKEEPER)
            {
                if (zone2LevelBracket.find(areaId) == zone2LevelBracket.end())
                    continue;

                LevelBracket bracket = zone2LevelBracket[areaId];
                WorldPosition loc(mapId, x + cos(orient) * 5.0f, y + sin(orient) * 5.0f, z + 0.5f, orient + M_PI);
                for (uint32 i = bracket.low; i <= bracket.high; i++)
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
            NpcLocation bLoc;
            bLoc.loc = WorldLocation(mapId, x + cos(orient) * 6.0f, y + sin(orient) * 6.0f, z + 2.0f, orient + M_PI);
            bLoc.entry = templateEntry;
            uint32 level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;
            for (uint32 l = 1; l <= maxLevel; l++)
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

    // Process temporary caches
    for (auto const& [gridTuple, creatureDataList] : tempLocsCache)
    {
        if (creatureDataList.size() >= 2)
        {
            CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creatureDataList[0].id);
            uint32 level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;

            float totalX = 0.0f;
            float totalY = 0.0f;
            float totalZ = 0.0f;
            for (CreatureData const& creatureData : creatureDataList)
            {
                totalX += creatureData.posX;
                totalY += creatureData.posY;
                totalZ += creatureData.posZ;
            }

            float avgX = totalX / creatureDataList.size();
            float avgY = totalY / creatureDataList.size();
            float avgZ = totalZ / creatureDataList.size();
            uint32 mapId = std::get<0>(gridTuple);

            for (int32 l = (int32)level - (int32)sPlayerbotAIConfig.RandomBotTeleLowerLevel;
                 l <= (int32)level + (int32)sPlayerbotAIConfig.RandomBotTeleHigherLevel; l++)
            {
                if (l < 1 || l > int32(maxLevel))
                    continue;

                locsPerLevelCache[(uint8)l].push_back(WorldLocation(mapId, avgX, avgY, avgZ, 0.0f));
            }
        }
    }
    for (auto const& [entry, areaMap] : tempCreatureCache)
    {
        for (auto const& [area, locList] : areaMap)
        {
            if (locList.size() > 3)
                continue;

            float totalX = 0, totalY = 0, totalZ = 0;
            for (auto const& loc : locList)
            {
                totalX += loc.GetPositionX();
                totalY += loc.GetPositionY();
                totalZ += loc.GetPositionZ();
            }
            float avgX = totalX / locList.size();
            float avgY = totalY / locList.size();
            float avgZ = totalZ / locList.size();
            creatureSpawnsByTemplate[entry].push_back(WorldLocation(locList[0].GetMapId(), avgX, avgY, avgZ, 0));
        }
    }
    // Add travel hubs based on player start locations
    for (uint32 i = 1; i < sRaceMgr->GetMaxRaces(); i++)
    {
        for (uint32 j = 1; j < MAX_CLASSES; j++)
        {
            PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);

            if (!info)
                continue;

            WorldPosition pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            for (int32 l = 1; l <= 5; l++)
            {
                if ((1 << (i - 1)) & sRaceMgr->GetAllianceRaceMask())
                    allianceHubsPerLevelCache[(uint8)l].push_back(pos);
                else
                    hordeHubsPerLevelCache[(uint8)l].push_back(pos);
            }
            break;
        }
    }

    for (uint32 level = 1; level <= maxLevel; ++level)
    {
        uint8 const key = static_cast<uint8>(level);
        locsPerLevelCache.try_emplace(key);
        allianceHubsPerLevelCache.try_emplace(key);
        hordeHubsPerLevelCache.try_emplace(key);
        bankerLocsPerLevelCache.try_emplace(key);
    }

    LOG_INFO("playerbots", ">> {} flight masters, {} innkeepers, {} bankers collected.", flightMastersCount, innkeepersCount, bankerCount);
}

void TravelMgr::PrepareQuestGiverTeleportIndex()
{
    questGiverTeleportIndex.clear();

    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    int32 const window = QUEST_MAX_LEVELS_ABOVE_BOT;
    uint32 candidateCount = 0;

    // Spawns tagged for a positive game event sit in the upper half of GameEventCreatureGuids and
    // only exist while the event runs (e.g. the AQ War Effort givers). `creature_queststarter` and
    // `CanTakeQuest` ignore event state, so without this they would be indexed all year.
    std::unordered_set<ObjectGuid::LowType> eventSpawns;
    auto const& eventCreatureGuids = sGameEventMgr->GameEventCreatureGuids;
    for (size_t i = sGameEventMgr->GetEventMap().size(); i < eventCreatureGuids.size(); ++i)
        eventSpawns.insert(eventCreatureGuids[i].begin(), eventCreatureGuids[i].end());

    for (auto const& [guid, creData] : sObjectMgr->GetAllCreatureData())
    {
        if (eventSpawns.count(guid))
            continue;

        if (std::find(sPlayerbotAIConfig.RandomBotMaps.begin(), sPlayerbotAIConfig.RandomBotMaps.end(),
                      creData.mapid) == sPlayerbotAIConfig.RandomBotMaps.end())
            continue;

        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creData.id);
        if (!creatureTemplate)
            continue;

        auto const& relations = sObjectMgr->GetCreatureQuestRelationBounds(creData.id);
        if (relations.first == relations.second)
            continue;

        // Team filter from the giver's hostileMask (same idiom as PrepareDestinationCache).
        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
        if (!factionEntry)
            continue;  // invalid faction template - skip defensively
        bool forAlliance = !(factionEntry->hostileMask & 2);
        bool forHorde = !(factionEntry->hostileMask & 4);
        if (!forAlliance && !forHorde)
            continue;

        for (auto it = relations.first; it != relations.second; ++it)
        {
            auto const* quest = sObjectMgr->GetQuestTemplate(it->second);
            if (!quest)
                continue;

            // Static worth/capable parts are baked out; level and core checks run at call time.
            if (quest->IsRepeatable() || quest->IsSeasonal())
                continue;
            if (quest->GetType() != 0 || quest->GetSuggestedPlayers() >= 2)
                continue;

            QuestGiverTeleportCandidate cand;
            cand.questId = it->second;
            cand.giverEntry = creData.id;

            // ~5y in front of the spawn, facing it (same offset idiom as the other hubs).
            float o = creData.orientation;
            cand.loc = WorldLocation(creData.mapid,
                                     creData.posX + cos(o) * 5.0f,
                                     creData.posY + sin(o) * 5.0f,
                                     creData.posZ + 0.5f,
                                     o + M_PI);

            int32 qLevel = quest->GetQuestLevel();
            cand.questLevel = (qLevel > 0) ? (uint16)qLevel : 0;
            cand.allowableRaces = quest->GetAllowableRaces();
            cand.requiredClasses = quest->GetRequiredClasses();

            // Unknown quest levels span to maxLevel and are pruned at call time by CanTakeQuest.
            uint32 lo = quest->GetMinLevel() > 0 ? quest->GetMinLevel() : 1;
            int32 hi = (qLevel > 0) ? (qLevel + window) : (int32)maxLevel;
            for (uint32 level = lo; level <= maxLevel && (int32)level <= hi; ++level)
            {
                if (forAlliance)
                {
                    questGiverTeleportIndex[level][TEAM_ALLIANCE].push_back(cand);
                    ++candidateCount;
                }
                if (forHorde)
                {
                    questGiverTeleportIndex[level][TEAM_HORDE].push_back(cand);
                    ++candidateCount;
                }
            }
        }
    }

    LOG_INFO("playerbots", ">> Prepared quest-giver teleport index: {} candidates across {} levels.",
             candidateCount, questGiverTeleportIndex.size());
}
