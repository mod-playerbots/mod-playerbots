/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELNODE_H
#define _PLAYERBOT_TRAVELNODE_H

#include <shared_mutex>

#include "G3D/Vector3.h"
#include "TravelMgr.h"

// THEORY
//
//  Pathfinding in (c)mangos is based on detour recast, an opensource navmesh creation and pathfinding codebase.
//  This system is used for mob and npc pathfinding and in this codebase also for bots.
//  Because mobs and npc movement is based on following a player or a set path the PathGenerator is limited to 296y.
//  This means that when trying to find a path from A to B distances beyond 296y will be a best guess often moving in a
//  straight path. Bots would get stuck moving from Northshire to Stormwind because there is no 296y path that doesn't
//  go (initially) the wrong direction.
//
//  To remedy this limitation without altering the PathGenerator limits too much this node system was introduced.
//
//   <S> ---> [N1] ---> [N2] ---> [N3] ---> <E>
//
//  Bot at <S> wants to move to <E>
//  [N1],[N2],[N3] are predefined nodes for which we know we can move from [N1] to [N2] and from [N2] to [N3] but not
//  from [N1] to [N3]. If we can move from [S] to [N1] and from [N3] to [E] we have a complete route to travel.
//
//  Terminology:
//  Node:  A location on a map for which we know bots are likely to want to travel to or need to travel past to reach
//         other nodes. Stored in DB table `playerbots_travelnode`.
//  Link:  The connection between two nodes. A link signifies that the bot can travel from one node to another.
//         A link is one-directional. Stored in `playerbots_travelnode_link`.
//  Path:  The waypoint path returned by the standard PathGenerator to move from one node (or position) to another.
//         A path can be incomplete or empty which means there is no link. Stored in `playerbots_travelnode_path`.
//  Route: The list of nodes that give the shortest route from a node to a distant node. Routes are calculated using
//         a standard A* search based on links.
//
//  Edge types (TravelNodePathType):
//    walk(1)          — Walk via navmesh waypoints (stored in DB)
//    portal(2)        — AreaTrigger teleport (auto-discovered at startup)
//    transport(3)     — Boat/zeppelin (auto-discovered from MO_TRANSPORT)
//    flightPath(4)    — Taxi flight between flight masters
//    teleportSpell(5) — Spell-based teleport (e.g. mage portals)
//    staticPortal(6)  — Manually defined teleport link (DB only, not pruned by generation)
//    flyingMount (7)  — Use Bots Flying mount to travel (Not currently enabled)
//
//  On server start saved nodes and links are loaded via TravelNodeMap::Init(). An index of nodes by zone is prepared
//  (instead of scanning all ~4000 nodes), precomputes connected components for O(1) reachability checks, and builds
//  a taxi BFS graph. Paths and routes are calculated on the fly and saved for future use. Nodes are only added at
//  startup or via the console `.generate` command — runtime mutation was removed because taking a unique_lock
//  caused 100-250ms contention spikes against bot threads.
//
//  Initially the current nodes have been made:
//  Flightmasters and Inns (Bots can use these to fast-travel so eventually they will be included in the route
//  calculation) WorldBosses and Unique bosses in instances (These are logical places bots might want to go in
//  instances) Player start spawns (Obviously all lvl1 bots will spawn and move from here) Area triggers locations with
//  teleport and their teleport destinations (These used to travel in or between maps) Transports including elevators
//  (Again used to travel in and in maps) (sub)Zone means (These are the center most point for each sub-zone which is
//  good for global coverage).
//
//  To increase coverage/linking extra nodes must be manually created via the "playerbot travel generatenode"
//  console command after importing the specified node. Current implementation places nodes on paths (including
//  complete) at sub-zone transitions or randomly. After calculating possible links the node is removed if it
//  does not create local coverage (.fullgenerate only).
//
//  Travel Flow:
//
//  GetFullPath finds nearest nodes (zone-indexed), runs A* to get a node route, then
//  BuildPath assembles a flat TravelPath with typed waypoints (walk, portal, transport, flight).
//  ExecuteTravelPlan iterates the path by stepIdx, dispatching on each point's PathNodeType.
//  Cross-map travel is handled naturally by portal/transport edges in the A* graph.
//
//  If setup cannot resolve (no node, no route, no flight), the bot teleports directly to the destination
//  as a fallback.
//
//  The use of hearthstones and mage teleporting was removed — it caused route mutations requiring locking that no longer made sense. Mage portals may be future item.
//
//  Thread Safety:
//
//  The node graph is immutable at runtime (no adds/removes after Init). A shared_timed_mutex (m_nMapMtx) still
//  exists and shared_locks are taken in GetFullPath and GenerateWalkPath for safety, but since there are no
//  runtime mutations these are effectively uncontested. The only exclusive locks are taken at startup
//  (saveNodeStore) and by the debug dump command.
//

