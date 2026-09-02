/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRAVELNODE_H
#define PLAYERBOTS_TRAVELNODE_H

#include "TravelMgr.h"
#include <shared_mutex>

struct AreaTrigger;

// THEORY
//
//  Pathfinding uses the detour recast navmesh engine for mob, npc, and bot movement.
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
//    areaTrigger(2)   — AreaTrigger teleport (auto-discovered at startup)
//    transport(3)     — Boat/zeppelin (auto-discovered from MO_TRANSPORT)
//    flightPath(4)    — Taxi flight between flight masters
//    teleportSpell(5) — Spell-based teleport (e.g. mage portals)
//    staticPortal(6)  — Manually defined teleport link (DB only, not pruned by generation)
//
//  On server start saved nodes and links are loaded via TravelNodeMap::Init(). Init() precomputes undirected connected components for O(1) reachability checks
//  (see PrecomputeReachability), and builds a taxi BFS graph. Paths and routes are calculated once during generation
//  and saved for future use — runtime path-building was removed (H3) because it mutated the shared graph from bot
//  map threads under only a shared_lock. Nodes are only added at startup or via the console `.generate` command —
//  runtime mutation was removed because taking a unique_lock caused 100-250ms contention spikes against bot threads.
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
//  GetFullPath picks the nearest candidate nodes on each map, runs A* to get a node route, then
//  BuildPath assembles a flat TravelPath with typed waypoints (walk, portal, transport, flight).
//  MoveTo2 reuses the cached TravelPath while the destination is unchanged; UpcomingSpecialMovement cuts
//  to the head segment when special; HandleSpecialMovement dispatches the matching
//  action (portal interact, area-trigger marker, transport board, flight taxi).
//  Cross-map travel is handled naturally by portal/transport edges in the A* graph.
//
//  If setup cannot resolve (no node, no route, no flight), the bot teleports directly to the destination
//  as a fallback.
//
//  The use of hearthstones and mage teleporting was removed — it caused route mutations requiring locking that no longer made sense. Mage portals may be future item.
//
//  Thread Safety:
//
//  The graph is built once at startup on the world thread (LoadNodeStore + generation in Init()) and is never
//  mutated by bot AI afterward: H3 removed the last runtime mutation path, where TravelNodeRoute::BuildPath's
//  canBuildHere branch called TravelNode::BuildPath (a Detour query that also calls setPathTo/setComplete) from
//  bot map threads. A missing/incomplete same-map segment now falls straight through to the existing node-point
//  hop instead (see TravelNodeRoute::BuildPath).
//
//  GetFullPath takes m_nMapMtx with try_lock_shared before reading the graph and returns an empty path when an
//  edit holds the lock, so bots fail a resolve instead of blocking their map thread. saveNodeStore() takes NO
//  lock at all; it only reads the graph to persist it.
//
//  GM-only commands (DebugAction.cpp 'load node' / 'crop path', console 'travel generatenode') remain the only
//  way to edit the graph at runtime; each takes a unique_lock on m_nMapMtx for the whole edit.
//

enum class TravelNodePathType : uint8
{
    none = 0,
    walk = 1,
    areaTrigger = 2,
    transport = 3,
    flightPath = 4,
    // teleportSpell = 5 // maybe someday
    staticPortal = 6
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
    std::vector<WorldPosition> const& GetPath() const { return path; }

    TravelNodePathType getPathType() { return pathType; }
    uint32 getPathObject() { return pathObject; }

    float getDistance() { return distance; }
    float getSwimDistance() { return swimDistance; }
    float getExtraCost() { return extraCost; }
    std::vector<uint8> getMaxLevelCreature() { return maxLevelCreature; }

    void setCalculated(bool calculated1 = true) { calculated = calculated1; }

    bool getCalculated() { return calculated; }

    // Transient marker (NOT persisted to DB): set true when this path is
    // (re)built during the current generation run in BuildPath. Lets the
    // cheating-link sweep touch only freshly-generated links and never the
    // curated links loaded from the DB. Defaults false; DB-loaded paths keep
    // it false since it is never read back from storage.
    void setBuiltDuringRun(bool built = true) { builtDuringRun = built; }
    bool getBuiltDuringRun() { return builtDuringRun; }

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

