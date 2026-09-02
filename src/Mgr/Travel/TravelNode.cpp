/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TravelNode.h"
#include "BudgetValues.h"
#include "MapMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "RaceMgr.h"
#include "ServerFacade.h"
#include "Transport.h"
#include "TransportMgr.h"

#include <array>
#include <iomanip>
#include <limits>
#include <queue>
#include <regex>
#include <unordered_set>

namespace
{
    constexpr uint32 DISPLAY_ID_PLUNGER = 808;
    constexpr uint32 DISPLAY_ID_SUBWAY = 3831;
    constexpr uint32 DISPLAY_ID_VATOR = 807;
    constexpr uint32 DISPLAY_ID_UNDERVATOR = 455;
    constexpr uint32 DISPLAY_ID_BOAT = 3015;
    constexpr uint32 DISPLAY_ID_ZEPPELIN = 3031;
    constexpr uint32 DISPLAY_ID_MOONSPRAY = 7087;
    constexpr uint32 CHEAT_GOLD_BUDGET = 10000000;
    constexpr float FLIGHT_MASTER_HANDOFF_DISTANCE = 20.0f;
    constexpr float AREA_TRIGGER_EXIT_ADJACENT_DISTANCE = 20.0f;

    constexpr uint32 MIN_STRUCTURAL_WALK_LINKS = 3;
}

// Prints all path properties as a comma-separated string (same order as the constructor arguments).
std::string const TravelNodePath::print()
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);
    out << distance << "f,";
    out << extraCost << "f,";
    out << std::to_string(uint8(pathType)) << ",";
    out << pathObject << ",";
    out << (calculated ? "true" : "false") << ",";
    out << std::to_string(maxLevelCreature[0]) << "," << std::to_string(maxLevelCreature[1]) << ","
        << std::to_string(maxLevelCreature[2]) << ",";
    out << swimDistance << "f";

    return out.str().c_str();
}

// Walks the stored path and gathers what getCost() needs: total distance, swim distance,
// and the highest-level hostile / alliance / horde creatures found near the route.
void TravelNodePath::calculateCost(bool distanceOnly)
{
    std::unordered_map<FactionTemplateEntry const*, bool> aReact, hReact;

    bool aFriend, hFriend;

    if (calculated)
        return;

    distance = 0.1f;
    maxLevelCreature = {0, 0, 0};
    swimDistance = 0;

    // Scan for nearby creatures only every 40y to keep this fast; distance and swim totals still use every waypoint.
    constexpr float CREATURE_SAMPLE_INTERVAL = 40.0f;
    WorldPosition lastScan = WorldPosition();

    WorldPosition lastPoint = WorldPosition();
    for (auto& point : path)
    {
        if (!distanceOnly && (!lastScan || point.distance(lastScan) >= CREATURE_SAMPLE_INTERVAL))
        {
            lastScan = point;
            for (CreatureData const* cData : point.getCreaturesNear(50))  // Aggro radius + 5
            {
                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cData->id);
                if (cInfo)
                {
                    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);

                    if (aReact.find(factionEntry) == aReact.end())
                        aReact.insert(std::make_pair(
                            factionEntry, Unit::GetFactionReactionTo(
                                              factionEntry, sFactionTemplateStore.LookupEntry(1)) > REP_NEUTRAL));
                    aFriend = aReact.find(factionEntry)->second;

                    if (hReact.find(factionEntry) == hReact.end())
                        hReact.insert(std::make_pair(
                            factionEntry, Unit::GetFactionReactionTo(
                                              factionEntry, sFactionTemplateStore.LookupEntry(2)) > REP_NEUTRAL));
                    hFriend = hReact.find(factionEntry)->second;

                    if (maxLevelCreature[0] < cInfo->maxlevel && !aFriend && !hFriend)
                        maxLevelCreature[0] = cInfo->maxlevel;
                    if (maxLevelCreature[1] < cInfo->maxlevel && aFriend && !hFriend)
                        maxLevelCreature[1] = cInfo->maxlevel;
                    if (maxLevelCreature[2] < cInfo->maxlevel && !aFriend && hFriend)
                        maxLevelCreature[2] = cInfo->maxlevel;
                }
            }
        }

        if (lastPoint && point.GetMapId() == lastPoint.GetMapId())
        {
            if (!distanceOnly && (point.isInWater() || lastPoint.isInWater()))
                swimDistance += point.distance(lastPoint);

            distance += point.distance(lastPoint);
        }

        lastPoint = point;
    }

    if (!distanceOnly)
        calculated = true;
}

// Estimated time (in seconds) for this bot to travel the path.
// Returns -1 if the path is unusable (dead bot, unknown or unaffordable taxi).
float TravelNodePath::getCost(Player* bot, uint32 cGold)
{
    float modifier = 1.0f;  // Global modifier
    float timeCost = 0.1f;
    float runDistance = distance - swimDistance;
    float speed = 8.0f;      // default run speed
    float swimSpeed = 4.0f;  // default swim speed.

    if (bot)
    {
        if (getPathType() == TravelNodePathType::flightPath && pathObject)
        {
            if (!bot->IsAlive())
                return -1.0f;

            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

            if (!taxiPath)
                return -1.0f;

            if (!bot->isTaxiCheater() && taxiPath->price > cGold)
                return -1.0f;

            if (!bot->isTaxiCheater() && !bot->m_taxi.IsTaximaskNodeKnown(taxiPath->to))
                return -1.0f;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);
            if (!startTaxiNode || !endTaxiNode ||
                !startTaxiNode->MountCreatureID[bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 0] ||
                !endTaxiNode->MountCreatureID[bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 0])
                return -1.0f;
        }

        speed = bot->GetSpeed(MOVE_RUN);
        swimSpeed = bot->GetSpeed(MOVE_SWIM);

        if (bot->HasSpell(1066))
            swimSpeed *= 1.5;

        uint32 level = bot->GetLevel();
        bool isAlliance = Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(),
                                                     sFactionTemplateStore.LookupEntry(1)) > REP_NEUTRAL;

        int factionAnnoyance = 0;
        if (maxLevelCreature.size() > 0)
        {
            int mobAnnoyance = (maxLevelCreature[0] - level) - 10;  // Mobs 10 levels below do not bother us.

            if (isAlliance)
                factionAnnoyance = (maxLevelCreature[2] - level) - 10;  // Same threshold for opposite-faction guards.
            else if (!isAlliance)
                factionAnnoyance = (maxLevelCreature[1] - level) - 10;

            if (mobAnnoyance > 0)
                modifier += 0.1 * mobAnnoyance;  // For each level the whole path takes 10% longer.
            if (factionAnnoyance > 0)
                modifier += 0.3 * factionAnnoyance;  // For each level the whole path takes 30% longer.
        }
    }
    else if (getPathType() == TravelNodePathType::flightPath)
        return -1.0f;

    if (getPathType() != TravelNodePathType::walk)
        timeCost = extraCost * modifier;
    else
        timeCost = (runDistance / speed + swimDistance / swimSpeed) * modifier;

    return timeCost;
}

uint32 TravelNodePath::getPrice()
{
    if (getPathType() != TravelNodePathType::flightPath)
        return 0;

    if (!pathObject)
        return 0;

    TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

    if (!taxiPath)
        return 0;

    return taxiPath->price;
}

// Builds (or finishes) the walkable path from this node to endNode, trying the
// reverse direction as a fallback. Returns the stored path.
TravelNodePath* TravelNode::BuildPath(TravelNode* endNode, Unit* bot, bool postProcess)
{
    if (GetMapId() != endNode->GetMapId())
        return nullptr;

    TravelNodePath* returnNodePath;

    if (!hasPathTo(endNode))  // Create path if it doesn't exists
        returnNodePath = setPathTo(endNode, TravelNodePath(), false);
    else
        returnNodePath = getPathTo(endNode);  // Get the exsisting path.

    if (returnNodePath->getComplete())  // Path is already complete. Return it.
        return returnNodePath;

    std::vector<WorldPosition> path = returnNodePath->GetPath();

    if (path.empty())
        path = {*getPosition()};  // Start the path from the current Node.

    WorldPosition* endPos = endNode->getPosition();  // Build the path to the end Node.

    path = endPos->getPathFromPath(path, bot);  // Pathfind from the existing path to the end Node.

    bool canPath = endPos->isPathTo(path);  // Check if we reached our destination.

    // Paths toward a portal/transport node often stall just short of it (the node
    // sits off-mesh). If we got within 20y, bridge the last stretch manually and accept.
    if (!canPath && !isTransport() && !isPortal() && !getAreaTriggerId() &&
        !hasStructuralIncoming() &&
        (endNode->getAreaTriggerId() || endNode->isTransport() ||
         endNode->isPortal() || endNode->hasStructuralIncoming()))
    {
        if (endPos->isPathTo(path, 20.0f))
        {
            if (path.back().distance(endPos) > 1.0f)
            {
                float mx = (endPos->GetPositionX() + path.back().GetPositionX()) * 0.5f;
                float my = (endPos->GetPositionY() + path.back().GetPositionY()) * 0.5f;
                float mz = (endPos->GetPositionZ() + path.back().GetPositionZ()) * 0.5f;
                path.emplace_back(endPos->GetMapId(), mx, my, mz);
            }
            path.push_back(*endPos);
            canPath = true;
        }
    }

    // Reject too-short or too-steep results — geometry shortcut that
    // mmap returns but a player can't actually walk.
    if (canPath && TravelPath::IsPathCheating(path, getPosition()->distance(endNode->getPosition())))
    {
        canPath = false;

        if (sPlayerbotAIConfig.hasLog("cheat.csv"))
        {
            std::ostringstream out;
            out << "buildgate," << getName() << "," << endNode->getName() << ","
                << path.size() << ",";
            WorldPosition().printWKT({*getPosition(), *endNode->getPosition()}, out, 1);
            sPlayerbotAIConfig.log("cheat.csv", out.str().c_str());
        }
    }

    // Persist the partial forward attempt before we try the reverse —
    // the recursive endNode->BuildPath below may itself check our state.
    returnNodePath->setPath(path);
    returnNodePath->setComplete(canPath);

    // Make sure the reverse path exists, building it if needed (the recursion
    // stops because BuildPath returns immediately once a path is complete).
    TravelNodePath* backNodePath = nullptr;
    if (!endNode->hasPathTo(this))
        backNodePath = endNode->BuildPath(this, bot, postProcess);
    else
        backNodePath = endNode->getPathTo(this);

    // Forward attempt failed — salvage with the reverse path: use it flipped if
    // it's complete, or stitch the two partials together if their ends meet (<5y).
    if (!canPath && backNodePath)
    {
        std::vector<WorldPosition> backPath = backNodePath->GetPath();
        if (!backPath.empty())
        {
            if (backNodePath->getComplete())
            {
                std::reverse(backPath.begin(), backPath.end());
                path = backPath;
                canPath = true;
            }
            else if (!path.empty() && path.back().distance(&backPath.back()) < 5.0f)
            {
                std::reverse(backPath.begin(), backPath.end());
                path.insert(path.end(), backPath.begin(), backPath.end());

                // Re-check the stitched result: partial paths keep their geometry even
                // when rejected earlier, so the reverse half may contain a jump its
                // own build already refused.
                canPath = !TravelPath::IsPathCheating(
                    path, getPosition()->distance(endNode->getPosition()));

                if (!canPath && sPlayerbotAIConfig.hasLog("cheat.csv"))
                {
                    std::ostringstream out;
                    out << "stitch," << getName() << "," << endNode->getName() << ","
                        << path.size() << ",";
                    WorldPosition().printWKT({*getPosition(), *endNode->getPosition()}, out, 1);
                    sPlayerbotAIConfig.log("cheat.csv", out.str().c_str());
                }
            }
        }
    }

    // A complete reverse path is trusted as-is; only the stitched case above needs re-checking.
    returnNodePath->setComplete(canPath);

    // Mark this path as generated in the current run so the cheating-link
    // sweep considers only fresh links, never curated DB-loaded ones.
    returnNodePath->setBuiltDuringRun(true);

    if (canPath && !hasLinkTo(endNode))
        setLinkTo(endNode, true);

    returnNodePath->setPath(path);

    if (!returnNodePath->getCalculated())
    {
        returnNodePath->calculateCost(!postProcess);
    }

    if (canPath && endNode->hasPathTo(this) && !endNode->hasLinkTo(this))
    {
        TravelNodePath* backNodePath = endNode->getPathTo(this);

        std::vector<WorldPosition> reversePath = path;
        reverse(reversePath.begin(), reversePath.end());
        backNodePath->setPath(reversePath);
        backNodePath->setBuiltDuringRun(true);
        endNode->setLinkTo(this, true);

        if (!backNodePath->getCalculated())
        {
            backNodePath->calculateCost(!postProcess);
        }
    }

    return returnNodePath;
}

// Removes the link to the given node, or (when node is nullptr) every
// reference to this node anywhere in the graph.
void TravelNode::removeLinkTo(TravelNode* node, bool removePaths)
{
    if (node)  // Unlink this specific node
    {
        if (removePaths)
            paths.erase(node);

        links.erase(node);
    }
    else
    {
        // Remove all references to this node.
        for (auto& node : TravelNodeMap::instance().getNodes())
        {
            if (node->hasPathTo(this))
                node->removeLinkTo(this, removePaths);
        }
        links.clear();
        paths.clear();
    }
}

bool TravelNode::hasStructuralIncoming()
{
    if (structuralIncomingCache >= 0)
        return structuralIncomingCache != 0;

    // Scan the graph for any node linking into this one via areaTrigger/transport/portal.
    // This is how portal and transport EXIT nodes are recognized on DB-loaded graphs
    // (protecting them from pruning). Cached: structural links don't change during a run.
    bool found = false;
    for (auto& other : TravelNodeMap::instance().getNodes())
    {
        if (other == this)
            continue;

        if (!other->hasLinkTo(this))
            continue;

        TravelNodePathType const type = other->getPathTo(this)->getPathType();
        if (type == TravelNodePathType::areaTrigger ||
            type == TravelNodePathType::transport ||
            type == TravelNodePathType::staticPortal)
        {
            found = true;
            break;
        }
    }

    structuralIncomingCache = found ? 1 : 0;
    return found;
}