constexpr float MAX_PATHFINDING_DISTANCE = 296.0f;

enum class TravelNodePathType : uint8
{
    none = 0,
    walk = 1,
    portal = 2,
    transport = 3,
    flightPath = 4,
    teleportSpell = 5,
    staticPortal = 6,
    flyingMount = 7
};

// A connection between two nodes.
class TravelNodePath
{
public:
    // Constructor
    TravelNodePath(float distance = 0.1f, float extraCost = 0,
                   uint8 pathType = (uint8)TravelNodePathType::walk,
                   uint32 pathObject = 0, bool calculated = false,
                   std::vector<uint8> maxLevelCreature = {0, 0, 0},
                   float swimDistance = 0)
        : extraCost(extraCost),
          calculated(calculated),
          distance(distance),
          maxLevelCreature(maxLevelCreature),
          swimDistance(swimDistance),
          pathType(TravelNodePathType(pathType)),
          pathObject(pathObject)  // reorder args - whipowill
    {
        if (pathType != (uint8)TravelNodePathType::walk)
            complete = true;
    }

    TravelNodePath(TravelNodePath* basePath)
    {
        complete = basePath->complete;
        path = basePath->path;
        extraCost = basePath->extraCost;
        calculated = basePath->calculated;
        distance = basePath->distance;
        maxLevelCreature = basePath->maxLevelCreature;
        swimDistance = basePath->swimDistance;
        pathType = basePath->pathType;
        pathObject = basePath->pathObject;
    }

    // Getters
    bool getComplete() { return complete || pathType != TravelNodePathType::walk; }
    std::vector<WorldPosition> GetPath() { return path; }

    TravelNodePathType getPathType() { return pathType; }
    uint32 getPathObject() { return pathObject; }

    float getDistance() { return distance; }
    float getSwimDistance() { return swimDistance; }
    float getExtraCost() { return extraCost; }
    std::vector<uint8> getMaxLevelCreature() { return maxLevelCreature; }

    void setCalculated(bool calculated1 = true) { calculated = calculated1; }

    bool getCalculated() { return calculated; }

    std::string const print();

    // Setters
    void setComplete(bool complete1) { complete = complete1; }

    void setPath(std::vector<WorldPosition> path1) { path = path1; }

    void setPathAndCost(std::vector<WorldPosition> path1, float speed)
    {
        setPath(path1);
        calculateCost(true);
        extraCost = distance / speed;
    }

    void setPathType(TravelNodePathType pathType1) { pathType = pathType1; }

    void setPathObject(uint32 pathObject1) { pathObject = pathObject1; }

    void calculateCost(bool distanceOnly = false);

    float getCost(Player* bot = nullptr, uint32 cGold = 0);
    uint32 getPrice();

private:
    // Does the path have all the points to get to the destination?
    bool complete = false;

    // List of WorldPositions to get to the destination.
    std::vector<WorldPosition> path = {};

    // The extra (loading/transport) time it takes to take this path.
    float extraCost = 0;

    bool calculated = false;

    // Derived distance in yards
    float distance = 0.1f;

    // Calculated mobs level along the way.
    std::vector<uint8> maxLevelCreature = {0, 0, 0};  // mobs, horde, alliance