    // Transient (not persisted): true if built this generation run. See
    // setBuiltDuringRun.
    bool builtDuringRun = false;

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

    // Intrinsic areaTrigger identity. Ported from cmangos getAreaTriggerId().
    void setAreaTriggerId(uint32 id) { areaTriggerId = id; }
    uint32 getAreaTriggerId() { return areaTriggerId; }
    void setAreaTriggerTarget(bool target) { areaTriggerTarget = target; }
    bool isAreaTriggerTarget() { return areaTriggerTarget; }

    // Getters
    std::string const getName() { return nodeName; }
    WorldPosition* getPosition() { return &point; }
    std::unordered_map<TravelNode*, TravelNodePath>* getPaths() { return &paths; }
    std::unordered_map<TravelNode*, TravelNodePath*>* getLinks() { return &links; }
    bool isImportant() { return important; }
    bool isLinked() { return linked; }

    // Does any OTHER node hold a structural (areaTrigger/transport/staticPortal) link INTO this node?
    bool hasStructuralIncoming();

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
            if (link.second->getPathType() == TravelNodePathType::areaTrigger ||
                link.second->getPathType() == TravelNodePathType::staticPortal)
                return true;
        return false;
    }

    // Symmetric structural identity: a node is "structural" if it has an
    // outgoing portal/transport link (isPortal/isTransport), an intrinsic
    // areaTrigger id, is a teleport target, or has any incoming structural
    // link (exit nodes). Used to gate prune protection so portal/transport
    // EXIT nodes are protected too.
    bool isStructural()
    {
        return isPortal() || isTransport() || getAreaTriggerId() ||
               isAreaTriggerTarget() || hasStructuralIncoming();
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

    // Removes links to other nodes that can also be reached by passing another node.
    bool isUselessLink(TravelNode* farNode);
    bool cropUselessLinks();
    bool canCropWalkLinkTo(TravelNode* other);

    // Returns all nodes that can be reached from this node. mapOnly restricts
    // the BFS to links that stay on this node's map (no portals/flights/
    // areaTriggers to other maps).
    std::vector<TravelNode*> getNodeMap(bool importantOnly = false,
        std::vector<TravelNode*> ignoreNodes = {}, bool mapOnly = false);

    // Checks if it is even possible to route to this node.
    bool hasRouteTo(TravelNode* node, bool mapOnly = false)
    {
        if (!node)
            return false;

        if (mapOnly)
            return mapComponentId != 0 && mapComponentId == node->mapComponentId;

        return componentId != 0 && componentId == node->componentId;
    }

    uint32 getRouteSize() { return (uint32)getNodeMap().size(); }

    // Set by TravelNodeMap::PrecomputeReachability() only. 0 = unassigned.
    void setComponentId(uint32 id) { componentId = id; }
    uint32 getComponentId() { return componentId; }
    void setMapComponentId(uint32 id) { mapComponentId = id; }
    uint32 getMapComponentId() { return mapComponentId; }

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

    // Undirected connected-component ids assigned by
    // TravelNodeMap::PrecomputeReachability() (H2). 0 = unassigned. See
    // hasRouteTo for how these are used and why the grouping is safe to
    // over-approximate.
    uint32 componentId = 0;
    uint32 mapComponentId = 0;

    // This node should not be removed
    bool important = false;

    // This node has been checked for nearby links
    bool linked = false;

    // Intrinsic areaTrigger identity (see setAreaTriggerId). 0 = not an areaTrigger entrance node.
    uint32 areaTriggerId = 0;

    // True when this node is the teleport DESTINATION of an areaTrigger (an exit node).
    bool areaTriggerTarget = false;

    // Cache for hasStructuralIncoming(): -1 = uncomputed, 0/1 = result.
    // Structural (areaTrigger/transport/staticPortal) links are created at the
    // start of generation and never removed by the walk-link crop passes, so
    // the incoming-structural status is stable across a generation run and safe
    // to memoize. Not persisted.
    int8 structuralIncomingCache = -1;

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
    NODE_AREA_TRIGGER = 3,
    NODE_TRANSPORT = 4,
    NODE_FLIGHTPATH = 5,
    // value 6 reserved (was NODE_TELEPORT — removed with teleportSpell)
    NODE_STATIC_PORTAL = 7
};