std::vector<TravelNode*> TravelNode::getNodeMap(bool importantOnly, std::vector<TravelNode*> ignoreNodes, bool mapOnly)
{
    std::vector<TravelNode*> openList;
    std::vector<TravelNode*> closeList;

    openList.push_back(this);

    uint32 i = 0;

    while (i < openList.size())
    {
        TravelNode* currentNode = openList[i];

        i++;

        if (!importantOnly || currentNode->isImportant())
            closeList.push_back(currentNode);

        for (auto& nextPath : *currentNode->getLinks())
        {
            TravelNode* nextNode = nextPath.first;

            if (mapOnly && nextNode->GetMapId() != GetMapId())
                continue;

            if (std::find(openList.begin(), openList.end(), nextNode) == openList.end())
            {
                if (ignoreNodes.empty() ||
                    std::find(ignoreNodes.begin(), ignoreNodes.end(), nextNode) == ignoreNodes.end())
                    openList.push_back(nextNode);
            }
        }
    }

    return closeList;
}

bool TravelNode::isUselessLink(TravelNode* farNode)
{
    if (getPathTo(farNode)->getPathType() != TravelNodePathType::walk)
        return false;

    float farLength;
    TravelNodePath* farPath = nullptr;
    if (hasLinkTo(farNode))
    {
        farPath = getPathTo(farNode);
        farLength = farPath->getDistance();

        if (getAreaTriggerId())
        {
            bool farIsTarget = farNode->isAreaTriggerTarget();

            for (auto& link : *getLinks())
            {
                TravelNode* nearNode = link.first;

                if (farNode == nearNode)
                    continue;

                if (farNode->GetMapId() != nearNode->GetMapId())
                    continue;

                if (!farIsTarget && nearNode->isAreaTriggerTarget() &&
                    getDistance(nearNode) < AREA_TRIGGER_EXIT_ADJACENT_DISTANCE)
                    return true;
            }
        }
    }
    else
        farLength = getDistance(farNode);

    for (auto& link : *getLinks())
    {
        TravelNode* nearNode = link.first;
        WorldPosition nearPos = *nearNode->getPosition();
        float nearLength = link.second->getDistance();

        if (farNode == nearNode)
            continue;

        if (farNode->hasLinkTo(this) && !nearNode->hasLinkTo(this))
            continue;

        // AreaTrigger nodes may themselves be removed later; don't count on
        // one as the alternate route.
        if (nearNode->getAreaTriggerId())
            continue;

        if (nearNode->hasLinkTo(farNode))
        {
            // Is it quicker to go past second node to reach first node instead of going directly?
            if (nearLength + nearNode->linkDistanceTo(farNode) < farLength * 1.1)
                return true;

            // If the direct path already passes right next to this neighbour,
            // the direct link is redundant.
            if (farPath && !farPath->GetPath().empty())
                if (nearPos.closestSq(farPath->GetPath()).distance(nearPos) < INTERACTION_DISTANCE)
                    return true;
        }
        else
        {
            // Only consider alternate routes that stay on this map: routing through
            // near-free portal/flight edges would make almost every direct walk link
            // look redundant and crop real overland connectivity.
            if (!nearNode->hasRouteTo(farNode, true))
                continue;

            // Unlimited budget: a budget-exhausted "no route" would read as
            // "no alternate" and under-prune the saved graph.
            TravelNodeRoute route = TravelNodeMap::instance().GetNodeRoute(
                nearNode, farNode, nullptr, TravelNodeMap::SEARCH_BUDGET_UNLIMITED);

            if (route.isEmpty())
                continue;

            if (route.hasNode(this))
                continue;

            bool leavesMap = false;
            for (TravelNode* routeNode : route.getNodes())
            {
                if (routeNode->GetMapId() != GetMapId())
                {
                    leavesMap = true;
                    break;
                }
            }
            if (leavesMap)
                continue;

            // Is it quicker to go past second (and multiple) nodes to reach the first node instead of going directly?
            if (nearLength + route.getTotalDistance() < farLength * 1.1)
                return true;
        }
    }

    return false;
}

bool TravelNode::canCropWalkLinkTo(TravelNode* other)
{
    if (!isStructural())
        return true;

    uint32 walkLinks = 0;
    TravelNode* shortest = nullptr;
    float best = std::numeric_limits<float>::max();
    for (auto& link : *getLinks())
    {
        if (link.second->getPathType() != TravelNodePathType::walk)
            continue;

        ++walkLinks;
        if (link.second->getDistance() < best)
        {
            best = link.second->getDistance();
            shortest = link.first;
        }
    }

    return walkLinks > MIN_STRUCTURAL_WALK_LINKS && shortest != other;
}