    // Calculated swiming distances along the way.
    float swimDistance = 0;

    TravelNodePathType pathType = TravelNodePathType::walk;
    uint32 pathObject = 0;

    /*
    //Is the path a portal/teleport to the destination?
    bool portal = false;
    //Area trigger Id
    uint32 portalId = 0;

    //Is the path transport based?
    bool transport = false;

    // Is the path a flightpath?
    bool flightPath = false;
    */
};

// A waypoint to travel from or to.
// Each node knows which other nodes can be reached without help.
class TravelNode
{
public:
    // Constructors
    TravelNode() {}

    TravelNode(WorldPosition point1, std::string const nodeName1 = "Travel Node",
               bool important1 = false)
    {
        nodeName = nodeName1;
        point = point1;
        important = important1;
    }

    TravelNode(TravelNode* baseNode)
    {
        nodeName = baseNode->nodeName;
        point = baseNode->point;
        important = baseNode->important;
    }

    // Setters
    void setLinked(bool linked1) { linked = linked1; }
    void setPoint(WorldPosition point1) { point = point1; }

    // Getters
    std::string const getName() { return nodeName; }
    WorldPosition* getPosition() { return &point; }
    std::unordered_map<TravelNode*, TravelNodePath>* getPaths() { return &paths; }
    std::unordered_map<TravelNode*, TravelNodePath*>* getLinks() { return &links; }
    bool isImportant() { return important; }
    bool isLinked() { return linked; }

    bool isTransport()
    {
        for (auto const& link : *getLinks())
            if (link.second->getPathType() == TravelNodePathType::transport)
                return true;

        return false;
    }

    uint32 getTransportId()
    {
        for (auto const& link : *getLinks())
            if (link.second->getPathType() == TravelNodePathType::transport)
                return link.second->getPathObject();

        return false;
    }

    bool isPortal()
    {
        for (auto const& link : *getLinks())
            if (link.second->getPathType() == TravelNodePathType::portal ||
                link.second->getPathType() == TravelNodePathType::staticPortal)
                return true;

        return false;
    }

    bool isWalking()
    {
        for (auto link : *getLinks())
            if (link.second->getPathType() == TravelNodePathType::walk)
                return true;

        return false;
    }

    // WorldLocation shortcuts
    uint32 GetMapId() { return point.GetMapId(); }
    float getX() { return point.GetPositionX(); }
    float getY() { return point.GetPositionY(); }
    float getZ() { return point.GetPositionZ(); }
    float getO() { return point.GetOrientation(); }
    float getDistance(WorldPosition pos) { return point.distance(pos); }
    float getDistance(TravelNode* node)
    {
        return point.distance(node->getPosition());
    }
    float fDist(TravelNode* node)
    {
        return point.fDist(node->getPosition());
    }
    float fDist(WorldPosition pos) { return point.fDist(pos); }

    TravelNodePath* setPathTo(TravelNode* node,
                              TravelNodePath path = TravelNodePath(),
                              bool isLink = true)
    {
        if (this != node)
        {
            paths[node] = path;
            if (isLink)
                links[node] = &paths[node];

            return &paths[node];
        }

        return nullptr;
    }

    bool hasPathTo(TravelNode* node)
    {
        return paths.find(node) != paths.end();
    }
    TravelNodePath* getPathTo(TravelNode* node)
    {
        return &paths[node];
    }
    bool hasCompletePathTo(TravelNode* node)
    {
        return hasPathTo(node) && getPathTo(node)->getComplete();
    }
    TravelNodePath* BuildPath(TravelNode* endNode, Unit* bot,
                              bool postProcess = false);

    void setLinkTo(TravelNode* node, float distance = 0.1f)
    {
        if (this != node)
        {
            if (!hasPathTo(node))
                setPathTo(node, TravelNodePath(distance));
            else
                links[node] = &paths[node];
        }
    }