struct PathNodePoint
{
    WorldPosition point;
    PathNodeType type = PathNodeType::NODE_PATH;
    uint32 entry = 0;

    bool operator==(const PathNodePoint& p1) const
    {
        return point == p1.point && type == p1.type && entry == p1.entry;
    }

    bool isWalkable() const { return (uint8)type <= (uint8)PathNodeType::NODE_NODE; }
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
    TravelPath(std::vector<WorldPosition> path, PathNodeType type = PathNodeType::NODE_PATH, uint32 entry = 0)
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
    std::vector<PathNodePoint> const& GetPath() const { return fullPath; }
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

    bool makeShortCut(WorldPosition startPos, float maxDist, Unit* bot = nullptr);

    // For each waypoint that's in/under water, snap its Z to the water surface.
    void surfaceSnapWaypoints(WorldPosition endPos);

    // Trim the path up to (and optionally including) the given point.
    bool cutTo(PathNodePoint point, bool including);

    // Returns true if the next reachable segment is a special-handling
    // node (portal / area-trigger / transport / flightpath / teleport)
    // and the bot is close enough / positioned right to handle it now.
    bool UpcomingSpecialMovement(WorldPosition startPos, float maxDist, bool onTransport);

    void ClipPath(PlayerbotAI* ai, Unit* mover, bool ignoreEnemyTargets = false);

    // Reject paths the navmesh accepts but a player can't walk: >50y 3D
    // segment, 2-point shortcut over 5y, steep terminal stub, or a point in
    // unswimmable water. Consecutive duplicate points are collapsed first.
    static bool IsPathCheating(std::vector<WorldPosition> const& path,
                               float endpointDistance);

    std::ostringstream const print();

private:

    static bool IsPointInAreaTrigger(AreaTrigger const* at, uint32 mapId, float x, float y, float z, float delta);

    std::vector<PathNodePoint>::iterator getNextPoint(WorldPosition startPos, float maxDist, bool onTransport);
    bool shouldMoveToNextPoint(WorldPosition startPos,
                               std::vector<PathNodePoint>::iterator beg,
                               std::vector<PathNodePoint>::iterator ed,
                               std::vector<PathNodePoint>::iterator p,
                               float& moveDist, float maxDist);

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

    std::vector<TravelNode*> const& getNodes() { return nodes; }

    TravelPath BuildPath(std::vector<WorldPosition> pathToStart = {}, std::vector<WorldPosition> pathToEnd = {}, Unit* bot = nullptr);

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
    float totalCost = 0.0f;
    float costFromStart = 0.0f;
    float heuristic = 0.0f;
    bool open = false;
    bool closed = false;
    TravelNodeStub* parent = nullptr;
    uint32 currentGold = 0;
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

    TravelNode* addNode(WorldPosition pos, std::string const preferedName = "Travel Node", bool isImportant = false,
                        bool checkDuplicate = true, bool transport = false, uint32 transportId = 0);
    void removeNode(TravelNode* node);
    void removeNodes()
    {
        while (!_nodes.empty())
            removeNode(_nodes.back());
    }

    // Get all nodes
    std::vector<TravelNode*> const& getNodes() { return _nodes; }
    std::vector<TravelNode*> getNodes(WorldPosition pos, float range = -1);

    // Find nearest node.
    TravelNode* getNode(TravelNode* sameNode)
    {
        for (auto& node : _nodes)
        {
            if (node->getName() == sameNode->getName()
                && node->getPosition() == sameNode->getPosition())
                return node;
        }

        return nullptr;
    }

    TravelNode* getNode(WorldPosition pos, std::vector<WorldPosition>& ppath, Unit* bot = nullptr, float range = -1);
    TravelNode* getNode(WorldPosition pos, Unit* bot = nullptr, float range = -1)
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