bool TravelNode::cropUselessLinks()
{
    bool hasRemoved = false;

    for (auto& firstLink : *getPaths())
    {
        TravelNode* farNode = firstLink.first;

        // Both endpoints must be croppable: each structural endpoint keeps its
        // degree floor and its shortest walk approach.
        if (!canCropWalkLinkTo(farNode) || !farNode->canCropWalkLinkTo(this))
            continue;

        // Decide both directions before removing anything: isUselessLink
        // reads links the other removal would mutate, so removing first
        // could sever both directions of a pair that should keep one.
        bool const forwardUseless = this->hasLinkTo(farNode) && this->isUselessLink(farNode);
        bool const reverseUseless = farNode->hasLinkTo(this) && farNode->isUselessLink(this);

        if (forwardUseless)
        {
            // One direction only; one-way links (ledge drops, teleports)
            // are legitimate.
            this->removeLinkTo(farNode);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                std::ostringstream out;
                out << getName() << ",";
                out << farNode->getName() << ",";
                WorldPosition().printWKT({*getPosition(), *farNode->getPosition()}, out, 1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }

        if (reverseUseless)
        {
            farNode->removeLinkTo(this);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                std::ostringstream out;
                out << farNode->getName() << ",";
                out << getName() << ",";
                WorldPosition().printWKT({*farNode->getPosition(), *getPosition()}, out, 1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }
    }

    return hasRemoved;

}

void TravelNode::print([[maybe_unused]] bool printFailed)
{
    uint32 mapSize = getNodeMap(true).size();

    std::ostringstream out;
    std::string name = getName();
    name.erase(std::remove(name.begin(), name.end(), '\"'), name.end());
    out << name.c_str() << ",";
    out << std::fixed << std::setprecision(2);
    point.printWKT(out);
    out << getZ() << ",";
    out << getO() << ",";
    out << (isImportant() ? 1 : 0) << ",";
    out << mapSize;

    sPlayerbotAIConfig.log("travelNodes.csv", out.str().c_str());

    std::vector<WorldPosition> ppath;

    for (auto& endNode : TravelNodeMap::instance().getNodes())
    {
        if (endNode == this)
            continue;

        if (!hasPathTo(endNode))
            continue;

        TravelNodePath* path = getPathTo(endNode);

        if (!hasLinkTo(endNode) && urand(0, 20) && !printFailed)
            continue;

        ppath = path->GetPath();

        if (ppath.size() < 2 && hasLinkTo(endNode))
        {
            ppath.push_back(point);
            ppath.push_back(*endNode->getPosition());
        }

        if (ppath.size() > 1)
        {
            std::ostringstream out;

            uint32 pathType = static_cast<uint32>(path->getPathType());
            if (!hasLinkTo(endNode))
                pathType = 0;
            else if (!path->getComplete())
                pathType = 0;

            out << pathType << ",";
            out << std::fixed << std::setprecision(2);
            point.printWKT(ppath, out, 1);
            out << path->getPathObject() << ",";
            out << path->getDistance() << ",";
            out << path->getCost() << ",";
            out << (path->getComplete() ? 0 : 1) << ",";
            out << std::to_string(path->getMaxLevelCreature()[0]) << ",";
            out << std::to_string(path->getMaxLevelCreature()[1]) << ",";
            out << std::to_string(path->getMaxLevelCreature()[2]);

            sPlayerbotAIConfig.log("travelPaths.csv", out.str().c_str());
        }
    }
}

// Returns true when a path looks like a navmesh artifact no player could actually
// walk: giant segment jumps, blind straight lines, near-vertical steps, or deep water.
bool TravelPath::IsPathCheating(std::vector<WorldPosition> const& rawPath, float endpointDistance)
{
    if (rawPath.empty())
        return false;

    // Collapse consecutive duplicate points first: a doubled endpoint makes
    // Guard 2 measure a zero-length terminal stub and miss the real one.
    std::vector<WorldPosition> path;
    path.reserve(rawPath.size());
    for (WorldPosition const& p : rawPath)
        if (path.empty() || path.back().GetMapId() != p.GetMapId() ||
            path.back().GetPositionX() != p.GetPositionX() ||
            path.back().GetPositionY() != p.GetPositionY() ||
            path.back().GetPositionZ() != p.GetPositionZ())
            path.push_back(p);

    // Guard 0: walk paths are resampled at 4y steps, so any >50y segment is
    // a fabricated jump, never legitimate string-pulling. Measured in 3D so
    // near-vertical drops count. Same-map segments only (cross-map pairs
    // are areaTrigger/portal seams).
    constexpr float MAX_WALK_SEGMENT = 50.0f;
    for (size_t i = 1; i < path.size(); ++i)
        if (path[i - 1].GetMapId() == path[i].GetMapId() &&
            path[i - 1].distance(path[i]) > MAX_WALK_SEGMENT)
            return true;

    // Guard 1: 2-point path for >5y is navmesh "gave up" — straight
    // line through whatever's between A and B.
    if (path.size() == 2 && endpointDistance > 5.0f)
        return true;

    // Guard 2: steep near-vertical steps. The first and last segments only join the
    // raw node position to the mesh and are often steep for harmless data reasons,
    // so a steep end segment fails the path only when the path is tiny (<=3 points),
    // the step covers a real horizontal run (>5y), or the slope continues into the mesh.
    if (path.size() > 2)
    {
        auto const isSteep = [](WorldPosition const& a, WorldPosition const& b)
        {
            float vDist = std::fabs(a.GetPositionZ() - b.GetPositionZ());
            float hDist = a.GetExactDist2d(b.GetPositionX(), b.GetPositionY());
            return vDist > 10.0f && (hDist == 0.0f || vDist / hDist > 2.0f);
        };

        constexpr float MAX_STUB_RUN = 5.0f;

        if (isSteep(path.front(), path[1]))
        {
            float stubRun = path.front().GetExactDist2d(path[1].GetPositionX(), path[1].GetPositionY());
            if (path.size() <= 3 || stubRun > MAX_STUB_RUN || isSteep(path[1], path[2]))
                return true;
        }

        WorldPosition const& c = path.back();
        WorldPosition const& d = path[path.size() - 2];
        if (isSteep(c, d))
        {
            float stubRun = c.GetExactDist2d(d.GetPositionX(), d.GetPositionY());
            if (path.size() <= 3 || stubRun > MAX_STUB_RUN || isSteep(d, path[path.size() - 3]))
                return true;
        }
    }
    for (WorldPosition point : path)
        if (point.isDarkWater())
            return true;

    return false;
}

bool TravelPath::cutTo(PathNodePoint point, bool including)
{
    auto it = std::find(fullPath.begin(), fullPath.end(), point);

    if (it == fullPath.end())
        return false;

    auto cutIt = including ? std::next(it) : it;
    fullPath.erase(fullPath.begin(), cutIt);
    return true;
}

bool TravelPath::IsPointInAreaTrigger(AreaTrigger const* at, uint32 mapId, float x, float y, float z, float delta)
{
    if (mapId != at->map)
        return false;

    if (at->radius > 0)
    {
        float dx = x - at->x;
        float dy = y - at->y;
        float dz = z - at->z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float r = at->radius + delta;
        return distSq <= r * r;
    }

    // Box: rotate the test point back to AT-local axes, then check
    // axis-aligned half-extents (length=X, width=Y, height=Z).
    double rot = 2.0 * M_PI - at->orientation;
    double sv = std::sin(rot);
    double cv = std::cos(rot);

    float lx = x - at->x;
    float ly = y - at->y;
    float rx = float(at->x + lx * cv - ly * sv) - at->x;
    float ry = float(at->y + ly * cv + lx * sv) - at->y;
    float rz = z - at->z;

    return std::fabs(rx) <= at->length / 2 + delta &&
           std::fabs(ry) <= at->width  / 2 + delta &&
           std::fabs(rz) <= at->height / 2 + delta;
}

bool TravelPath::shouldMoveToNextPoint(WorldPosition startPos,
                                       std::vector<PathNodePoint>::iterator beg,
                                       std::vector<PathNodePoint>::iterator ed,
                                       std::vector<PathNodePoint>::iterator p,
                                       float& moveDist, float maxDist)
{
    if (p == ed)
        return false;

    auto nextP = std::next(p);
    if (nextP == ed)
        return false;

    // Stop at adjacent area-trigger pair sharing entry — second is the
    // teleport-out point we want to land on, not skip past.
    if (p->type == PathNodeType::NODE_AREA_TRIGGER &&
        nextP->type == PathNodeType::NODE_AREA_TRIGGER &&
        p->entry == nextP->entry)
        return false;

    // Same idea for static-portal pair.
    if (p->type == PathNodeType::NODE_STATIC_PORTAL &&
        nextP->type == PathNodeType::NODE_STATIC_PORTAL &&
        p->entry == nextP->entry)
        return false;

    // Approaching a transport boarding node — stop before it.
    if (nextP->type == PathNodeType::NODE_TRANSPORT && nextP->entry)
        return false;

    // Mid-transport: traverse to the disembark side.
    if (p->type == PathNodeType::NODE_TRANSPORT && p->entry)
    {
        // Off-transport detour around a transport segment (rare): skip.
        if (nextP->type != PathNodeType::NODE_TRANSPORT && p != beg &&
            std::prev(p)->type != PathNodeType::NODE_TRANSPORT)
            return true;
        return false;
    }

    // Stop within a flightpath run.
    if (p->type == PathNodeType::NODE_FLIGHTPATH &&
        nextP->type == PathNodeType::NODE_FLIGHTPATH)
        return false;

    float nextMove = p->point.distance(nextP->point);

    if (p->point.GetMapId() != startPos.GetMapId() ||
        ((moveDist + nextMove > maxDist ||
          startPos.distance(nextP->point) > maxDist) && moveDist > 0))
        return false;

    moveDist += nextMove;
    return true;
}

std::vector<PathNodePoint>::iterator
TravelPath::getNextPoint(WorldPosition startPos, float maxDist, bool onTransport)
{
    float minDist = FLT_MAX;
    auto startP = fullPath.begin();

    if (!onTransport)
    {
        // Closest walkable point on the path (same map as the bot).
        for (auto p = fullPath.begin(); p != fullPath.end(); ++p)
        {
            if (p->point.GetMapId() != startPos.GetMapId())
                continue;
            if (!p->isWalkable())
                continue;

            float curDist = p->point.distance(startPos);
            if (curDist <= minDist)
            {
                minDist = curDist;
                startP = p;
            }
        }
    }

    if (startP == fullPath.end())
        return startP;

    float moveDist = startP->point.distance(startPos);

    for (auto p = startP; p != fullPath.end(); ++p)
    {
        if (shouldMoveToNextPoint(startPos, fullPath.begin(), fullPath.end(),
                                  p, moveDist, maxDist))
            continue;

        startP = p;
        break;
    }

    if (startP == fullPath.end() || !startP->isWalkable())
        return startP;

    auto nextP = std::next(startP);
    if (nextP == fullPath.end())
        return startP;

    // If startPos is between startP and nextP, skip ahead to nextP.
    float project = startPos.projectOnSegment(startP->point, nextP->point);
    if (project > 0.0f && project < 1.0f)
        return nextP;

    return startP;
}

bool TravelPath::UpcomingSpecialMovement(WorldPosition startPos,
                                          float maxDist, bool onTransport)
{
    if (fullPath.empty())
        return false;

    auto startP = getNextPoint(startPos, maxDist, onTransport);
    if (startP == fullPath.end())
        return false;

    auto prevP = startP, nextP = startP;
    if (startP != fullPath.begin())
        prevP = std::prev(prevP);
    if (std::next(nextP) != fullPath.end())
        nextP = std::next(nextP);

    // Area trigger: zone-gated. With entry, must be inside the trigger
    // zone; without entry, fire as soon as we reach it.
    if (startP->type == PathNodeType::NODE_AREA_TRIGGER)
    {
        if (startP->entry)
        {
            // sObjectMgr->GetAreaTrigger is AC's loaded view of the trigger
            // data, so it's the only existence check needed.
            AreaTrigger const* at = sObjectMgr->GetAreaTrigger(startP->entry);
            if (!at)
                return false;

            if (!IsPointInAreaTrigger(at, startPos.GetMapId(),
                                      startPos.GetPositionX(),
                                      startPos.GetPositionY(),
                                      startPos.GetPositionZ(), 0.5f))
                return false;
        }

        cutTo(*startP, false);
        return true;
    }

    // Static portal (game-object spellcaster): interact when in range.
    if (startP->type == PathNodeType::NODE_STATIC_PORTAL &&
        startPos.distance(startP->point) < INTERACTION_DISTANCE)
    {
        cutTo(*startP, false);
        return true;
    }

    // Flight path: hand over to the taxi handler when near the node. The 20y range
    // (wider than interaction distance) tolerates node points stored on unreachable
    // spots; the handler validates proximity itself.
    if (startP->type == PathNodeType::NODE_FLIGHTPATH)
    {
        float const fmDist = startPos.distance(startP->point);
        if (fmDist < FLIGHT_MASTER_HANDOFF_DISTANCE)
        {
            cutTo(*startP, false);
            return true;
        }
    }

    // Transports are always ridden for real: cut to dock if off-transport,
    // traverse to disembark if on-transport.
    if (startP->type == PathNodeType::NODE_TRANSPORT)
    {
        uint32 const entry = nextP->entry;

        if (!onTransport)
        {
            // prevP = dock, startP = where transport will stop.
            cutTo(*prevP, false);
            return true;
        }

        // On transport: walk to disembark.
        for (auto p = startP; p != fullPath.end(); ++p)
        {
            if (p->type != PathNodeType::NODE_TRANSPORT ||
                (p->entry && p->entry != entry))
            {
                cutTo(*p, false);
                return true;
            }
        }
    }

    return false;
}

void TravelPath::ClipPath(PlayerbotAI* ai, Unit* mover, bool ignoreEnemyTargets)
{
    if (fullPath.empty())
        return;

    auto startP = getNextPoint(WorldPosition(mover), 0.0f, false);
    if (startP == fullPath.end())
        return;

    // cutTo re-finds the point by value and erases everything before it,
    // which invalidates startP — don't touch the iterator afterwards
    // (cutTo with including=false always keeps the found point, so the
    // path stays non-empty).
    cutTo(*startP, false);

    std::vector<std::pair<WorldPosition, float>> threats;
    Player* bot = ai ? ai->GetBot() : nullptr;
    if (bot && ai->GetState() != BOT_STATE_COMBAT && !bot->isDead() && !ignoreEnemyTargets)
    {
        AiObjectContext* context = ai->GetAiObjectContext();
        for (ObjectGuid const& targetGuid : AI_VALUE(GuidVector, "possible targets"))
        {
            if (!targetGuid.IsCreature())
                continue;
            Unit* unit = ai->GetUnit(targetGuid);
            if (!unit || unit->isDead())
                continue;
            if (unit->GetLevel() > mover->GetLevel() + 5)
                continue;
            Creature* cre = unit->ToCreature();
            if (!cre)
                continue;
            if (!cre->CanCreatureAttack(mover, true) || !unit->IsWithinLOSInMap(mover))
                continue;
            float const range = cre->GetAttackDistance(mover);
            threats.emplace_back(WorldPosition(unit), range * range);
        }
    }

    auto endP = fullPath.end();
    auto prevP = fullPath.begin();
    float const reactSq = sPlayerbotAIConfig.reactDistance * sPlayerbotAIConfig.reactDistance;

    for (auto p = fullPath.begin(); p != fullPath.end(); ++p)
    {
        for (auto& threat : threats)
        {
            if (threat.first.sqDistance(p->point) > threat.second)
                continue;

            endP = prevP;
            break;
        }
        if (endP != fullPath.end())
            break;

        // Reject paths that drift past reactDistance from the start —
        // a sign the path looped or wandered.
        if (p->point.sqDistance(fullPath.begin()->point) > reactSq)
            endP = p;
        // Non-walkable hop in the middle (portal/transport/etc.) terminates.
        else if (!p->isWalkable())
            endP = p;
        // Gap between adjacent points > ~11y (sqDist 125) — likely bad data.
        else if (p->point.sqDistance(prevP->point) > 125.0f)
            endP = prevP;

        if (endP != fullPath.end())
            break;

        prevP = p;
    }

    if (endP == fullPath.end())
        return;

    fullPath.erase(std::next(endP), fullPath.end());
}

void TravelPath::surfaceSnapWaypoints(WorldPosition endPos)
{
    if (fullPath.empty())
        return;
    // Same map + dest is on land. If dest is itself underwater the bot
    // wants to dive; leave waypoints alone.
    if (fullPath.front().point.GetMapId() != endPos.GetMapId() ||
        endPos.isUnderWater())
        return;
    for (auto& p : fullPath)
    {
        if (p.point.isUnderWater())
            p.point.setAtWaterSurface();
    }
}

bool TravelPath::makeShortCut(WorldPosition startPos, float maxDist, Unit* bot)
{
    if (fullPath.empty())
        return false;

    float maxDistSq = maxDist * maxDist;
    float minDist = -1;
    float totalDist = fullPath.begin()->point.sqDistance(startPos);
    std::vector<PathNodePoint> newPath;
    WorldPosition firstNode;

    for (auto& p : fullPath)  // cycle over the full path
    {
        // Walkability filter: portals/transports/taxis aren't valid
        // anchor points — picking one as the new start of the trimmed
        // path would leave the bot anchored on a hop.
        if (p.point.GetMapId() == startPos.GetMapId() && p.isWalkable())
        {
            float curDist = p.point.sqDistance(startPos);

            if (&p != &fullPath.front())
                totalDist += p.point.sqDistance(std::prev(&p)->point);

            if (curDist <
                sPlayerbotAIConfig.tooCloseDistance *
                    sPlayerbotAIConfig.tooCloseDistance)  // We are on the path. This is a good starting point
            {
                minDist = curDist;
                totalDist = curDist;
                newPath.clear();
            }

            if (p.type != PathNodeType::NODE_PREPATH)  // Only look at the part after the first node and in the same map.
            {
                if (!firstNode)
                    firstNode = p.point;

                if (minDist == -1 || curDist < minDist ||
                    (curDist < maxDistSq && curDist < totalDist / 2))  // Start building from the last closest point or
                                                                       // a point that is close but far on the path.
                {
                    minDist = curDist;
                    totalDist = curDist;
                    newPath.clear();
                }
            }
        }

        newPath.push_back(p);
    }

    if (newPath.empty() || minDist > maxDistSq || newPath.front().point.GetMapId() != startPos.GetMapId())
    {
        clear();
        return false;
    }

    WorldPosition beginPos = newPath.begin()->point;

    // The old path seems to be the best — either the closest walkable
    // point IS the original front, or it's within tooCloseDistance.
    if (newPath.front() == fullPath.front() ||
        beginPos.distance(firstNode) < sPlayerbotAIConfig.tooCloseDistance)
        return false;

    // We are (nearly) on the new path. Just follow the rest.
    if (beginPos.distance(startPos) < sPlayerbotAIConfig.tooCloseDistance)
    {
        fullPath = newPath;
        return true;
    }

    // Pass the bot into getPathTo so PathGenerator picks up its
    // collision/swim/fly state. nullptr defaults to a generic mover
    // which can produce paths the bot can't actually walk.
    std::vector<WorldPosition> toPath = startPos.getPathTo(beginPos, bot);

    // We can not reach the new begin position. Follow the complete path.
    if (!beginPos.isPathTo(toPath))
        return false;

    // Move to the new path and continue.
    fullPath.clear();
    addPath(toPath);
    addPath(newPath);

    return true;
}

std::ostringstream const TravelPath::print()
{
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00,"
        << "1,";
    out << std::fixed;

    WorldPosition().printWKT(getPointPath(), out, 1);

    return out;
}

float TravelNodeRoute::getTotalDistance()
{
    if (nodes.size() < 2)
        return 0;

    float totalLength = 0;
    for (uint32 i = 0; i < nodes.size() - 1; i++)
        totalLength += nodes[i]->linkDistanceTo(nodes[i + 1]);

    return totalLength;
}

TravelPath TravelNodeRoute::BuildPath(std::vector<WorldPosition> pathToStart, std::vector<WorldPosition> pathToEnd,
                                      [[maybe_unused]] Unit* bot)
{
    TravelPath travelPath;

    if (!pathToStart.empty())  // From start position to start of path.
        travelPath.addPath(pathToStart, PathNodeType::NODE_PREPATH);

    TravelNode* prevNode = nullptr;
    for (auto& node : nodes)
    {
        if (prevNode)
        {
            TravelNodePath* nodePath = nullptr;
            if (prevNode->hasPathTo(node))  // Get the path to the next node if it exists.
                nodePath = prevNode->getPathTo(node);


            if (!nodePath || !nodePath->getComplete())  // If we can not build a path just try to move to the node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_NODE);
                prevNode = node;
                continue;
            }

            if (nodePath->getPathType() == TravelNodePathType::areaTrigger)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject());
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject());
            }
            else if (nodePath->getPathType() == TravelNodePathType::staticPortal)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject());
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject());
            }
            else if (nodePath->getPathType() == TravelNodePathType::transport)
            {
                // Emit the transport's full waypoint route, not just board+exit.
                // Intermediate points carry NODE_TRANSPORT type so the executor
                // sees consecutive transport waypoints as one block (board at
                // first, disembark at last).
                travelPath.addPath(nodePath->GetPath(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject());
            }
            else if (nodePath->getPathType() == TravelNodePathType::flightPath)
            {
                // Full taxi waypoint route; same reasoning as transport.
                travelPath.addPath(nodePath->GetPath(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject());
            }
            else
            {
                std::vector<WorldPosition> path = nodePath->GetPath();

                if (path.size() > 1 &&
                    node != nodes.back())  // Remove the last point since that will also be the start of the next path.
                    path.pop_back();

                if (path.size() > 1 && prevNode->isPortal() &&
                    nodePath->getPathType() != TravelNodePathType::areaTrigger &&
                    nodePath->getPathType() != TravelNodePathType::staticPortal)
                    path.erase(path.begin());

                if (path.size() > 1 && prevNode->isTransport() &&
                    nodePath->getPathType() != TravelNodePathType::transport)
                    path.erase(path.begin());

                travelPath.addPath(path, PathNodeType::NODE_PATH);
            }
        }
        prevNode = node;
    }

    if (!pathToEnd.empty())
        travelPath.addPath(pathToEnd, PathNodeType::NODE_PATH);

    return travelPath;
}

std::ostringstream const TravelNodeRoute::print()
{
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00"
        << ",0,"
        << "\"LINESTRING(";

    for (auto& node : nodes)
    {
        out << std::fixed << node->getPosition()->getDisplayX() << " " << node->getPosition()->getDisplayY() << ",";
    }

    out << ")\"";

    return out;
}

TravelNode* TravelNodeMap::addNode(WorldPosition pos, std::string const preferedName, bool isImportant,
                                   bool checkDuplicate, [[maybe_unused]] bool transport,
                                   [[maybe_unused]] uint32 transportId)
{
    TravelNode* newNode;

    if (checkDuplicate)
    {
        newNode = getNode(pos, nullptr, 5.0f);
        if (newNode)
            return newNode;
    }

    std::string finalName = preferedName;

    if (!isImportant)
    {
        std::regex last_num("[[:digit:]]+$");
        finalName = std::regex_replace(finalName, last_num, "");
        uint32 nameCount = 1;

        for (auto& node : getNodes())
        {
            if (node->getName().find(preferedName + std::to_string(nameCount)) != std::string::npos)
                nameCount++;
        }

        if (nameCount)
            finalName += std::to_string(nameCount);
    }

    newNode = new TravelNode(pos, finalName, isImportant);

    _nodes.push_back(newNode);

    return newNode;
}