    bool hasLinkTo(TravelNode* node)
    {
        return links.find(node) != links.end();
    }
    float linkCostTo(TravelNode* node)
    {
        return paths.find(node)->second.getDistance();
    }
    float linkDistanceTo(TravelNode* node)
    {
        return paths.find(node)->second.getDistance();
    }
    void removeLinkTo(TravelNode* node, bool removePaths = false);

    bool isEqual(TravelNode* compareNode);

    // Removes links to other nodes that can also be reached by passing another node.
    bool isUselessLink(TravelNode* farNode);
    void cropUselessLink(TravelNode* farNode);
    bool cropUselessLinks();

    // Returns all nodes that can be reached from this node.
    std::vector<TravelNode*> getNodeMap(bool importantOnly = false,
        std::vector<TravelNode*> ignoreNodes = {});

    // Checks if it is even possible to route to this node.
    bool hasRouteTo(TravelNode* node)
    {
        if (routes.empty())
            for (auto mNode : getNodeMap())
                routes[mNode] = true;

        return routes.find(node) != routes.end();
    }

    void clearRoutes() { routes.clear(); }
    void setRouteTo(TravelNode* node) { routes[node] = true; }

    void print(bool printFailed = true);

protected:
    // Logical name of the node
    std::string nodeName;
    // WorldPosition of the node.
    WorldPosition point;

    // List of paths to other nodes.
    std::unordered_map<TravelNode*, TravelNodePath> paths;
    // List of links to other nodes.
    std::unordered_map<TravelNode*, TravelNodePath*> links;

    // List of nodes and if there is 'any' route possible
    std::unordered_map<TravelNode*, bool> routes;

    // This node should not be removed
    bool important = false;

    // This node has been checked for nearby links
    bool linked = false;

    // This node is a (moving) transport.
    // bool transport = false;
    // Entry of transport.
    // uint32 transportId = 0;
};

// Route step type
enum class PathNodeType : uint8
{
    NODE_PREPATH = 0,
    NODE_PATH = 1,
    NODE_NODE = 2,
    NODE_PORTAL = 3,
    NODE_TRANSPORT = 4,
    NODE_FLIGHTPATH = 5,
    NODE_TELEPORT = 6,
    NODE_FLYING_MOUNT = 7
};

struct PathNodePoint
{
    WorldPosition point;
    PathNodeType type = PathNodeType::NODE_PATH;
    uint32 entry = 0;
};

// A complete list of points the bots has to walk to or teleport to.
class TravelPath
{
public:
    TravelPath() {}
    TravelPath(std::vector<PathNodePoint> fullPath1)
    {
        fullPath = fullPath1;
    }
    TravelPath(std::vector<WorldPosition> path,
               PathNodeType type = PathNodeType::NODE_PATH,
               uint32 entry = 0)
    {
        addPath(path, type, entry);
    }

    void addPoint(PathNodePoint point) { fullPath.push_back(point); }
    void addPoint(WorldPosition point,
                  PathNodeType type = PathNodeType::NODE_PATH,
                  uint32 entry = 0)
    {
        fullPath.push_back(PathNodePoint{point, type, entry});
    }
    void addPath(std::vector<WorldPosition> path,
                 PathNodeType type = PathNodeType::NODE_PATH,
                 uint32 entry = 0)
    {
        for (auto& p : path)
            fullPath.push_back(PathNodePoint{p, type, entry});
    }
    void addPath(std::vector<PathNodePoint> newPath)
    {
        fullPath.insert(fullPath.end(), newPath.begin(), newPath.end());
    }
    void clear() { fullPath.clear(); }

    bool empty() const { return fullPath.empty(); }
    size_t size() const { return fullPath.size(); }
    const PathNodePoint& operator[](size_t idx) const { return fullPath[idx]; }
    std::vector<PathNodePoint> GetPath() { return fullPath; }
    const std::vector<PathNodePoint>& GetPathRef() const { return fullPath; }
    WorldPosition getFront() { return fullPath.front().point; }
    WorldPosition getBack() { return fullPath.back().point; }