    // Default cap on A* node expansions for a runtime routing request.
    static constexpr uint32 SEARCH_BUDGET_DEFAULT = 2000;

    // No cap; generation-time callers must use this so the saved graph
    // never depends on the runtime budget.
    static constexpr uint32 SEARCH_BUDGET_UNLIMITED = 0;

    // Finds the best nodePath between two nodes (A* over the node graph).
    // For a cross-map goal the route may end at the first walkable node
    // past a map crossing instead of at goal (caller detects this as
    // back() != goal); the executor re-plans after each crossing anyway.
    // searchBudget caps expansions; exhaustion returns an empty route,
    // same as no route.
    TravelNodeRoute GetNodeRoute(TravelNode* start, TravelNode* goal, Player* bot,
                                 uint32 searchBudget = SEARCH_BUDGET_DEFAULT, int64 presetGold = -1,
                                 bool* budgetExhausted = nullptr);

    // Picks the nearest start/end nodes for two world positions and runs A*
    // between them. Debug command only; a cross-map route may end at a map
    // seam rather than at the end node.
    TravelNodeRoute FindRouteNearestNodes(WorldPosition startPos, WorldPosition endPos, std::vector<WorldPosition>& startPath,
                                          Player* bot = nullptr);

    void setHasToGen() { hasToGen = true; }

    void generateNpcNodes();
    void generateStartNodes();
    void generateAreaTriggerNodes();
    void generateNodes();
    void generateTransportNodes();
    void generatePortalNodes();
    void makeDockNode(TravelNode* node, WorldPosition exitPos,
                      std::string const dockName);
    void generateZoneMeanNodes();

    void generateWalkPathMap(uint32 mapId);
    void generateWalkPaths();
    void generateHelperNodes(uint32 mapId);
    void generateHelperNodes();
    void removeUselessPaths();
    void removeCheatingPaths();
    void calculatePathCosts();
    void generateTaxiPaths();
    void generatePaths();

    void generateAll();

    void Init();

    void printMap();

    void printNodeStore();
    void saveNodeStore();
    void LoadNodeStore();

    void calcMapOffset();
    WorldPosition getMapOffset(uint32 mapId);

    // Taxi graph (BFS-based path lookup between taxi nodes)
    void InitTaxiGraph();
    std::vector<uint32> FindTaxiPath(uint32 fromNode, uint32 toNode);

    void PrecomputeReachability(bool reportComponents = false);

    TravelPath GetFullPath(WorldPosition botPos,
        WorldPosition destination, Unit* bot = nullptr);

    std::shared_timed_mutex m_nMapMtx;

private:
    TravelNodeMap() = default;
    ~TravelNodeMap() = default;

    TravelNodeMap(TravelNodeMap const&) = delete;
    TravelNodeMap& operator=(TravelNodeMap const&) = delete;

    TravelNodeMap(TravelNodeMap&&) = delete;
    TravelNodeMap& operator=(TravelNodeMap&&) = delete;

    // Logs the componentId partition PrecomputeReachability computed:
    // summary at INFO, per-component detail at DEBUG.
    void logComponentConnectivity();

    static uint32 TravelBudgetFor(Player* bot);
    uint32 pruneCheatingWalkLinks(std::vector<TravelNode*> const& scanNodes);

    // Taxi graph internals
    void BuildTaxiGraph();
    void ComputeAllPaths();
    std::unordered_map<uint32, uint32> BFS(uint32 startNode);
    std::vector<uint32> BuildPath(uint32 fromNode, uint32 toNode,
                                  std::unordered_map<uint32, uint32> const& parentMap);

    std::unordered_map<uint32, std::vector<uint32>> taxiGraph;
    std::map<uint32, std::map<uint32, std::vector<uint32>>> taxiPathCache;

    std::vector<TravelNode*> _nodes;

    std::vector<std::pair<uint32, WorldPosition>> mapOffsets;

    bool hasToSave = false;
    bool hasToGen = false;
    bool hasToFullGen = false;
    bool initialized = false;
};

#define sTravelNodeMap TravelNodeMap::instance()

#endif