void TravelNodeMap::removeNode(TravelNode* node)
{
    node->removeLinkTo(nullptr, true);

    for (auto& tnode : _nodes)
    {
        if (tnode == node)
        {
            delete tnode;
            tnode = nullptr;
        }
    }

    _nodes.erase(std::remove(_nodes.begin(), _nodes.end(), nullptr), _nodes.end());
}

std::vector<TravelNode*> TravelNodeMap::getNodes(WorldPosition pos, float range)
{
    std::vector<TravelNode*> retVec;

    for (auto& node : _nodes)
    {
        if (node->GetMapId() == pos.GetMapId())
            if (range == -1 || node->getDistance(pos) <= range)
                retVec.push_back(node);
    }

    std::sort(retVec.begin(), retVec.end(),
              [pos](TravelNode* i, TravelNode* j)
              { return i->getPosition()->distance(pos) < j->getPosition()->distance(pos); });

    return retVec;
}

TravelNode* TravelNodeMap::getNode(WorldPosition pos, [[maybe_unused]] std::vector<WorldPosition>& ppath, Unit* bot,
                                   float range)
{
    if (bot && !bot->GetMap())
        return nullptr;

    uint32 c = 0;

    std::vector<TravelNode*> nodes = TravelNodeMap::instance().getNodes(pos, range);
    for (auto& node : _nodes)
    {
        if (!bot || pos.canPathTo(*node->getPosition(), bot))
            return node;

        c++;

        if (c > 5)  // Max 5 attempts
            break;
    }

    return nullptr;
}

uint32 TravelNodeMap::TravelBudgetFor(Player* bot)
{
    if (!bot)
        return 0;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return bot->GetMoney();

    if (botAI->HasCheat(BotCheatMask::gold))
        return CHEAT_GOLD_BUDGET;

    AiObjectContext* context = botAI->GetAiObjectContext();
    uint32 budget = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::travel);
    bool const isLeader = botAI->GetGroupLeader() == bot;
    for (ObjectGuid guid : AI_VALUE(GuidVector, "group members"))
    {
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            continue;
        if (!isLeader && player != bot)
            continue;
        if (!botAI->IsSafe(player))
        {
            budget = 0;
            continue;
        }
        if (!GET_PLAYERBOT_AI(player))
            continue;
        budget = std::min(budget, PAI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::travel));
    }

    return budget;
}

TravelNodeRoute TravelNodeMap::GetNodeRoute(TravelNode* start, TravelNode* goal,
    Player* bot, uint32 searchBudget, int64 presetGold, bool* budgetExhausted)
{
    float botSpeed = bot ? bot->GetSpeed(MOVE_RUN) : 7.0f;

    if (start == goal)
        return TravelNodeRoute();

    // Arms the cross-map seam exit in the main loop below.
    bool const crossMap = start->GetMapId() != goal->GetMapId();

    // Basic A* algorithm
    std::unordered_map<TravelNode*, TravelNodeStub> m_stubs;

    TravelNodeStub* startStub = &m_stubs.insert(std::make_pair(start, TravelNodeStub(start))).first->second;

    TravelNodeStub* currentNode = nullptr;
    TravelNodeStub* childNode = nullptr;
    float f = 0.f;
    float g = 0.f;
    float h = 0.f;

    std::vector<std::pair<float, TravelNodeStub*>> open;

    startStub->currentGold = presetGold >= 0 ? (uint32)presetGold : TravelBudgetFor(bot);

    if (!start->hasRouteTo(goal))
        return TravelNodeRoute();

    auto heapComp = [](std::pair<float, TravelNodeStub*> const& i, std::pair<float, TravelNodeStub*> const& j)
    { return i.first > j.first; };

    uint32 expansions = 0;

    open.emplace_back(startStub->totalCost, startStub);
    startStub->open = true;

    while (!open.empty())
    {
        std::pop_heap(open.begin(), open.end(), heapComp);
        float const poppedCost = open.back().first;
        currentNode = open.back().second;
        open.pop_back();

        if (currentNode->closed || poppedCost > currentNode->totalCost)
            continue;

        currentNode->open = false;
        currentNode->closed = true;
        expansions++;

        // For a cross-map goal, stop at the first walkable node on another map:
        // the route is re-planned after every map crossing anyway. Callers detect
        // the truncation as back() != goal.
        if (currentNode->dataNode == goal ||
            (crossMap && currentNode->dataNode->GetMapId() != start->GetMapId() &&
             currentNode->dataNode->isWalking()))
        {
            TravelNodeStub* parent = currentNode->parent;

            std::vector<TravelNode*> path;
            path.push_back(currentNode->dataNode);

            while (parent != nullptr)
            {
                path.push_back(parent->dataNode);
                parent = parent->parent;
            }

            reverse(path.begin(), path.end());
            return TravelNodeRoute(path);
        }

        // Budget exhausted: return empty, same as no route. Callers already
        // handle empty; a partial route would never be walked.
        if (searchBudget > 0 && expansions >= searchBudget)
        {
            LOG_DEBUG("playerbots",
                      "[TravelFail] A* search budget ({}) exhausted routing {} -> {}, treating as no route",
                      searchBudget, start->getName(), goal->getName());
            if (budgetExhausted)
                *budgetExhausted = true;
            return TravelNodeRoute();
        }

        for (auto const& link : *currentNode->dataNode->getLinks())  // for each successor n' of n
        {
            TravelNode* linkNode = link.first;
            float linkCost = link.second->getCost(bot, currentNode->currentGold);

            if (linkCost <= 0)
                continue;

            childNode = &m_stubs.insert(std::make_pair(linkNode, TravelNodeStub(linkNode))).first->second;
            g = currentNode->costFromStart + linkCost;  // stance from start + distance between the two nodes
            if ((childNode->open || childNode->closed) &&
                childNode->costFromStart <= g)  // n' is already in opend or closed with a lower cost g(n')
                continue;             // consider next successor

            h = childNode->dataNode->fDist(goal) / botSpeed;
            f = g + h; // compute f(n')
            childNode->totalCost = f;
            childNode->costFromStart = g;
            childNode->heuristic = h;
            childNode->parent = currentNode;

            if (bot && !bot->isTaxiCheater())
                childNode->currentGold = currentNode->currentGold - link.second->getPrice();

            childNode->closed = false;
            childNode->open = true;
            open.emplace_back(f, childNode);
            std::push_heap(open.begin(), open.end(), heapComp);
        }
    }

    return TravelNodeRoute();
}

TravelNodeRoute TravelNodeMap::FindRouteNearestNodes(WorldPosition startPos, WorldPosition endPos,
                                            std::vector<WorldPosition>& startPath, Player* bot)
{
    if (_nodes.empty() || !bot)
        return TravelNodeRoute();

    constexpr uint32 K = 3;
    if (_nodes.size() < K)
        return TravelNodeRoute();

    // Single copy of the node list, find closest K for start and end
    std::vector<TravelNode*> nodesCopy = this->_nodes;

    // nth_element is O(n) — partitions so the first K are the closest (unordered)
    std::nth_element(nodesCopy.begin(), nodesCopy.begin() + K, nodesCopy.end(),
                     [startPos](TravelNode* i, TravelNode* j) { return i->fDist(startPos) < j->fDist(startPos); });
    // Sort just the K closest
    std::sort(nodesCopy.begin(), nodesCopy.begin() + K,
              [startPos](TravelNode* i, TravelNode* j) { return i->fDist(startPos) < j->fDist(startPos); });

    // Save the K closest start nodes before reusing the vector for end nodes
    std::array<TravelNode*, K> startNodes;
    std::copy_n(nodesCopy.begin(), K, startNodes.begin());

    std::nth_element(nodesCopy.begin(), nodesCopy.begin() + K, nodesCopy.end(),
                     [endPos](TravelNode* i, TravelNode* j) { return i->fDist(endPos) < j->fDist(endPos); });
    std::sort(nodesCopy.begin(), nodesCopy.begin() + K,
              [endPos](TravelNode* i, TravelNode* j) { return i->fDist(endPos) < j->fDist(endPos); });

    std::array<TravelNode*, K> endNodes;
    std::copy_n(nodesCopy.begin(), K, endNodes.begin());

    // Cycle over the combinations of these K nodes.
    uint32 startI = 0, endI = 0;
    while (startI < K && endI < K)
    {
        TravelNode* startNode = startNodes[startI];
        TravelNode* endNode = endNodes[endI];

        WorldPosition startNodePosition = *startNode->getPosition();

        TravelNodeRoute route = GetNodeRoute(startNode, endNode, bot);

        if (!route.isEmpty())
        {
            // Check if the bot can actually walk to this start node using mmap pathfinding.
            if (startNodePosition.GetMapId() == bot->GetMapId())
            {
                PathGenerator path(bot);
                path.SetNavTerrainCost(NAV_GROUND_STEEP, 5.0f);
                path.SetNavTerrainCost(NAV_WATER, 10.0f);
                path.CalculatePath(startNodePosition.GetPositionX(), startNodePosition.GetPositionY(), startNodePosition.GetPositionZ());
                PathType type = path.GetPathType();
                bool reachable = !(type & ~(PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY));

                if (reachable)
                {
                    startPath = {startPos, startNodePosition};
                    return route;
                }
            }
            startI++;
        }

        // Prefer a different end-node.
        endI++;

        // Cycle to a different start-node if needed.
        if (endI > startI + 1)
        {
            startI++;
            endI = 0;
        }
    }

    return TravelNodeRoute();
}

TravelPath TravelNodeMap::GetFullPath(WorldPosition botPos,
    WorldPosition destination, Unit* bot)
{
    TravelPath path;

    // Tiered routing: short same-map trips (<= travelNodeDirectDistance, default
    // 300y) try a direct mmap path first; longer trips route through the node graph,
    // with the direct probe kept only as a no-route fallback at the bottom. A failed
    // short probe leaves its waypoints in beginPath for reuse below.
    std::vector<WorldPosition> beginPath;
    if (sPlayerbotAIConfig.travelNodeProbeSteps > 0 &&
        botPos.GetMapId() == destination.GetMapId() &&
        botPos.distance(destination) <= sPlayerbotAIConfig.travelNodeDirectDistance)
    {
        beginPath = destination.getPathFromPath({botPos}, bot,
                                                sPlayerbotAIConfig.travelNodeProbeSteps);
        if (destination.isPathTo(beginPath, sPlayerbotAIConfig.spellDistance))
            return TravelPath(beginPath);
    }

    std::shared_lock<std::shared_timed_mutex> guard(m_nMapMtx, std::try_to_lock);
    if (!guard.owns_lock())
        return path;

    Player* const player = bot ? bot->ToPlayer() : nullptr;

    // If the bot is mid-transport, the first valid route wins immediately
    // (no ground validation — the transport handles position).
    uint32 transportEntry = 0;
    if (bot && bot->GetTransport())
        transportEntry = bot->GetTransport()->GetEntry();

    // Pick the K nearest candidate nodes (3D distance) for both ends. Scan the
    // whole map: filtering by zone would miss nodes just across a zone boundary.
    constexpr uint32 K = 5;
    auto pickKNearest = [&](WorldPosition pos) -> std::vector<TravelNode*>
    {
        std::vector<TravelNode*> candidates;
        for (TravelNode* n : _nodes)
            if (n && n->getPosition()->GetMapId() == pos.GetMapId())
                candidates.push_back(n);
        if (candidates.empty())
            return {};
        uint32 const n = std::min<uint32>(K, (uint32)candidates.size());
        std::partial_sort(candidates.begin(), candidates.begin() + n, candidates.end(),
                          [pos](TravelNode* i, TravelNode* j)
                          { return i->getPosition()->sqDistance(pos) < j->getPosition()->sqDistance(pos); });
        candidates.resize(n);
        return candidates;
    };

    std::vector<TravelNode*> startCandidates = pickKNearest(botPos);
    std::vector<TravelNode*> endCandidates = pickKNearest(destination);

    if (startCandidates.empty() || endCandidates.empty())
    {
        LOG_DEBUG("playerbots",
                  "[TravelFail] no node candidates: startMap {} ({}) endMap {} ({})",
                  botPos.GetMapId(), startCandidates.size(),
                  destination.GetMapId(), endCandidates.size());
        return path;  // empty
    }

    // Try candidate pairs, validating each end with real mmap paths and
    // remembering failures so a bad node is only tested once.
    std::vector<TravelNode*> badStartNodes, badEndNodes;
    int64 const travelGold = (int64)TravelBudgetFor(player);

    for (TravelNode* e : endCandidates)
    {
        if (std::find(badEndNodes.begin(), badEndNodes.end(), e) != badEndNodes.end())
            continue;
        if (!e)
            continue;
        WorldPosition endNodePos = *e->getPosition();

        // Validate that the endNode can reach the destination. Use a generous band
        // (spellDistance horizontally, 25y vertically): destinations are NPCs that
        // often stand on raised or off-mesh spots, and the bot covers the last
        // stretch locally after arriving.
        constexpr float endMaxZ = 25.0f;
        std::vector<WorldPosition> endProbe;
        bool endPathOk = false;
        // Only probe the final leg when the bot is already on the destination map:
        // pathfinding another map's navmesh from this thread races that map's own
        // update thread and can freeze the world thread. For cross-map routes the
        // final leg is stubbed and resolved locally once the bot arrives.
        if (endNodePos.GetMapId() == destination.GetMapId() &&
            bot && bot->GetMapId() == destination.GetMapId())
        {
            endProbe = endNodePos.getPathTo(destination, bot);
            endPathOk = destination.isPathTo(endProbe, sPlayerbotAIConfig.spellDistance, endMaxZ);
        }
        else
        {
            // Deferred / cross-map: the endNode is the approach target; the exact
            // final leg is computed locally on arrival.
            endProbe = {endNodePos, destination};
            endPathOk = true;
        }

        if (!endPathOk)
        {
            badEndNodes.push_back(e);
            continue;
        }

        for (TravelNode* s : startCandidates)
        {
            if (std::find(badStartNodes.begin(), badStartNodes.end(), s) != badStartNodes.end())
                continue;
            if (!s || s == e)
                continue;
            if (!s->hasRouteTo(e))
                continue;

            WorldPosition startNodePos = *s->getPosition();

            // A* on the graph.
            bool budgetExhausted = false;
            TravelNodeRoute route = GetNodeRoute(s, e, player, SEARCH_BUDGET_DEFAULT, travelGold, &budgetExhausted);
            if (route.isEmpty())
            {
                if (budgetExhausted)
                    badStartNodes.push_back(s);
                continue;
            }

            // If A* stopped at a map seam (route doesn't end at e), don't append the
            // probe computed for e — use the bare destination instead; a fresh plan
            // is made after the map crossing anyway.
            std::vector<WorldPosition> const finalLeg =
                route.getNodes().back() == e ? endProbe : std::vector<WorldPosition>{destination};

            // On a transport: skip ground validation, accept the route.
            if (transportEntry)
            {
                path = route.BuildPath({botPos}, finalLeg, bot);
                return path;
            }

            // Validate the bot can walk to the start node, reusing the earlier
            // probe waypoints when possible instead of re-running mmap.
            float const maxStartDistance = s->isTransport() ? 20.0f : INTERACTION_DISTANCE;
            std::vector<WorldPosition> pathToStart = beginPath;
            bool startPathOk = !pathToStart.empty() &&
                startNodePos.cropPathTo(pathToStart, maxStartDistance);

            if (!startPathOk && bot && botPos.GetMapId() == startNodePos.GetMapId())
            {
                pathToStart = botPos.getPathTo(startNodePos, bot);
                // 25y vertical tolerance: multi-level areas put the node a few
                // yards above or below the bot's mesh level.
                startPathOk = startNodePos.isPathTo(pathToStart, maxStartDistance, 25.0f);
            }

            if (!startPathOk)
            {
                badStartNodes.push_back(s);
                continue;
            }

            // Both ends validated — build and return, keeping pathToStart for reuse.
            beginPath = pathToStart;
            path = route.BuildPath(pathToStart, finalLeg, bot);
            return path;
        }
    }

    // Fallback when the graph gave no usable route: try a direct chained mmap probe
    // on the bot's own map (step budget: AiPlayerbot.TravelNodeProbeSteps, 0 = off).
    if (sPlayerbotAIConfig.travelNodeProbeSteps > 0 &&
        botPos.GetMapId() == destination.GetMapId())
    {
        std::vector<WorldPosition> direct =
            destination.getPathFromPath({botPos}, bot, sPlayerbotAIConfig.travelNodeProbeSteps);
        if (destination.isPathTo(direct, sPlayerbotAIConfig.spellDistance))
            return TravelPath(direct);

        // A partial probe still helps: the path is re-planned as the bot advances,
        // so returning the partial leg chains probes across a long journey instead
        // of leaving the bot stuck with a useless one-point path.
        float const remaining = direct.empty() ? -1.0f : destination.distance(direct.back());
        if (direct.size() > 1 &&
            remaining + 10.0f < destination.distance(botPos))
        {
            LOG_DEBUG("playerbots",
                      "[TravelFail] no graph route map {} — partial probe: {} pts, {:.0f}y remaining of {:.0f}y",
                      botPos.GetMapId(), direct.size(), remaining, destination.distance(botPos));
            return TravelPath(direct);
        }

        LOG_DEBUG("playerbots",
                  "[TravelFail] no graph route map {} and probe made no progress ({} pts, {:.0f}y to dest)",
                  botPos.GetMapId(), direct.size(), destination.distance(botPos));
    }
    else if (botPos.GetMapId() == destination.GetMapId())
        LOG_DEBUG("playerbots",
                  "[TravelFail] no graph route map {} (probe fallback disabled), {:.0f}y to dest",
                  botPos.GetMapId(), destination.distance(botPos));

    return path;  // empty
}