    std::vector<WorldPosition> getPointPath()
    {
        std::vector<WorldPosition> retVec;
        for (auto const& p : fullPath)
            retVec.push_back(p.point);
        return retVec;
    }

    bool makeShortCut(WorldPosition startPos, float maxDist);

    std::ostringstream const print();

private:
    std::vector<PathNodePoint> fullPath;
};

// An stored A* search that gives a complete route from one node to another.
class TravelNodeRoute
{
public:
    TravelNodeRoute() {}
    TravelNodeRoute(std::vector<TravelNode*> nodes1)
    {
        nodes = nodes1;
    }

    bool isEmpty() { return nodes.empty(); }

    bool hasNode(TravelNode* node)
    {
        return findNode(node) != nodes.end();
    }
    float getTotalDistance();

    std::vector<TravelNode*> getNodes() { return nodes; }

    TravelPath BuildPath(
        std::vector<WorldPosition> pathToStart = {},
        std::vector<WorldPosition> pathToEnd = {},
        Unit* bot = nullptr);

    std::ostringstream const print();

private:
    std::vector<TravelNode*>::iterator findNode(TravelNode* node)
    {
        return std::find(nodes.begin(), nodes.end(), node);
    }
    std::vector<TravelNode*> nodes;
};

// A node container to aid A* calculations with nodes.
class TravelNodeStub
{
public:
    TravelNodeStub(TravelNode* dataNode1) { dataNode = dataNode1; }

    TravelNode* dataNode;
    float totalCost = 0.0;
    float costFromStart = 0.0;
    float heuristic = 0.0;
    bool open = false;
    bool closed = false;
    TravelNodeStub* parent = nullptr;
    uint32 currentGold = 0;
};

struct TravelPlan
{
    WorldPosition destination;

    // Flat waypoint path built upfront by GetFullPath:
    TravelPath steps;
    uint32 stepIdx{0};

    // Spline scratch (used by executor):
    std::vector<G3D::Vector3> walkPoints;
    bool splineActive{false};
    uint32 splineStartTime{0};
    uint32 expectedDuration{0};

    // Taxi scratch:
    std::vector<uint32> route;

    bool IsActive() const { return !steps.empty(); }

    void Reset()
    {
        destination = WorldPosition();
        steps.clear();
        stepIdx = 0;
        walkPoints.clear();
        splineActive = false;
        splineStartTime = 0;
        expectedDuration = 0;
        route.clear();
    }
};

// The container of all nodes.
class TravelNodeMap
{
public:
    static TravelNodeMap& instance()
    {
        static TravelNodeMap instance;

        return instance;
    }

    TravelNode* addNode(WorldPosition pos,
                        std::string const preferedName = "Travel Node",
                        bool isImportant = false,
                        bool checkDuplicate = true,
                        bool transport = false,
                        uint32 transportId = 0);
    void removeNode(TravelNode* node);
    bool removeNodes()
    {
        if (m_nMapMtx.try_lock_for(std::chrono::seconds(10)))
        {
            for (auto& node : nodes)
                removeNode(node);

            m_nMapMtx.unlock();
            return true;
        }

        return false;
    }

    void fullLinkNode(TravelNode* startNode, Unit* bot);

    // Get all nodes
    std::vector<TravelNode*> getNodes() { return nodes; }
    std::vector<TravelNode*> getNodes(WorldPosition pos, float range = -1);

    // Find nearest node.
    TravelNode* getNode(TravelNode* sameNode)
    {
        for (auto& node : nodes)
        {
            if (node->getName() == sameNode->getName()
                && node->getPosition() == sameNode->getPosition())
                return node;
        }

        return nullptr;
    }

    TravelNode* getNode(WorldPosition pos,
                        std::vector<WorldPosition>& ppath,
                        Unit* bot = nullptr, float range = -1);
    TravelNode* getNode(WorldPosition pos, Unit* bot = nullptr,
                        float range = -1)
    {
        std::vector<WorldPosition> ppath;
        return getNode(pos, ppath, bot, range);
    }

    // Get Random Node
    TravelNode* getRandomNode(WorldPosition pos)
    {
        std::vector<TravelNode*> rNodes = getNodes(pos);
        if (rNodes.empty())
            return nullptr;

        return rNodes[urand(0, rNodes.size() - 1)];
    }

    // Finds the best nodePath between two nodes (A* over the node graph)
    TravelNodeRoute GetNodeRoute(TravelNode* start, TravelNode* goal,
                                 Player* bot);

    // Find the nearest start/end nodes for two world positions
    TravelNodeRoute GetNearestNodes(WorldPosition startPos,
                                    WorldPosition endPos,
                                    std::vector<WorldPosition>& startPath,
                                    Player* bot = nullptr);


    // Manage/update nodes
    void manageNodes(Unit* bot, bool mapFull = false);

    void setHasToGen() { hasToGen = true; }

    void generateNpcNodes();
    void generateStartNodes();
    void generateAreaTriggerNodes();
    void generateNodes();
    void generateTransportNodes();
    void generateZoneMeanNodes();

    void generateWalkPaths();
    void removeLowNodes();
    void removeUselessPaths();
    void calculatePathCosts();
    void generateTaxiPaths();
    void generatePaths(bool fullGen = false);

    void generateAll();

    void Init();

    void printMap();

    void printNodeStore();
    void saveNodeStore();
    void LoadNodeStore();

    bool cropUselessNode(TravelNode* startNode);
    TravelNode* addZoneLinkNode(TravelNode* startNode);
    TravelNode* addRandomExtNode(TravelNode* startNode);

    void calcMapOffset();
    WorldPosition getMapOffset(uint32 mapId);

    // Taxi graph (BFS-based path lookup between taxi nodes)
    void InitTaxiGraph();
    std::vector<uint32> FindTaxiPath(uint32 fromNode, uint32 toNode);

    void BuildZoneIndex();
    void PrecomputeReachability();

    TravelNode* GetNearestNodeInZone(WorldPosition pos, uint32 zoneId);
    TravelNode* GetNearestNodeOnMap(WorldPosition pos);

    bool GetFullPath(TravelPlan& plan, WorldPosition botPos,
        uint32 botZoneId, uint32 teamId,
        WorldPosition destination);

    // Resolve A* route between two world positions (returns node vector)
    std::vector<TravelNode*> ResolveRoute(WorldPosition startPos,
        WorldPosition endPos);

    // Get stored walk points for one edge (from→to). Empty if no path.
    std::vector<G3D::Vector3> GetEdgeWalkPoints(TravelNode* from,
        TravelNode* to);

    std::shared_timed_mutex m_nMapMtx;

private:
    TravelNodeMap() = default;
    ~TravelNodeMap() = default;

    TravelNodeMap(const TravelNodeMap&) = delete;
    TravelNodeMap& operator=(const TravelNodeMap&) = delete;

    TravelNodeMap(TravelNodeMap&&) = delete;
    TravelNodeMap& operator=(TravelNodeMap&&) = delete;

    // Taxi graph internals
    void BuildTaxiGraph();
    void ComputeAllPaths();
    std::unordered_map<uint32, uint32> BFS(uint32 startNode);
    std::vector<uint32> BuildPath(
        uint32 fromNode, uint32 toNode,
        const std::unordered_map<uint32, uint32>& parentMap);

    std::unordered_map<uint32, std::vector<uint32>> m_taxiGraph;
    std::map<uint32, std::map<uint32, std::vector<uint32>>>
        m_taxiPathCache;

    std::vector<TravelNode*> nodes;

    std::unordered_map<uint32, std::vector<TravelNode*>> m_zoneIndex;
    std::unordered_map<uint32, std::vector<TravelNode*>> m_mapIndex;

    std::vector<std::pair<uint32, WorldPosition>> mapOffsets;

    bool hasToSave = false;
    bool hasToGen = false;
    bool hasToFullGen = false;
};

#define sTravelNodeMap TravelNodeMap::instance()

#endif