void TravelNodeMap::generateNpcNodes()
{
    std::unordered_map<uint32, std::pair<CreatureTemplate const*, WorldPosition>> bossMap;

    for (auto& creatureData : WorldPosition().getCreaturesNear())
    {
        WorldPosition guidP(creatureData->mapid, creatureData->posX, creatureData->posY, creatureData->posZ,
                            creatureData->orientation);

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creatureData->id);
        if (!cInfo)
            continue;

        uint32 flagMask = UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_FLIGHTMASTER | UNIT_NPC_FLAG_SPIRITHEALER |
                          UNIT_NPC_FLAG_SPIRITGUIDE;

        if (cInfo->npcflag & flagMask)
        {
            std::string nodeName = guidP.getAreaName(false);

            if (cInfo->npcflag & UNIT_NPC_FLAG_INNKEEPER)
                nodeName += " innkeeper";
            else if (cInfo->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER)
                nodeName += " flightMaster";
            else if (cInfo->npcflag & UNIT_NPC_FLAG_SPIRITHEALER)
                nodeName += " spirithealer";
            else if (cInfo->npcflag & UNIT_NPC_FLAG_SPIRITGUIDE)
                nodeName += " spiritguide";

            TravelNodeMap::instance().addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->rank == 3)
        {
            std::string const nodeName = cInfo->Name;

            TravelNodeMap::instance().addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->rank == 1 && !guidP.isOverworld())
        {
            if (bossMap.find(cInfo->Entry) == bossMap.end())
                bossMap[cInfo->Entry] = std::make_pair(cInfo, guidP);
            else if (bossMap[cInfo->Entry].second)
                bossMap[cInfo->Entry] = std::make_pair(nullptr, GuidPosition());
        }
    }

    for (auto boss : bossMap)
    {
        WorldPosition guidP = boss.second.second;
        if (!guidP)
            continue;

        CreatureTemplate const* cInfo = boss.second.first;
        if (!cInfo)
            continue;

        std::string const nodeName = cInfo->Name;

        TravelNodeMap::instance().addNode(guidP, nodeName, true, true);
    }
}

void TravelNodeMap::generateStartNodes()
{
    std::map<uint8, std::string> startNames;
    startNames[RACE_HUMAN] = "Human";
    startNames[RACE_ORC] = "Orc and Troll";
    startNames[RACE_DWARF] = "Dwarf and Gnome";
    startNames[RACE_NIGHTELF] = "Night Elf";
    startNames[RACE_UNDEAD_PLAYER] = "Undead";
    startNames[RACE_TAUREN] = "Tauren";
    startNames[RACE_GNOME] = "Dwarf and Gnome";
    startNames[RACE_TROLL] = "Orc and Troll";

    for (uint32 i = 0; i < sRaceMgr->GetMaxRaces(); i++)
    {
        for (uint32 j = 0; j < MAX_CLASSES; j++)
        {
            PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);

            if (!info)
                continue;

            WorldPosition pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            std::string const nodeName = startNames[i] + " start";

            TravelNodeMap::instance().addNode(pos, nodeName, true, true);

            break;
        }
    }
}

void TravelNodeMap::generateAreaTriggerNodes()
{
    // Entrance nodes
    for (auto const& itr : sObjectMgr->GetAllAreaTriggerTeleports())
    {
        AreaTriggerTeleport const& atEntry = itr.second;
        AreaTrigger const* at = sObjectMgr->GetAreaTrigger(itr.first);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(at->map, at->x, at->y, at->z, at->orientation);
        WorldPosition outPos = WorldPosition(atEntry.target_mapId, atEntry.target_X, atEntry.target_Y, atEntry.target_Z,
                                             atEntry.target_Orientation);

        std::string nodeName;
        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        TravelNodeMap::instance().addNode(inPos, nodeName, true, true);
    }

    // Exit nodes + area-trigger link
    for (auto const& itr : sObjectMgr->GetAllAreaTriggerTeleports())
    {
        AreaTriggerTeleport const& atEntry = itr.second;
        AreaTrigger const* at = sObjectMgr->GetAreaTrigger(itr.first);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(at->map, at->x, at->y, at->z, at->orientation);
        WorldPosition outPos = WorldPosition(atEntry.target_mapId, atEntry.target_X, atEntry.target_Y, atEntry.target_Z,
                                             atEntry.target_Orientation);

        std::string nodeName;
        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        TravelNode* outNode = TravelNodeMap::instance().addNode(outPos, nodeName, true, true);
        TravelNode* inNode = TravelNodeMap::instance().getNode(inPos, nullptr, 5.0f);

        if (outNode && inNode)
        {
            TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::areaTrigger, itr.first, true);
            travelPath.setPath({*inNode->getPosition(), *outNode->getPosition()});
            inNode->setPathTo(outNode, travelPath);

            // Tag structural identity: the entrance carries the trigger id, the
            // destination is flagged as a teleport target (flags aren't saved to DB).
            inNode->setAreaTriggerId(itr.first);
            outNode->setAreaTriggerTarget(true);
        }
    }
}

void TravelNodeMap::makeDockNode(TravelNode* node, WorldPosition exitPos, std::string const dockName)
{
    // A hand-placed node named "<ship node name><dockName>" within 75y overrides
    // the computed dock position: adopt it and wire the boarding links to it.
    std::string const curatedName = node->getName() + dockName;
    for (TravelNode* n : _nodes)
    {
        if (!n || n == node || n->getName() != curatedName)
            continue;
        if (n->getPosition()->GetMapId() != node->getPosition()->GetMapId() ||
            n->getPosition()->distance(*node->getPosition()) > 75.0f)
            continue;

        WorldPosition dockPos = *n->getPosition();
        WorldPosition nodePos = *node->getPosition();
        TravelNodePath travelPath(dockPos.distance(nodePos), 0.1f, (uint8)TravelNodePathType::transport, 0, true);
        travelPath.setComplete(true);
        travelPath.setPath({dockPos, nodePos});
        n->setPathTo(node, travelPath, true);
        travelPath.setPath({nodePos, dockPos});
        node->setPathTo(n, travelPath, true);
        node->setLinked(true);
        return;
    }

    // Settle the exit position onto the sampled ground height, falling back to
    // the raw offset position if the ground can't be sampled.
    if (Map* map = exitPos.getMap())
    {
        // Boot-time generation runs before these grids are loaded; without
        // terrain the height sample fails and the water check below lies.
        map->EnsureGridCreated(Acore::ComputeGridCoord(exitPos.GetPositionX(), exitPos.GetPositionY()));

        float const gh = map->GetHeight(exitPos.GetPositionX(), exitPos.GetPositionY(),
                                        exitPos.GetPositionZ() + 5.0f, true, 50.0f);
        if (gh > INVALID_HEIGHT)
            exitPos.setZ(gh);

        // Ship stops sit mid-harbor, so the settled point may still be in the water.
        // Search outward in rings for the nearest dry ground and put the dock there;
        // if nothing dry is found within 40y, keep the settled position.
        if (exitPos.isInWater())
        {
            bool found = false;
            for (float radius = 5.0f; radius <= 40.0f && !found; radius += 5.0f)
            {
                for (uint32 step = 0; step < 12 && !found; ++step)
                {
                    float const angle = step * 2.0f * float(M_PI) / 12.0f;
                    float const cx = exitPos.GetPositionX() + radius * std::cos(angle);
                    float const cy = exitPos.GetPositionY() + radius * std::sin(angle);
                    float const cz = map->GetHeight(cx, cy, exitPos.GetPositionZ() + 10.0f, true, 50.0f);
                    if (cz <= INVALID_HEIGHT)
                        continue;

                    WorldPosition candidate(exitPos.GetMapId(), cx, cy, cz);
                    if (candidate.isInWater())
                        continue;

                    exitPos = candidate;
                    found = true;
                }
            }
        }
    }

    // Only add paths if we are adding a new node (reuse an existing dock).
    TravelNode* exitNode = TravelNodeMap::instance().getNode(exitPos, nullptr, 1.0f);
    if (exitNode)
        return;

    exitNode = TravelNodeMap::instance().addNode(exitPos, node->getName() + dockName, true, false);
    if (!exitNode)
        return;

    // Pre-made complete transport link both ways (the path is part of the
    // transport, so it is not a walk link and is exempt from every prune).
    WorldPosition nodePos = *node->getPosition();

    TravelNodePath travelPath(exitPos.distance(nodePos), 0.1f, (uint8)TravelNodePathType::transport, 0, true);
    travelPath.setComplete(true);
    travelPath.setPath({exitPos, nodePos});
    exitNode->setPathTo(node, travelPath, true);

    travelPath.setPath({nodePos, exitPos});
    node->setPathTo(exitNode, travelPath, true);

    // Keeps the transport node out of the walk pass as an unlinked start.
    node->setLinked(true);
}

void TravelNodeMap::generateTransportNodes()
{
    for (auto const& itr : *sObjectMgr->GetGameObjectTemplates())
    {
        uint32 const entry = itr.first;
        GameObjectTemplate const* data = &itr.second;
        if (!data || (data->type != GAMEOBJECT_TYPE_TRANSPORT && data->type != GAMEOBJECT_TYPE_MO_TRANSPORT))
            continue;

        if (data->displayId == DISPLAY_ID_PLUNGER)
            continue;

        uint32 pathId = data->moTransport.taxiPathId;
        float moveSpeed = data->moTransport.moveSpeed;
        if (pathId >= sTaxiPathNodesByPath.size())
            continue;

        TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

        std::vector<WorldPosition> ppath;
        TravelNode* prevNode = nullptr;

        if (path.empty())
        {
            // Elevators / trams have no taxi path: positions come from the transport
            // animation keyframes, rotated and offset to each spawned GO's location.
            TransportAnimation const* animation = sTransportMgr->GetTransportAnimInfo(entry);
            if (!animation)
                continue;

            TransportPathContainer const& aPath = animation->Path;

            for (GameObjectData const* goData : WorldPosition().getGameObjectsNear(0, entry))
            {
                prevNode = nullptr;
                WorldPosition basePos(goData->mapid, goData->posX, goData->posY, goData->posZ, goData->orientation);
                WorldPosition lPos = WorldPosition();
                uint32 timeStart = 0;

                for (auto const& kf : aPath)
                {
                    TransportAnimationEntry const* p = kf.second;

                    float const o = basePos.GetOrientation();
                    float const dx = std::cos(o) * p->X - std::sin(o) * p->Y;
                    float const dy = std::sin(o) * p->X + std::cos(o) * p->Y;

                    WorldPosition pos = WorldPosition(basePos.GetMapId(), basePos.GetPositionX() + dx,
                                                      basePos.GetPositionY() + dy, basePos.GetPositionZ() + p->Z, o);

                    if (prevNode)
                        ppath.push_back(pos);

                    // A keyframe at the same position as the previous one is a
                    // stop (the elevator pauses there).
                    if (pos.distance(lPos) == 0)
                    {
                        TravelNode* node =
                            TravelNodeMap::instance().addNode(pos, data->name, true, true, true, entry);

                        WorldPosition exitPos = pos;

                        if (data->displayId == DISPLAY_ID_SUBWAY)
                            exitPos.setZ(exitPos.GetPositionZ() - 10.0f);
                        if (data->displayId == DISPLAY_ID_VATOR)
                            exitPos.setZ(exitPos.GetPositionZ() - 1.25f);
                        if (data->displayId == DISPLAY_ID_UNDERVATOR)
                            exitPos.setZ(exitPos.GetPositionZ() - 0.46f);

                        makeDockNode(node, exitPos, "entry");

                        if (!prevNode)
                        {
                            ppath.push_back(pos);
                            timeStart = p->TimeSeg;
                        }
                        else
                        {
                            float const totalTime = (p->TimeSeg - timeStart) / 1000.0f;

                            TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::transport, entry,
                                                      true);
                            travelPath.setPath(ppath);
                            prevNode->setPathTo(node, travelPath);
                            ppath.clear();
                            ppath.push_back(pos);
                            timeStart = p->TimeSeg;
                        }

                        prevNode = node;
                    }

                    lPos = pos;
                }

                ppath.clear();
            }
        }
        else  // Boats / zeppelins (taxi-path transports)
        {
            // Loop over the path and connect stop locations.
            for (auto& p : path)
            {
                WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                if (prevNode)
                    ppath.push_back(pos);

                if (p->delay > 0)
                {
                    TravelNode* node = TravelNodeMap::instance().addNode(pos, data->name, true, true, true, entry);

                    WorldPosition exitPos = pos;

                    if (data->displayId == DISPLAY_ID_BOAT)
                        exitPos.setZ(exitPos.GetPositionZ() + 6.0f);
                    else if (data->displayId == DISPLAY_ID_ZEPPELIN)
                        exitPos.setZ(exitPos.GetPositionZ() - 17.0f);
                    else if (data->displayId == DISPLAY_ID_MOONSPRAY)
                        exitPos.setZ(exitPos.GetPositionZ() + 4.88f);

                    makeDockNode(node, exitPos, "dock");

                    if (!prevNode)
                    {
                        ppath.push_back(pos);
                    }
                    else
                    {
                        TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                        travelPath.setPathAndCost(ppath, moveSpeed);
                        prevNode->setPathTo(node, travelPath);
                        ppath.clear();
                        ppath.push_back(pos);
                    }

                    prevNode = node;
                }
            }

            if (prevNode)
            {
                // Continue from start until first stop and connect to end.
                for (auto& p : path)
                {
                    WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);
                    ppath.push_back(pos);

                    if (p->delay > 0)
                    {
                        TravelNode* node = TravelNodeMap::instance().getNode(pos, nullptr, 5.0f);

                        if (node && node != prevNode)
                        {
                            TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                            travelPath.setPathAndCost(ppath, moveSpeed);
                            prevNode->setPathTo(node, travelPath);
                        }
                    }
                }
            }
        }
        ppath.clear();
    }
}

void TravelNodeMap::generatePortalNodes()
{
    // Static portals are spellcaster GameObjects whose spell teleports the user.
    // Each becomes a one-way staticPortal link from the GO spawn to the destination.
    for (GameObjectData const* goData : WorldPosition().getGameObjectsNear(0, 0))
    {
        GameObjectTemplate const* data = sObjectMgr->GetGameObjectTemplate(goData->id);
        if (!data || data->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(data->spellcaster.spellId);
        if (!spellInfo)
            continue;

        // Follow one level of triggered spell.
        if (spellInfo->Effects[EFFECT_0].TriggerSpell)
            if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(spellInfo->Effects[EFFECT_0].TriggerSpell))
                spellInfo = triggered;

        uint8 teleIndex = MAX_SPELL_EFFECTS;
        for (uint8 i = EFFECT_0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_TELEPORT_UNITS)
            {
                teleIndex = i;
                break;
            }
        }

        if (teleIndex == MAX_SPELL_EFFECTS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr->GetSpellTargetPosition(spellInfo->Id, SpellEffIndex(teleIndex));
        if (!pos)
            continue;

        WorldPosition inPos(goData->mapid, goData->posX, goData->posY, goData->posZ, goData->orientation);
        WorldPosition outPos(pos->target_mapId, pos->target_X, pos->target_Y, pos->target_Z,
                             pos->target_Orientation);

        TravelNode* inNode = TravelNodeMap::instance().addNode(inPos, data->name, true, true);
        TravelNode* outNode = TravelNodeMap::instance().addNode(outPos, data->name, true, true);

        if (!inNode || !outNode || inNode == outNode)
            continue;

        TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::staticPortal, goData->id, true);
        travelPath.setPath({*inNode->getPosition(), *outNode->getPosition()});
        inNode->setPathTo(outNode, travelPath);
    }
}

void TravelNodeMap::generateZoneMeanNodes()
{
    // Zone means
    for (auto& loc : TravelMgr::instance().exploreLocs)
    {
        std::vector<WorldPosition*> points;

        for (auto p : loc.second->getPoints(true))
            if (!p->isUnderWater())
                points.push_back(p);

        if (points.empty())
            points = loc.second->getPoints(true);

        WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

        TravelNodeMap::instance().addNode(pos, pos.getAreaName(), true, true, false);
    }
}

void TravelNodeMap::generateNodes()
{
    LOG_INFO("playerbots", "-Generating Start nodes");
    generateStartNodes();
    LOG_INFO("playerbots", "-Generating npc nodes");
    generateNpcNodes();
    LOG_INFO("playerbots", "-Generating area trigger nodes");
    generateAreaTriggerNodes();
    LOG_INFO("playerbots", "-Generating transport nodes");
    generateTransportNodes();
    LOG_INFO("playerbots", "-Generating zone mean nodes");
    generateZoneMeanNodes();
    LOG_INFO("playerbots", "-Generating portal nodes");
    generatePortalNodes();
}

void TravelNodeMap::generateWalkPathMap(uint32 mapId)
{
    std::vector<TravelNode*> mapNodes = TravelNodeMap::instance().getNodes(WorldPosition(mapId, 1, 1));

    // Progress totals count only unlinked nodes — the actual work this call does
    // (repeat calls from the helper pass usually add just one new node).
    uint32 total = 0;
    for (auto& n : mapNodes)
        if (n && !n->isLinked())
            ++total;

    uint32 processed = 0;

    for (auto& startNode : mapNodes)
    {
        if (startNode->isLinked())
            continue;

        ++processed;
        WorldPosition* p = startNode->getPosition();
        if (processed % 50 == 0 || processed == total)
            LOG_INFO("playerbots", "-- walkpaths: map {} {}/{}", mapId, processed, total);

        // Logged before the inner loop so a hang inside BuildPath is pinned
        // to this exact node.
        LOG_DEBUG("playerbots", "-- walkpaths: map {} {}/{} start | node ({:.0f},{:.0f},{:.0f})",
                 mapId, processed, total,
                 p->GetPositionX(), p->GetPositionY(), p->GetPositionZ());

        for (auto& endNode : TravelNodeMap::instance().getNodes(*startNode->getPosition(), 2000.0f))
        {
            if (endNode->isTransport() && endNode->isLinked())
                continue;

            if (startNode == endNode)
                continue;

            if (startNode->hasCompletePathTo(endNode))
                continue;

            if (startNode->GetMapId() != endNode->GetMapId())
                continue;

            startNode->BuildPath(endNode, nullptr, false);
        }

        startNode->setLinked(true);

        LOG_DEBUG("playerbots", "-- walkpaths: map {} {}/{} done | got {} links",
                 mapId, processed, total, (uint32)startNode->getLinks()->size());
    }
}

void TravelNodeMap::generateWalkPaths()
{
    std::map<uint32, bool> nodeMaps;

    for (auto& startNode : TravelNodeMap::instance().getNodes())
        nodeMaps[startNode->GetMapId()] = true;

    LOG_INFO("playerbots", "-- walkpaths: linking across {} map(s)", (uint32)nodeMaps.size());

    for (auto& map : nodeMaps)
        generateWalkPathMap(map.first);

    LOG_INFO("playerbots", ">> Generated paths for {} nodes.",
             TravelNodeMap::instance().getNodes().size());
}

void TravelNodeMap::generateHelperNodes(uint32 mapId)
{
    std::vector<TravelNode*> startNodes = getNodes(WorldPosition(mapId, 1, 1));

    std::vector<std::pair<WorldPosition, std::string>> placesToReach;

    // Find all places we might want to reach.
    for (auto& node : startNodes)
    {
        if (node->isTransport())
            continue;

        if (node->isPortal())
            continue;

        if (node->getRouteSize() > 1000)
            continue;

        placesToReach.push_back(std::make_pair(*node->getPosition(), node->getName()));
    }

    if (placesToReach.empty() || startNodes.empty())
        return;

    for (auto& pos : placesToReach)
    {
        std::vector<TravelNode*> startNodes = getNodes(WorldPosition(mapId, 1, 1));
        // Find closest 5 nodes.
        std::partial_sort(startNodes.begin(), startNodes.begin() + std::min(int(startNodes.size()), 5),
                          startNodes.end(),
                          [pos](TravelNode* i, TravelNode* j) { return i->fDist(pos.first) < j->fDist(pos.first); });

        bool found = false;

        for (uint8 i = 0; i < std::min(int(startNodes.size()), 5); i++)
        {
            TravelNode* node = startNodes[i];

            if (node->isTransport())
                continue;

            if (node->isPortal())
                continue;

            if (node->getPosition()->canPathTo(pos.first, nullptr))
                continue;

            TravelNode* otherNode = getNode(pos.first, nullptr, 1.0f);

            if (otherNode && node->hasLinkTo(otherNode))
                continue;

            for (auto& path : *node->getPaths())
            {
                WorldPosition prevPoint;
                for (WorldPosition ppoint : path.second.GetPath())
                {
                    if (prevPoint && ppoint.sqDistance2d(prevPoint) < 100.0f)
                        continue;

                    prevPoint = ppoint;

                    if (!ppoint.canPathTo(pos.first, nullptr))
                        continue;

                    std::string name = node->getName() + " to " + pos.second;
                    addNode(ppoint, name, false, true);
                    found = true;

                    break;
                }

                if (found)
                    break;
            }

            if (found)
            {
                generateWalkPathMap(mapId);
                break;
            }
        }

        if (!found)
        {
            std::string name = pos.second;
            addNode(pos.first, name, false, true);
        }
    }

    for (auto& node : startNodes)
    {
        if (!node->isTransport())
            node->setLinked(false);
    }

    generateWalkPathMap(mapId);
}

void TravelNodeMap::generateHelperNodes()
{
    std::map<uint32, bool> nodeMaps;

    uint32 old = (uint32)TravelNodeMap::instance().getNodes().size();

    for (auto& startNode : TravelNodeMap::instance().getNodes())
        nodeMaps[startNode->GetMapId()] = true;

    uint32 placesToReach = 0;

    for (auto& map : nodeMaps)
    {
        std::vector<TravelNode*> startNodes = getNodes(WorldPosition(map.first, 1, 1));
        // Find all places we might want to reach.
        for (auto& node : startNodes)
        {
            if (node->isTransport())
                continue;

            if (node->isPortal())
                continue;

            if (node->getRouteSize() > 1000)
                continue;

            placesToReach++;
        }
    }

    LOG_INFO("playerbots",
             "-Finding new nodes to reach {} nodes that can currently not be properly reached.",
             placesToReach);

    for (auto& map : nodeMaps)
        generateHelperNodes(map.first);

    LOG_INFO("playerbots", ">> Generated {} helper nodes.",
             (uint32)(TravelNodeMap::instance().getNodes().size() - old));
}

void TravelNodeMap::generateTaxiPaths()
{
    uint32 const totalPaths = sTaxiPathStore.GetNumRows();
    uint32 processed = 0;
    uint32 created = 0;
    LOG_INFO("playerbots", "-- taxi: processing {} taxi paths", totalPaths);

    for (uint32 i = 0; i < totalPaths; ++i)
    {
        // Progress heartbeat: this loop covers thousands of taxi paths.
        if (++processed % 500 == 0)
            LOG_INFO("playerbots", "-- taxi: {}/{} processed | {} flight links",
                     processed, totalPaths, created);

        TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

        if (!taxiPath)
            continue;

        TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);

        if (!startTaxiNode)
            continue;

        TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);

        if (!endTaxiNode)
            continue;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];

        if (nodes.empty())
            continue;

        WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
        WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

        TravelNode* startNode = TravelNodeMap::instance().getNode(startPos, nullptr, 15.0f);
        TravelNode* endNode = TravelNodeMap::instance().getNode(endPos, nullptr, 15.0f);

        if (!startNode || !endNode)
            continue;

        std::vector<WorldPosition> ppath;

        for (auto& n : nodes)
            ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

        float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

        TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::flightPath, i, true);
        travelPath.setPath(ppath);

        // Don't overwrite an existing valid walk link (the position lookup can
        // resolve to a non-flight-master node such as an innkeeper). Check hasLinkTo,
        // not hasPathTo: failed walk attempts also sit in the paths map and would
        // wrongly suppress real flight links.
        if (startNode->hasLinkTo(endNode) &&
            startNode->getPathTo(endNode)->getPathType() == TravelNodePathType::walk)
            continue;

        startNode->setPathTo(endNode, travelPath);
        ++created;
    }

    LOG_INFO("playerbots", ">> Generated {} flight links from {} taxi paths.", created, totalPaths);
}

void TravelNodeMap::removeUselessPaths()
{
    PrecomputeReachability();

    uint32 it = 0;
    while (true)
    {
        uint32 rem = 0;
        // Clean up node links
        for (auto& startNode : TravelNodeMap::instance().getNodes())
        {
            if (startNode->cropUselessLinks())
                rem++;
        }

        if (!rem)
            break;

        PrecomputeReachability();

        hasToSave = true;
        it++;

        LOG_INFO("playerbots", "Iteration {}, removed {}", it, rem);
    }
}

uint32 TravelNodeMap::pruneCheatingWalkLinks(std::vector<TravelNode*> const& scanNodes)
{
    uint32 removed = 0;
    uint32 scanned = 0;

    for (auto& startNode : scanNodes)
    {
        if (!startNode)
            continue;

        // Collect first; removeLinkTo mutates the link map we're iterating.
        std::vector<TravelNode*> cheats;
        for (auto& link : *startNode->getLinks())
        {
            if (link.second->getPathType() != TravelNodePathType::walk)
                continue;

            // Only touch links built during this run: DB-loaded links are curated
            // data, and deleting one is permanent (nothing would recreate it).
            if (!link.second->getBuiltDuringRun())
                continue;

            // Never prune the walk approach to a structure node: the steep step
            // at an elevator/zeppelin/portal is the structure itself, not a cheat.
            if (startNode->isStructural() || link.first->isStructural())
                continue;

            ++scanned;

            float const endDist =
                startNode->getPosition()->distance(link.first->getPosition());
            if (TravelPath::IsPathCheating(link.second->GetPath(), endDist))
                cheats.push_back(link.first);
        }

        for (auto* endNode : cheats)
        {
            WorldPosition* sp = startNode->getPosition();
            WorldPosition* ep = endNode->getPosition();
            // Per-link detail at debug; removeCheatingPaths logs the
            // pruned total, and cheat.csv carries the forensic record.
            LOG_DEBUG("playerbots",
                     "-- prunecheat: dropping walk link '{}' -> '{}' "
                     "(map {} ({:.0f},{:.0f},{:.0f}) -> ({:.0f},{:.0f},{:.0f}))",
                     startNode->getName(), endNode->getName(), startNode->GetMapId(),
                     sp->GetPositionX(), sp->GetPositionY(), sp->GetPositionZ(),
                     ep->GetPositionX(), ep->GetPositionY(), ep->GetPositionZ());

            if (sPlayerbotAIConfig.hasLog("cheat.csv"))
            {
                std::ostringstream out;
                out << "sweep," << startNode->getName() << "," << endNode->getName()
                    << "," << startNode->getPathTo(endNode)->GetPath().size() << ",";
                WorldPosition().printWKT({*sp, *ep}, out, 1);
                sPlayerbotAIConfig.log("cheat.csv", out.str().c_str());
            }

            // removePaths=true so the stored geometry is gone too, not just
            // the link flag (a lingering path would re-link on a later pass).
            startNode->removeLinkTo(endNode, true);
            endNode->removeLinkTo(startNode, true);
            ++removed;
        }
    }

    return removed;
}

void TravelNodeMap::removeCheatingPaths()
{
    uint32 const removed = pruneCheatingWalkLinks(_nodes);

    if (removed)
        hasToSave = true;

    LOG_INFO("playerbots", ">> Pruned {} cheating walk links from the graph.", removed);
}

void TravelNodeMap::calculatePathCosts()
{
    uint32 const total = (uint32)TravelNodeMap::instance().getNodes().size();
    uint32 processed = 0;
    uint32 costed = 0;

    for (auto& startNode : TravelNodeMap::instance().getNodes())
    {
        ++processed;
        WorldPosition* sp = startNode->getPosition();
        if (processed % 50 == 0 || processed == total)
            LOG_INFO("playerbots", "-- pathcosts: {}/{}", processed, total);

        // Logged before costing this node's links so a hang inside
        // calculateCost is pinned to this exact node.
        LOG_DEBUG("playerbots", "-- pathcosts: {}/{} start | map {} node ({:.0f},{:.0f},{:.0f}) {} links",
                 processed, total, startNode->GetMapId(),
                 sp->GetPositionX(), sp->GetPositionY(), sp->GetPositionZ(),
                 (uint32)startNode->getLinks()->size());

        for (auto& path : *startNode->getLinks())
        {
            TravelNodePath* nodePath = path.second;

            if (path.second->getPathType() != TravelNodePathType::walk)
                continue;

            if (nodePath->getCalculated())
                continue;

            nodePath->calculateCost();
            ++costed;
        }
    }

    LOG_INFO("playerbots", ">> Calculated pathcost for {} nodes ({} links costed).", total, costed);
}

void TravelNodeMap::generatePaths()
{
    // Re-create the areaTrigger, transport and portal links every run: they are only
    // born in generateNodes(), so a link reset would otherwise leave instance
    // entrances, portals and transports orphaned. Must run before generateWalkPaths,
    // which relies on isTransport()/isPortal() (defined by these very links).
    LOG_INFO("playerbots", "-Generating area trigger nodes");
    generateAreaTriggerNodes();
    LOG_INFO("playerbots", "-Generating transport nodes");
    generateTransportNodes();
    LOG_INFO("playerbots", "-Generating portal nodes");
    generatePortalNodes();

    LOG_INFO("playerbots", "-Calculating walkable paths");
    generateWalkPaths();

    LOG_INFO("playerbots", "-Generating helper nodes");
    generateHelperNodes();

    // The cheat sweep must run before removeUselessPaths: cropping keeps a link
    // only when an alternate route exists, and that route must not run through
    // links the sweep is about to delete.
    LOG_INFO("playerbots", "-Removing cheating paths");
    removeCheatingPaths();

    // Drop links already covered by another route; without this the graph
    // balloons to tens of millions of redundant path points.
    LOG_INFO("playerbots", "-Removing useless paths");
    removeUselessPaths();

    LOG_INFO("playerbots", "-Calculating path costs");
    calculatePathCosts();
    LOG_INFO("playerbots", "-Generating taxi paths");
    generateTaxiPaths();

    TravelMgr::instance().ReleasePathingCreatures();
}

void TravelNodeMap::generateAll()
{
    if (_nodes.empty())
        generateNodes();

    generatePaths();
    hasToSave = true;
    saveNodeStore();

    PrecomputeReachability(true);
}

void TravelNodeMap::Init()
{
    if (initialized)
        return;
    initialized = true;

    InitTaxiGraph();

    if (!sPlayerbotAIConfig.enableTravelNodes)
        return;

    LoadNodeStore();
    calcMapOffset();

    // Incremental boot-time generation self-loads the grids/mmap tiles it needs
    // (TravelMgr::GetPathingCreature); it only makes boot slower.
    if (hasToFullGen)
    {
        hasToFullGen = false;
        hasToGen = false;
        LOG_ERROR("playerbots",
                  "Travel node store is empty or unreadable; boot-time generation skipped. "
                  "Import the node SQL or run '.playerbots travel generatenode' from the console.");
    }
    else if (hasToGen)
    {
        generatePaths();
        hasToGen = false;
        hasToSave = true;
        saveNodeStore();
    }

    PrecomputeReachability(true);
}

void TravelNodeMap::printMap()
{
    if (!sPlayerbotAIConfig.hasLog("travelNodes.csv") && !sPlayerbotAIConfig.hasLog("travelPaths.csv"))
        return;

    printf("\r [Qgis] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog("travelNodes.csv", "w");
    sPlayerbotAIConfig.openLog("travelPaths.csv", "w");

    std::vector<TravelNode*> anodes = getNodes();

    for (auto& node : anodes)
    {
        node->print(false);
    }
}

void TravelNodeMap::printNodeStore()
{
    std::string const nodeStore = "TravelNodeStore.h";

    if (!sPlayerbotAIConfig.hasLog(nodeStore))
        return;

    printf("\r [Map] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog(nodeStore, "w");

    std::unordered_map<TravelNode*, uint32> saveNodes;

    std::vector<TravelNode*> anodes = getNodes();

    sPlayerbotAIConfig.log(nodeStore, "#pragma once");
    sPlayerbotAIConfig.log(nodeStore, "#include \"TravelMgr.h\"");
    sPlayerbotAIConfig.log(nodeStore, "class TravelNodeStore");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "    public:");
    sPlayerbotAIConfig.log(nodeStore, "    static void loadNodes()");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "        TravelNode** nodes = new TravelNode*[%zu];", anodes.size());

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        std::ostringstream out;

        std::string name = node->getName();
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());

        //        struct addNode {uint32 node; WorldPosition point; std::string const name; bool isPortal; bool
        //        isTransport; uint32 transportId; };
        out << std::fixed << std::setprecision(2) << "        addNodes.push_back(addNode{" << i << ",";
        out << "WorldPosition(" << node->GetMapId() << ", " << node->getX() << "f, " << node->getY() << "f, "
            << node->getZ() << "f, " << node->getO() << "f),";
        out << "\"" << name << "\"";
        if (node->isTransport())
            out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();
        out << "});";

        sPlayerbotAIConfig.log(nodeStore, out.str().c_str());

        saveNodes.insert(std::make_pair(node, i));
    }

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        for (auto& Link : *node->getLinks())
        {
            std::ostringstream out;

            //        struct linkNode { uint32 node1; uint32 node2; float distance; float extraCost; bool isPortal; bool
            //        isTransport; uint32 maxLevelMob; uint32 maxLevelAlliance; uint32 maxLevelHorde; float
            //        swimDistance; };

            out << std::fixed << std::setprecision(2) << "        linkNodes3.push_back(linkNode3{" << i << ","
                << saveNodes.find(Link.first)->second << ",";
            out << Link.second->print() << "});";

            sPlayerbotAIConfig.log(nodeStore, out.str().c_str());
        }
    }

    sPlayerbotAIConfig.log(nodeStore, "    }");
    sPlayerbotAIConfig.log(nodeStore, "};");

    printf("\r [Done] \r\x3D");
    fflush(stdout);
}

void TravelNodeMap::saveNodeStore()
{
    if (!hasToSave)
        return;

    hasToSave = false;

    constexpr uint32 STMTS_PER_TX = 500;  // bounded transaction size

    // Phase 1: deletes in their own transaction.
    {
        PlayerbotsDatabaseTransaction delTrans = PlayerbotsDatabase.BeginTransaction();
        delTrans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE));
        delTrans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE_LINK));
        delTrans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE_PATH));
        PlayerbotsDatabase.CommitTransaction(delTrans);
    }

    std::unordered_map<TravelNode*, uint32> saveNodes;
    std::vector<TravelNode*> anodes = TravelNodeMap::instance().getNodes();

    // Phase 2: node inserts, chunked at STMTS_PER_TX per transaction.
    {
        PlayerbotsDatabaseTransaction nodeTrans = PlayerbotsDatabase.BeginTransaction();
        uint32 inTx = 0;
        for (uint32 i = 0; i < anodes.size(); i++)
        {
            TravelNode* node = anodes[i];

            std::string name = node->getName();
            name.erase(remove(name.begin(), name.end(), '\''), name.end());

            PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_TRAVELNODE);
            stmt->SetData(0, i);
            stmt->SetData(1, name);
            stmt->SetData(2, node->GetMapId());
            stmt->SetData(3, node->getX());
            stmt->SetData(4, node->getY());
            stmt->SetData(5, node->getZ());
            stmt->SetData(6, node->isLinked());
            nodeTrans->Append(stmt);

            saveNodes.insert(std::make_pair(node, i));

            if (++inTx >= STMTS_PER_TX)
            {
                PlayerbotsDatabase.CommitTransaction(nodeTrans);
                nodeTrans = PlayerbotsDatabase.BeginTransaction();
                inTx = 0;
            }
        }
        PlayerbotsDatabase.CommitTransaction(nodeTrans);
    }

    LOG_INFO("playerbots", ">> Saved {} travelNodes.", anodes.size());

    // Phase 3: link inserts, chunked at STMTS_PER_TX per transaction.
    uint32 paths = 0;
    {
        PlayerbotsDatabaseTransaction linkTrans = PlayerbotsDatabase.BeginTransaction();
        uint32 inTx = 0;
        for (uint32 i = 0; i < anodes.size(); i++)
        {
            TravelNode* node = anodes[i];

            for (auto& link : *node->getLinks())
            {
                TravelNodePath* path = link.second;

                PlayerbotsDatabasePreparedStatement* stmt =
                    PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_TRAVELNODE_LINK);
                stmt->SetData(0, i);
                stmt->SetData(1, saveNodes.find(link.first)->second);
                stmt->SetData(2, static_cast<uint8>(path->getPathType()));
                stmt->SetData(3, path->getPathObject());
                stmt->SetData(4, path->getDistance());
                stmt->SetData(5, path->getSwimDistance());
                stmt->SetData(6, path->getExtraCost());
                stmt->SetData(7, path->getCalculated());
                stmt->SetData(8, path->getMaxLevelCreature()[0]);
                stmt->SetData(9, path->getMaxLevelCreature()[1]);
                stmt->SetData(10, path->getMaxLevelCreature()[2]);
                linkTrans->Append(stmt);

                paths++;

                if (++inTx >= STMTS_PER_TX)
                {
                    PlayerbotsDatabase.CommitTransaction(linkTrans);
                    linkTrans = PlayerbotsDatabase.BeginTransaction();
                    inTx = 0;
                }
            }
        }
        PlayerbotsDatabase.CommitTransaction(linkTrans);
    }

    // Phase 4: path points, committed every ~10000 rows. A single mega-transaction
    // (~1.5M rows) used to exceed MySQL limits and partial-commit, corrupting the store.
    constexpr uint32 BATCH_SIZE = 500;
    constexpr uint32 BATCHES_PER_COMMIT = 20;  // 20 * 500 = 10000 rows per tx
    uint32 points = 0;
    std::ostringstream ss;
    uint32 batchCount = 0;
    uint32 batchesInCurrentTx = 0;
    PlayerbotsDatabaseTransaction pathTrans = PlayerbotsDatabase.BeginTransaction();

    auto flushBatch = [&]()
    {
        if (batchCount == 0)
            return;

        std::string sql = ss.str();
        sql.back() = ';';  // Replace trailing comma
        pathTrans->Append(sql.c_str());
        ss.str("");
        ss.clear();
        batchCount = 0;
        batchesInCurrentTx++;
    };

    auto commitIfFull = [&]()
    {
        if (batchesInCurrentTx >= BATCHES_PER_COMMIT)
        {
            PlayerbotsDatabase.CommitTransaction(pathTrans);
            pathTrans = PlayerbotsDatabase.BeginTransaction();
            batchesInCurrentTx = 0;
        }
    };

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        for (auto& link : *node->getLinks())
        {
            TravelNodePath* path = link.second;
            uint32 toId = saveNodes.find(link.first)->second;
            std::vector<WorldPosition> ppath = path->GetPath();

            for (uint32 j = 0; j < ppath.size(); j++)
            {
                WorldPosition& point = ppath[j];

                if (batchCount == 0)
                    ss << "INSERT INTO `playerbots_travelnode_path` (`node_id`,`to_node_id`,`nr`,`map_id`,`x`,`y`,`z`) VALUES ";

                ss << std::fixed << std::setprecision(4)
                   << "(" << i << "," << toId << "," << j << ","
                   << point.GetMapId() << ","
                   << point.GetPositionX() << ","
                   << point.GetPositionY() << ","
                   << point.GetPositionZ() << "),";

                batchCount++;
                points++;

                if (batchCount >= BATCH_SIZE)
                {
                    flushBatch();
                    commitIfFull();
                }
            }
        }
    }

    flushBatch();
    PlayerbotsDatabase.CommitTransaction(pathTrans);

    LOG_INFO("playerbots", ">> Saved {} travelNode Paths, {} points.", paths, points);
    LOG_INFO("playerbots",
             ">> NOTE: writes are queued ASYNC. Run '.server shutdown 1' to flush "
             "the queue; killing the process now will lose pending rows.");
}

void TravelNodeMap::LoadNodeStore()
{
    std::unordered_map<uint32, TravelNode*> saveNodes;

    {
        if (PreparedQueryResult result =
                PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TRAVELNODE)))
        {
            do
            {
                Field* fields = result->Fetch();

                TravelNode* node = addNode(WorldPosition(fields[2].Get<uint32>(), fields[3].Get<float>(),
                                                         fields[4].Get<float>(), fields[5].Get<float>()),
                                           fields[1].Get<std::string>(), true, false);

                if (fields[6].Get<bool>())
                    node->setLinked(true);
                else
                    hasToGen = true;

                saveNodes.insert(std::make_pair(fields[0].Get<uint32>(), node));

            } while (result->NextRow());

            LOG_INFO("playerbots", ">> Loaded {} travelNodes.", saveNodes.size());
        }
        else
        {
            hasToFullGen = true;
            LOG_ERROR("playerbots", ">> Error loading travelNodes.");
        }
    }

    {
        if (PreparedQueryResult result =
                PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TRAVELNODE_LINK)))
        {
            do
            {
                Field* fields = result->Fetch();

                auto startIt = saveNodes.find(fields[0].Get<uint32>());
                auto endIt = saveNodes.find(fields[1].Get<uint32>());

                if (startIt == saveNodes.end() || endIt == saveNodes.end())
                    continue;

                TravelNode* startNode = startIt->second;
                TravelNode* endNode = endIt->second;

                startNode->setPathTo(
                    endNode,
                    TravelNodePath(fields[4].Get<float>(), fields[6].Get<float>(), fields[2].Get<uint8>(),
                                   fields[3].Get<uint64>(), fields[7].Get<bool>(),
                                   {fields[8].Get<uint8>(), fields[9].Get<uint8>(), fields[10].Get<uint8>()},
                                   fields[5].Get<float>()),
                    true);

                if (!fields[7].Get<bool>())
                    hasToGen = true;

            } while (result->NextRow());

            LOG_INFO("playerbots", ">> Loaded {} travelNode paths.", result->GetRowCount());
        }
        else
        {
            LOG_ERROR("playerbots", ">> Error loading travelNode links.");
        }
    }

    {
        if (PreparedQueryResult result =
                PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TRAVELNODE_PATH)))
        {
            do
            {
                Field* fields = result->Fetch();

                auto startIt = saveNodes.find(fields[0].Get<uint32>());
                auto endIt = saveNodes.find(fields[1].Get<uint32>());

                if (startIt == saveNodes.end() || endIt == saveNodes.end())
                    continue;

                TravelNode* startNode = startIt->second;
                TravelNode* endNode = endIt->second;

                if (!startNode->hasPathTo(endNode))
                    continue;

                TravelNodePath* path = startNode->getPathTo(endNode);

                std::vector<WorldPosition> ppath = path->GetPath();
                ppath.push_back(WorldPosition(fields[3].Get<uint32>(), fields[4].Get<float>(), fields[5].Get<float>(),
                                              fields[6].Get<float>()));

                path->setPath(ppath);

                if (path->getCalculated())
                    path->setComplete(true);

            } while (result->NextRow());

            LOG_INFO("playerbots", ">> Loaded {} travelNode paths points.", result->GetRowCount());
        }
        else
        {
            LOG_ERROR("playerbots", ">> Error loading travelNode paths.");
        }
    }
}

void TravelNodeMap::calcMapOffset()
{
    mapOffsets.push_back(std::make_pair(0, WorldPosition(0, 0, 0, 0, 0)));
    mapOffsets.push_back(std::make_pair(1, WorldPosition(1, -3680.0, 13670.0, 0, 0)));
    mapOffsets.push_back(std::make_pair(530, WorldPosition(530, 15000.0, -20000.0, 0, 0)));
    mapOffsets.push_back(std::make_pair(571, WorldPosition(571, 10000.0, 5000.0, 0, 0)));

    std::vector<uint32> mapIds;

    for (auto& node : _nodes)
    {
        if (!node->getPosition()->isOverworld())
            if (std::find(mapIds.begin(), mapIds.end(), node->GetMapId()) == mapIds.end())
                mapIds.push_back(node->GetMapId());
    }

    std::sort(mapIds.begin(), mapIds.end());

    std::vector<WorldPosition> min, max;

    for (auto& mapId : mapIds)
    {
        bool doPush = true;
        for (auto& node : _nodes)
        {
            if (node->GetMapId() != mapId)
                continue;

            if (doPush)
            {
                min.push_back(*node->getPosition());
                max.push_back(*node->getPosition());
                doPush = false;
            }
            else
            {
                min.back().setX(std::min(min.back().GetPositionX(), node->getX()));
                min.back().setY(std::min(min.back().GetPositionY(), node->getY()));
                max.back().setX(std::max(max.back().GetPositionX(), node->getX()));
                max.back().setY(std::max(max.back().GetPositionY(), node->getY()));
            }
        }
    }

    WorldPosition curPos = WorldPosition(0, -13000, -13000, 0, 0);
    WorldPosition endPos = WorldPosition(0, 3000, -13000, 0, 0);

    uint32 i = 0;
    float maxY = 0;
    //+X -> -Y
    for (auto& mapId : mapIds)
    {
        mapOffsets.push_back(std::make_pair(
            mapId, WorldPosition(mapId, curPos.GetPositionX() - min[i].GetPositionX(),
                                 curPos.GetPositionY() - max[i].GetPositionY(), 0, 0)));

        maxY = std::max(maxY, (max[i].GetPositionY() - min[i].GetPositionY() + 500));
        curPos.setX(curPos.GetPositionX() + (max[i].GetPositionX() - min[i].GetPositionX() + 500));

        if (curPos.GetPositionX() > endPos.GetPositionX())
        {
            curPos.setY(curPos.GetPositionY() - maxY);
            curPos.setX(-13000);
        }

        i++;
    }
}

WorldPosition TravelNodeMap::getMapOffset(uint32 mapId)
{
    for (auto& offset : mapOffsets)
    {
        if (offset.first == mapId)
            return offset.second;
    }

    return WorldPosition(mapId, 0, 0, 0, 0);
}

// TravelNodeMap taxi graph (BFS-based flight path lookup)
void TravelNodeMap::InitTaxiGraph()
{
    BuildTaxiGraph();
    ComputeAllPaths();
}

std::vector<uint32> TravelNodeMap::FindTaxiPath(uint32 fromNode, uint32 toNode)
{
    if (fromNode == toNode)
        return {};

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    TaxiNodesEntry const* endNode = sTaxiNodesStore.LookupEntry(toNode);

    if (!startNode || !endNode)
        return {};

    auto cacheItr = taxiPathCache.find(fromNode);
    if (cacheItr == taxiPathCache.end())
        return {};

    auto toNodeItr = cacheItr->second.find(toNode);
    if (toNodeItr == cacheItr->second.end())
        return {};

    return toNodeItr->second;
}

void TravelNodeMap::BuildTaxiGraph()
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
    }
    for (auto const& [node, neighbors] : tempGraph)
        taxiGraph[node] = std::vector<uint32>(neighbors.begin(), neighbors.end());
}

void TravelNodeMap::ComputeAllPaths()
{
    std::set<uint32> allNodes;
    for (auto const& [source, neighbors] : taxiGraph)
    {
        allNodes.insert(source);
        allNodes.insert(neighbors.begin(), neighbors.end());
    }

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

std::unordered_map<uint32, uint32> TravelNodeMap::BFS(uint32 fromNode)
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

        auto graphItr = taxiGraph.find(current);
        if (graphItr == taxiGraph.end())
            continue;

        for (uint32 next : graphItr->second)
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

std::vector<uint32> TravelNodeMap::BuildPath(uint32 fromNode, uint32 toNode,
                                              std::unordered_map<uint32, uint32> const& parentMap)
{
    if (!parentMap.count(toNode))
        return {}; // unreachable

    std::vector<uint32> path;
    uint32 current = toNode;
    while (current != fromNode)
    {
        path.push_back(current);
        auto it = parentMap.find(current);
        if (it == parentMap.end() || it->second == 0)
            break;
        current = it->second;
    }

    path.push_back(fromNode);
    std::reverse(path.begin(), path.end());
    return path;
}

// Logs the componentId partition: component count, main size, and a
// capped list of the rest. Components are weakly connected (undirected
// edges), so separation means no path in either direction.
void TravelNodeMap::logComponentConnectivity()
{
    // Detail-line cap so a badly fragmented graph doesn't flood the log.
    constexpr uint32 MAX_REPORTED_DISCONNECTED_COMPONENTS = 50;

    std::unordered_map<uint32, uint32> sizeById;
    std::unordered_map<uint32, TravelNode*> representativeById;

    uint32 totalNodes = 0;
    for (auto* node : _nodes)
    {
        if (!node)
            continue;

        ++totalNodes;
        uint32 const id = node->getComponentId();
        ++sizeById[id];
        representativeById.try_emplace(id, node);
    }

    if (sizeById.empty())
        return;

    uint32 mainId = 0;
    uint32 mainSize = 0;
    for (auto const& [id, size] : sizeById)
    {
        if (size > mainSize)
        {
            mainSize = size;
            mainId = id;
        }
    }

    if (sizeById.size() == 1)
    {
        LOG_INFO("playerbots", "-- component check: graph is fully connected ({} nodes).", mainSize);
        return;
    }

    LOG_INFO("playerbots",
             "-- component check: {} disconnected components, largest holds {} of {} nodes.",
             sizeById.size(), mainSize, totalNodes);

    uint32 reported = 0;
    for (auto const& [id, size] : sizeById)
    {
        if (id == mainId)
            continue;

        if (reported >= MAX_REPORTED_DISCONNECTED_COMPONENTS)
            continue;

        TravelNode* rep = representativeById[id];
        WorldPosition* pos = rep->getPosition();
        LOG_DEBUG("playerbots",
                 "---- component {}: {} node(s), representative '{}' (map {}, {:.1f},{:.1f},{:.1f})",
                 id, size, rep->getName(), rep->GetMapId(),
                 pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ());
        ++reported;
    }

    uint32 const remaining = static_cast<uint32>(sizeById.size()) - 1 - reported;
    if (remaining > 0)
        LOG_DEBUG("playerbots", "---- {} further disconnected component(s) not listed.", remaining);
}

void TravelNodeMap::PrecomputeReachability(bool reportComponents)
{
    std::unordered_map<TravelNode*, std::vector<TravelNode*>> reverseAdj;
    for (auto* node : _nodes)
    {
        if (!node)
            continue;

        for (auto const& link : *node->getLinks())
            if (link.first)
                reverseAdj[link.first].push_back(node);
    }

    std::vector<TravelNode*> neighbors;
    auto collectUndirectedNeighbors = [&](TravelNode* current)
    {
        neighbors.clear();

        for (auto const& link : *current->getLinks())
            if (link.first)
                neighbors.push_back(link.first);

        auto it = reverseAdj.find(current);
        if (it != reverseAdj.end())
            neighbors.insert(neighbors.end(), it->second.begin(), it->second.end());
    };

    // Global components: every link type, may cross maps via portal /
    // transport / flight edges. Feeds hasRouteTo(node) (mapOnly = false),
    // e.g. GetNodeRoute's runtime early-out.
    {
        std::unordered_set<TravelNode*> visited;
        uint32 nextId = 1;

        for (auto* node : _nodes)
        {
            if (!node || visited.count(node))
                continue;

            uint32 const id = nextId++;
            std::queue<TravelNode*> q;
            q.push(node);
            visited.insert(node);

            while (!q.empty())
            {
                TravelNode* current = q.front();
                q.pop();
                current->setComponentId(id);

                collectUndirectedNeighbors(current);
                for (auto* neighbor : neighbors)
                {
                    if (visited.count(neighbor))
                        continue;

                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }

    // Same-map components: cross-map links are never followed, giving a finer
    // partition. Feeds hasRouteTo(node, true), used by the crop pass so a cheap
    // cross-map portal "route" doesn't count as a same-map alternative.
    {
        std::unordered_set<TravelNode*> visited;
        uint32 nextId = 1;

        for (auto* node : _nodes)
        {
            if (!node || visited.count(node))
                continue;

            uint32 const id = nextId++;
            uint32 const mapId = node->GetMapId();
            std::queue<TravelNode*> q;
            q.push(node);
            visited.insert(node);

            while (!q.empty())
            {
                TravelNode* current = q.front();
                q.pop();
                current->setMapComponentId(id);

                collectUndirectedNeighbors(current);
                for (auto* neighbor : neighbors)
                {
                    if (visited.count(neighbor) || neighbor->GetMapId() != mapId)
                        continue;

                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }

    if (reportComponents)
        logComponentConnectivity();
}

