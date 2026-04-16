/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TravelNode.h"

#include <array>
#include <iomanip>
#include <queue>
#include <regex>
#include <unordered_set>

#include "BudgetValues.h"
#include "MapMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "RaceMgr.h"
#include "ServerFacade.h"
#include "TransportMgr.h"

// TravelNodePath(float distance = 0.1f, float extraCost = 0, TravelNodePathType pathType = TravelNodePathType::walk,
// uint32 pathObject = 0, bool calculated = false, std::vector<uint8> maxLevelCreature = { 0,0,0 }, float swimDistance =
// 0)
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

// Gets the extra information needed to properly calculate the cost.
void TravelNodePath::calculateCost(bool distanceOnly)
{
    std::unordered_map<FactionTemplateEntry const*, bool> aReact, hReact;

    bool aFriend, hFriend;

    if (calculated)
        return;

    distance = 0.1f;
    maxLevelCreature = {0, 0, 0};
    swimDistance = 0;

    WorldPosition lastPoint = WorldPosition();
    for (auto& point : path)
    {
        if (!distanceOnly)
        {
            for (CreatureData const* cData : point.getCreaturesNear(50))  // Agro radius + 5
            {
                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cData->id1);
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

// The cost to travel this path.
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
                return -1;

            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

            if (!taxiPath)
                return -1;

            if (!bot->isTaxiCheater() && taxiPath->price > cGold)
                return -1;

            if (!bot->isTaxiCheater() && !bot->m_taxi.IsTaximaskNodeKnown(taxiPath->to))
                return -1;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);
            if (!startTaxiNode || !endTaxiNode ||
                !startTaxiNode->MountCreatureID[bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 0] ||
                !endTaxiNode->MountCreatureID[bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 0])
                return -1;
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
                factionAnnoyance = (maxLevelCreature[2] - level) - 10;  // Opposite faction below 30 do not bother us.
            else if (!isAlliance)
                factionAnnoyance = (maxLevelCreature[1] - level) - 10;

            if (mobAnnoyance > 0)
                modifier += 0.1 * mobAnnoyance;  // For each level the whole path takes 10% longer.
            if (factionAnnoyance > 0)
                modifier += 0.3 * factionAnnoyance;  // For each level the whole path takes 10% longer.
        }
        if (getPathType() == TravelNodePathType::flyingMount)
        {
            if (!bot->IsAlive() || bot->GetLevel() < 70 || !bot->CanFly())
                return -1;

            float flySpeed = bot->GetSpeed(MOVE_FLIGHT);
            if (flySpeed < 1.0f)
                flySpeed = 20.0f;  // 280% base flying speed fallback
            return (distance / flySpeed) * modifier;
        }
    }
    else if (getPathType() == TravelNodePathType::flightPath || getPathType() == TravelNodePathType::flyingMount)
        return -1;

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

// Creates or appends the path from one node to another. Returns if the path.
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

    if (!canPath && endNode->hasLinkTo(this))  // Unable to find a path? See if the reverse is possible.
    {
        TravelNodePath backNodePath = *endNode->getPathTo(this);

        if (backNodePath.getPathType() == TravelNodePathType::walk)
        {
            std::vector<WorldPosition> bPath = backNodePath.GetPath();

            if (!backNodePath.getComplete())  // Build it if it's not already complete.
            {
                if (bPath.empty())
                    bPath = {*endNode->getPosition()};  // Start the path from the end Node.

                WorldPosition* thisPos = getPosition();  // Build the path to this Node.

                bPath = thisPos->getPathFromPath(bPath, bot);  // Pathfind from the existing path to the this Node.

                canPath = thisPos->isPathTo(bPath);  // Check if we reached our destination.
            }
            else
                canPath = true;

            if (canPath)
            {
                std::reverse(bPath.begin(), bPath.end());
                path = bPath;
            }
        }
    }

    // Transports are (probably?) not solid at this moment. We need to walk over them so we need extra code for this.
    // Some portals are 'too' solid so we can't properly walk in them. Again we need to bypass this.
    if (!isTransport() && !isPortal() && (endNode->isPortal() || endNode->isTransport()))
    {
        if (endNode->isTransport() && path.back().isInWater())  // Do not swim to boats.
            canPath = false;
        else if (!canPath && endPos->isPathTo(path, 20.0f))  // Cheat a little for transports and portals.
        {
            path.push_back(*endPos);
            canPath = true;

            if (!endNode->hasPathTo(this) || !endNode->getPathTo(this)->getComplete())
            {
                std::vector<WorldPosition> reversePath = path;
                std::reverse(reversePath.begin(), reversePath.end());

                TravelNodePath* backNodePath = endNode->setPathTo(this, TravelNodePath(), false);

                backNodePath->setComplete(canPath);

                endNode->setLinkTo(this, true);

                backNodePath->setPath(reversePath);

                backNodePath->calculateCost(!postProcess);
            }
        }
    }

    if (isTransport() && path.size() > 1)
    {
        WorldPosition secondPos =
            *std::next(path.begin());  // This is to prevent bots from jumping in the water from a transport. Need to
                                       // remove this when transports are properly handled.
        if (secondPos.getMap() && secondPos.isInWater())
            canPath = false;
    }

    returnNodePath->setComplete(canPath);

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
        endNode->setLinkTo(this, true);

        if (!backNodePath->getCalculated())
        {
            backNodePath->calculateCost(!postProcess);
        }
    }

    return returnNodePath;
}

// Generic routine to remove references to nodes.
void TravelNode::removeLinkTo(TravelNode* node, bool removePaths)
{
    if (node)  // Unlink this specific node
    {
        if (removePaths)
            paths.erase(node);

        links.erase(node);
        routes.erase(node);
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
        routes.clear();
    }
}

std::vector<TravelNode*> TravelNode::getNodeMap(bool importantOnly, std::vector<TravelNode*> ignoreNodes)
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
    if (hasLinkTo(farNode))
        farLength = getPathTo(farNode)->getDistance();
    else
        farLength = getDistance(farNode);

    for (auto& link : *getLinks())
    {
        TravelNode* nearNode = link.first;
        float nearLength = link.second->getDistance();

        if (farNode == nearNode)
            continue;

        if (farNode->hasLinkTo(this) && !nearNode->hasLinkTo(this))
            continue;

        if (nearNode->hasLinkTo(farNode))
        {
            // Is it quicker to go past second node to reach first node instead of going directly?
            if (nearLength + nearNode->linkDistanceTo(farNode) < farLength * 1.1)
                return true;
        }
        else
        {
            TravelNodeRoute route = TravelNodeMap::instance().GetNodeRoute(nearNode, farNode, nullptr);

            if (route.isEmpty())
                continue;

            if (route.hasNode(this))
                continue;

            // Is it quicker to go past second (and multiple) nodes to reach the first node instead of going directly?
            if (nearLength + route.getTotalDistance() < farLength * 1.1)
                return true;
        }
    }

    return false;
}

void TravelNode::cropUselessLink(TravelNode* farNode)
{
    if (isUselessLink(farNode))
        removeLinkTo(farNode);
}

bool TravelNode::cropUselessLinks()
{
    bool hasRemoved = false;

    for (auto& firstLink : *getPaths())
    {
        TravelNode* farNode = firstLink.first;
        if (this->hasLinkTo(farNode) && this->isUselessLink(farNode))
        {
            LOG_DEBUG("playerbots", "[CropLink] '{}' → '{}' (dist {:.0f}) — redundant, removing",
                      getName(), farNode->getName(), getPathTo(farNode)->getDistance());
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

        if (farNode->hasLinkTo(this) && farNode->isUselessLink(this))
        {
            LOG_DEBUG("playerbots", "[CropLink] '{}' → '{}' (dist {:.0f}) — redundant, removing",
                      farNode->getName(), getName(), farNode->getPathTo(this)->getDistance());
            farNode->removeLinkTo(this);
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
    }

    return hasRemoved;

    /*

    //std::vector<std::pair<TravelNode*, TravelNode*>> toRemove;
    for (auto& firstLink : getLinks())
    {

        TravelNode* firstNode = firstLink.first;
        float firstLength = firstLink.second.getDistance();
        for (auto& secondLink : getLinks())
        {
            TravelNode* secondNode = secondLink.first;
            float secondLength = secondLink.second.getDistance();

            if (firstNode == secondNode)
                continue;

            if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](std::pair<TravelNode*, TravelNode*>
    pair) {return pair.first == firstNode || pair.first == secondNode;}) != toRemove.end()) continue;

            if (firstNode->hasLinkTo(secondNode))
            {
                //Is it quicker to go past first node to reach second node instead of going directly?
                if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
            else
            {
                TravelNodeRoute route = TravelNodeMap::instance().GetNodeRoute(firstNode, secondNode, nullptr);

                if (route.isEmpty())
                    continue;

                if (route.hasNode(this))
                    continue;

                //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going
    directly? if (firstLength + route.getLength() < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
        }

        //Reverse cleanup. This is needed when we add a node in an existing map.
        if (firstNode->hasLinkTo(this))
        {
            firstLength = firstNode->getPathTo(this)->getDistance();

            for (auto& secondLink : firstNode->getLinks())
            {
                TravelNode* secondNode = secondLink.first;
                float secondLength = secondLink.second.getDistance();

                if (this == secondNode)
                    continue;

                if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](std::pair<TravelNode*,
    TravelNode*> pair) {return pair.first == firstNode || pair.first == secondNode; }) != toRemove.end()) continue;

                if (firstNode->hasLinkTo(secondNode))
                {
                    //Is it quicker to go past first node to reach second node instead of going directly?
                    if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
                else
                {
                    TravelNodeRoute route = TravelNodeMap::instance().GetNodeRoute(firstNode, secondNode, nullptr);

                    if (route.isEmpty())
                        continue;

                    if (route.hasNode(this))
                        continue;

                    //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going
    directly? if (firstLength + route.getLength() < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
            }
        }

    }

    for (auto& nodePair : toRemove)
        nodePair.first->unlinkNode(nodePair.second, false);
        */
}

bool TravelNode::isEqual(TravelNode* compareNode)
{
    if (!hasLinkTo(compareNode))
        return false;

    if (!compareNode->hasLinkTo(this))
        return false;

    for (auto& node : TravelNodeMap::instance().getNodes())
    {
        if (node == this || node == compareNode)
            continue;

        if (node->hasLinkTo(this) != node->hasLinkTo(compareNode))
            return false;

        if (hasLinkTo(node) != compareNode->hasLinkTo(node))
            return false;
    }

    return true;
}

void TravelNode::print([[maybe_unused]] bool printFailed)
{
    // WorldPosition* startPosition = getPosition(); //not used, line marked for removal.

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

// Attempts to move ahead of the path.
bool TravelPath::makeShortCut(WorldPosition startPos, float maxDist)
{
    if (GetPath().empty())
        return false;

    float maxDistSq = maxDist * maxDist;
    float minDist = -1;
    float totalDist = fullPath.begin()->point.sqDistance(startPos);
    std::vector<PathNodePoint> newPath;
    WorldPosition firstNode;

    for (auto& p : fullPath)  // cycle over the full path
    {
        // if (p.point.GetMapId() != startPos.GetMapId())
        //    continue;

        if (p.point.GetMapId() == startPos.GetMapId())
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

    // The old path seems to be the best.
    if (beginPos.distance(firstNode) < sPlayerbotAIConfig.tooCloseDistance)
        return false;

    // We are (nearly) on the new path. Just follow the rest.
    if (beginPos.distance(startPos) < sPlayerbotAIConfig.tooCloseDistance)
    {
        fullPath = newPath;
        return true;
    }

    std::vector<WorldPosition> toPath = startPos.getPathTo(beginPos, nullptr);

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

            if (!nodePath || !nodePath->getComplete())  // Build the path to the next node if it doesn't exist.
            {
                // Only attempt runtime path building when we have a bot entity.
                if (bot)
                {
                    if (!prevNode->isTransport())
                        nodePath = prevNode->BuildPath(node, bot);
                    else
                    {
                        node->BuildPath(prevNode, bot);
                        nodePath = prevNode->getPathTo(node);
                    }
                }
            }

            TravelNodePath returnNodePath;

            if (!nodePath || !nodePath->getComplete())
            {
                if (bot)
                {
                    returnNodePath =
                        *node->BuildPath(prevNode, bot);
                    std::vector<WorldPosition> path = returnNodePath.GetPath();
                    std::reverse(path.begin(), path.end());
                    returnNodePath.setPath(path);
                    nodePath = &returnNodePath;
                }
            }

            if (!nodePath || !nodePath->getComplete())  // If we can not build a path just try to move to the node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_NODE);
                prevNode = node;
                continue;
            }

            if (nodePath->getPathType() == TravelNodePathType::portal ||
                nodePath->getPathType() == TravelNodePathType::staticPortal)  // Teleport to next node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_PORTAL, nodePath->getPathObject());  // Entry point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_PORTAL, nodePath->getPathObject());      // Exit point
            }
            else if (nodePath->getPathType() == TravelNodePathType::transport)  // Move onto transport
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TRANSPORT,
                                    nodePath->getPathObject());  // Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject());  // Arrival point
            }
            else if (nodePath->getPathType() == TravelNodePathType::flightPath)  // Use the flightpath
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_FLIGHTPATH,
                                    nodePath->getPathObject());  // Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject());  // Arrival point
            }
            else if (nodePath->getPathType() == TravelNodePathType::teleportSpell)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
            }
            else if (nodePath->getPathType() == TravelNodePathType::flyingMount)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_FLYING_MOUNT, 0);
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_FLYING_MOUNT, 0);
            }
            else
            {
                std::vector<WorldPosition> path = nodePath->GetPath();

                if (path.size() > 1 &&
                    node != nodes.back())  // Remove the last point since that will also be the start of the next path.
                    path.pop_back();

                if (path.size() > 1 && prevNode->isPortal() &&
                    nodePath->getPathType() != TravelNodePathType::portal &&
                    nodePath->getPathType() != TravelNodePathType::staticPortal)  // Do not move to the area trigger if we
                                                                                  // don't plan to take the portal.
                    path.erase(path.begin());

                if (path.size() > 1 && prevNode->isTransport() &&
                    nodePath->getPathType() !=
                        TravelNodePathType::transport)  // Do not move to the transport if we aren't going to take it.
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

    nodes.push_back(newNode);

    return newNode;
}

void TravelNodeMap::removeNode(TravelNode* node)
{
    node->removeLinkTo(nullptr, true);

    for (auto& tnode : nodes)
    {
        if (tnode == node)
        {
            delete tnode;
            tnode = nullptr;
        }
    }

    nodes.erase(std::remove(nodes.begin(), nodes.end(), nullptr), nodes.end());
}

void TravelNodeMap::fullLinkNode(TravelNode* startNode, Unit* bot)
{
    WorldPosition* startPosition = startNode->getPosition();
    std::vector<TravelNode*> linkNodes = getNodes(*startPosition);

    for (auto& endNode : linkNodes)
    {
        if (endNode == startNode)
            continue;

        if (startNode->hasLinkTo(endNode))
            continue;

        startNode->BuildPath(endNode, bot);
        endNode->BuildPath(startNode, bot);
    }

    startNode->setLinked(true);
}

std::vector<TravelNode*> TravelNodeMap::getNodes(WorldPosition pos, float range)
{
    std::vector<TravelNode*> retVec;

    for (auto& node : nodes)
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
    //float x = pos.getX(); //not used, line marked for removal.
    //float y = pos.getY(); //not used, line marked for removal.
    //float z = pos.getZ(); //not used, line marked for removal.

    if (bot && !bot->GetMap())
        return nullptr;

    uint32 c = 0;

    std::vector<TravelNode*> nodes = TravelNodeMap::instance().getNodes(pos, range);
    for (auto& node : nodes)
    {
        if (!bot || pos.canPathTo(*node->getPosition(), bot))
            return node;

        c++;

        if (c > 5)  // Max 5 attempts
            break;
    }

    return nullptr;
}

TravelNodeRoute TravelNodeMap::GetNodeRoute(TravelNode* start, TravelNode* goal,
    Player* bot)
{
    float botSpeed = bot ? bot->GetSpeed(MOVE_RUN) : 7.0f;

    if (start == goal)
        return TravelNodeRoute();

    // Basic A* algorithm
    std::unordered_map<TravelNode*, TravelNodeStub> m_stubs;

    TravelNodeStub* startStub = &m_stubs.insert(std::make_pair(start, TravelNodeStub(start))).first->second;

    TravelNodeStub* currentNode = nullptr;
    TravelNodeStub* childNode = nullptr;
    float f = 0.f;
    float g = 0.f;
    float h = 0.f;

    std::vector<TravelNodeStub*> open, closed;

    if (bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
        {
            if (botAI->HasCheat(BotCheatMask::gold))
                startStub->currentGold = 10000000;
            else
            {
                AiObjectContext* context = botAI->GetAiObjectContext();
                startStub->currentGold = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::travel);
            }
        }
        else
            startStub->currentGold = bot->GetMoney();
    }

    if (!start->hasRouteTo(goal))
        return TravelNodeRoute();

    // Min-heap: smallest f at front
    auto heapComp = [](TravelNodeStub* i, TravelNodeStub* j) { return i->totalCost > j->totalCost; };

    open.push_back(startStub);
    std::push_heap(open.begin(), open.end(), heapComp);
    startStub->open = true;

    constexpr uint32 MAX_A_STAR_EXPLORED = 500;
    uint32 nodesExplored = 0;

    while (!open.empty())
    {
        if (++nodesExplored > MAX_A_STAR_EXPLORED)
            return TravelNodeRoute();

        std::pop_heap(open.begin(), open.end(), heapComp);
        currentNode = open.back();
        open.pop_back();
        currentNode->open = false;

        currentNode->closed = true;
        closed.push_back(currentNode);

        if (currentNode->dataNode == goal)
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

            if (childNode->closed)
                childNode->closed = false;

            if (!childNode->open)
            {
                open.push_back(childNode);
                std::push_heap(open.begin(), open.end(), heapComp);
                childNode->open = true;
            }
        }
    }

    return TravelNodeRoute();
}

TravelNodeRoute TravelNodeMap::GetNearestNodes(WorldPosition startPos, WorldPosition endPos,
                                            std::vector<WorldPosition>& startPath, Player* bot)
{
    if (nodes.empty() || !bot)
        return TravelNodeRoute();

    constexpr uint32 K = 3;
    if (nodes.size() < K)
        return TravelNodeRoute();

    // Single copy of the node list, find closest K for start and end
    std::vector<TravelNode*> nodesCopy = this->nodes;

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

bool TravelNodeMap::GetFullPath(TravelPlan& plan,
    WorldPosition botPos, uint32 botZoneId,
    uint32 teamId, WorldPosition destination)
{
    plan.Reset();
    plan.destination = destination;

    // Short distance — direct walk, no nodes needed
    if (botPos.fDist(destination) < MAX_PATHFINDING_DISTANCE &&
        botPos.GetMapId() == destination.GetMapId())
    {
        plan.steps.addPoint(botPos, PathNodeType::NODE_PREPATH);
        plan.steps.addPoint(destination, PathNodeType::NODE_PATH);
        return true;
    }

    std::shared_lock<std::shared_timed_mutex> guard(m_nMapMtx);

    // Find nearest nodes (zone-indexed, fast)
    TravelNode* startNode = GetNearestNodeInZone(botPos, botZoneId);
    if (!startNode)
        startNode = GetNearestNodeOnMap(botPos);

    uint32 destZone = sMapMgr->GetZoneId(PHASEMASK_NORMAL, destination);
    TravelNode* endNode = GetNearestNodeInZone(destination, destZone);
    if (!endNode)
        endNode = GetNearestNodeOnMap(destination);

    if (!startNode || !endNode)
        return false;

    if (!startNode->hasRouteTo(endNode))
        return false;

    TravelNodeRoute route = GetNodeRoute(startNode, endNode, nullptr);
    if (route.isEmpty())
        return false;

    // Build flat waypoint path from A* route
    std::vector<WorldPosition> pathToStart = {botPos};
    std::vector<WorldPosition> pathToEnd = {destination};
    plan.steps = route.BuildPath(pathToStart, pathToEnd, nullptr);

    LOG_DEBUG("playerbots",
        "[TravelPlan] '{}' → '{}', {} points",
        startNode->getName(), endNode->getName(),
        plan.steps.size());

    return !plan.steps.empty();
}

bool TravelNodeMap::cropUselessNode(TravelNode* startNode)
{
    if (!startNode->isLinked() || startNode->isImportant())
        return false;

    std::vector<TravelNode*> ignore = {startNode};

    for (auto& node : getNodes(*startNode->getPosition(), 5000.f))
    {
        if (startNode == node)
            continue;

        if (node->getNodeMap(true).size() > node->getNodeMap(true, ignore).size())
            return false;
    }

    LOG_INFO("playerbots", "[CropNode] Removing useless node '{}' (map {}, {:.0f},{:.0f},{:.0f}) — no neighbor depends on it",
             startNode->getName(), startNode->GetMapId(), startNode->getX(), startNode->getY(), startNode->getZ());

    removeNode(startNode);

    return true;
}

TravelNode* TravelNodeMap::addZoneLinkNode(TravelNode* startNode)
{
    for (auto& path : *startNode->getPaths())
    {
        //TravelNode* endNode = path.first; //not used, line marked for removal.

        std::string zoneName = startNode->getPosition()->getAreaName(true, true);
        for (auto& pos : path.second.GetPath())
        {
            std::string const newZoneName = pos.getAreaName(true, true);
            if (zoneName != newZoneName)
            {
                if (!getNode(pos, nullptr, 100.0f))
                {
                    std::string const nodeName = zoneName + " to " + newZoneName;
                    return TravelNodeMap::instance().addNode(pos, nodeName, false, true);
                }

                zoneName = newZoneName;
            }
        }
    }

    return nullptr;
}

TravelNode* TravelNodeMap::addRandomExtNode(TravelNode* startNode)
{
    std::unordered_map<TravelNode*, TravelNodePath> paths = *startNode->getPaths();

    if (paths.empty())
        return nullptr;

    for (uint32 i = 0; i < 20; i++)
    {
        auto random_it = std::next(std::begin(paths), urand(0, paths.size() - 1));

        TravelNode* endNode = random_it->first;
        std::vector<WorldPosition> path = random_it->second.GetPath();

        if (path.empty())
            continue;

        // Prefer to skip complete links
        if (endNode->hasLinkTo(startNode) && startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        // Prefer to skip no links
        if (!startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        WorldPosition point = path[urand(0, path.size() - 1)];

        if (!getNode(point, nullptr, 100.0f))
            return TravelNodeMap::instance().addNode(point, startNode->getName(), false, true);
    }

    return nullptr;
}

void TravelNodeMap::manageNodes(Unit* bot, bool mapFull)
{
    // Runtime node mutation disabled — the graph is fully built at startup via Init()
    // and .generate. Taking a unique_lock here caused 100-250ms contention spikes for
    // all bot threads holding shared_locks in TravelPlan.
    // Node pruning/creation is still available via the .generate console command.
}

void TravelNodeMap::generateNpcNodes()
{
    std::unordered_map<uint32, std::pair<CreatureTemplate const*, WorldPosition>> bossMap;

    for (auto& creatureData : WorldPosition().getCreaturesNear())
    {
        WorldPosition guidP(creatureData->mapid, creatureData->posX, creatureData->posY, creatureData->posZ,
                            creatureData->orientation);

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creatureData->id1);
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

            /*TravelNode* node = */ TravelNodeMap::instance().addNode(guidP, nodeName, true, true); //node not used, fragment marked for removal.
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

    // Exit nodes

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

        //TravelNode* entryNode = TravelNodeMap::instance().getNode(outPos, nullptr, 20.0f);  // Entry side, portal exit. //not used, line marked for removal.

        TravelNode* outNode = TravelNodeMap::instance().addNode(outPos, nodeName, true, true);  // Exit size, portal exit.

        TravelNode* inNode = TravelNodeMap::instance().getNode(inPos, nullptr, 5.0f);  // Entry side, portal center.

        // Portal link from area trigger to area trigger destination.
        if (outNode && inNode)
        {
            TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::portal, itr.first, true);
            travelPath.setPath({*inNode->getPosition(), *outNode->getPosition()});
            inNode->setPathTo(outNode, travelPath);
        }
    }
}

void TravelNodeMap::generateTransportNodes()
{
    for (auto const& itr : *sObjectMgr->GetGameObjectTemplates())
    {
        GameObjectTemplate const* data = &itr.second;
        if (!data || (data->type != GAMEOBJECT_TYPE_TRANSPORT && data->type != GAMEOBJECT_TYPE_MO_TRANSPORT))
            continue;

        uint32 pathId = data->moTransport.taxiPathId;
        float moveSpeed = data->moTransport.moveSpeed;
        if (pathId >= sTaxiPathNodesByPath.size())
            continue;

        TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

        // Keep only transports with taxi paths (boats/zeppelins).
        if (path.empty())
            continue;

        std::vector<WorldPosition> ppath;
        TravelNode* prevNode = nullptr;

        // Loop over the path and connect stop locations.
        for (auto& p : path)
        {
            WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

            if (prevNode)
                ppath.push_back(pos);

            if (p->delay > 0)
            {
                TravelNode* node = TravelNodeMap::instance().addNode(pos, data->name, true, true, true, itr.first);

                if (!prevNode)
                {
                    ppath.push_back(pos);
                }
                else
                {
                    TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, itr.first, true);
                    travelPath.setPathAndCost(ppath, moveSpeed);
                    node->setPathTo(prevNode, travelPath);
                    ppath.clear();
                    ppath.push_back(pos);
                }

                prevNode = node;
            }
        }

        if (!prevNode)
            continue;

        // Continue from start until first stop and connect to end.
        for (auto& p : path)
        {
            WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);
            ppath.push_back(pos);

            if (p->delay > 0)
            {
                TravelNode* node = TravelNodeMap::instance().getNode(pos, nullptr, 5.0f);

                if (node != prevNode)
                {
                    TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, itr.first, true);
                    travelPath.setPathAndCost(ppath, moveSpeed);

                    node->setPathTo(prevNode, travelPath);
                }
            }
        }
        ppath.clear();
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

        /*TravelNode* node = */TravelNodeMap::instance().addNode(pos, pos.getAreaName(), true, true, false); //node not used, but addNode as side effect, fragment marked for removal.
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
}

void TravelNodeMap::generateWalkPaths()
{
    // Pathfinder
    std::vector<WorldPosition> ppath;

    std::map<uint32, bool> nodeMaps;

    uint32 totalProcessed = 0, totalSkipped = 0;

    for (auto& startNode : TravelNodeMap::instance().getNodes())
    {
        nodeMaps[startNode->GetMapId()] = true;
    }

    for (auto& map : nodeMaps)
    {
        uint32 mapProcessed = 0, mapSkipped = 0;
        for (auto& startNode : TravelNodeMap::instance().getNodes(WorldPosition(map.first, 1, 1)))
        {
            if (startNode->isLinked())
            {
                mapSkipped++;
                continue;
            }

            uint32 nearby = 0;
            uint32 linked = 0;
            for (auto& endNode : TravelNodeMap::instance().getNodes(*startNode->getPosition(), 2000.0f))
            {
                if (startNode == endNode)
                    continue;

                if (startNode->hasCompletePathTo(endNode))
                    continue;

                if (startNode->GetMapId() != endNode->GetMapId())
                    continue;

                nearby++;
                startNode->BuildPath(endNode, nullptr, false);
                if (startNode->hasCompletePathTo(endNode))
                    linked++;
            }

            LOG_INFO("playerbots", "  Node '{}' (map {}): {} nearby, {} linked",
                     startNode->getName(), startNode->GetMapId(), nearby, linked);

            startNode->setLinked(true);
            mapProcessed++;
        }

        LOG_INFO("playerbots", "[WalkPaths] Map {}: processed {} nodes, skipped {} (already linked)",
                 map.first, mapProcessed, mapSkipped);
        totalProcessed += mapProcessed;
        totalSkipped += mapSkipped;
    }

    LOG_INFO("playerbots", ">> Generated paths for {} nodes ({} processed, {} skipped as already linked).",
             TravelNodeMap::instance().getNodes().size(), totalProcessed, totalSkipped);
}

void TravelNodeMap::generateTaxiPaths()
{
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
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

        startNode->setPathTo(endNode, travelPath);
    }
}

void TravelNodeMap::removeLowNodes()
{
    std::vector<TravelNode*> goodNodes;
    std::vector<TravelNode*> remNodes;
    uint32 totalOverworld = 0;
    for (auto& node : TravelNodeMap::instance().getNodes())
    {
        if (!node->getPosition()->isOverworld())
            continue;

        totalOverworld++;

        if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
            continue;

        if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
            continue;

        std::vector<TravelNode*> nodes = node->getNodeMap(true);

        if (nodes.size() < 5)
        {
            std::vector<TravelNode*> allInCluster = node->getNodeMap();
            LOG_INFO("playerbots", "[RemoveLow] Cluster starting at '{}' (map {}) has only {} important nodes, {} total — marking for removal:",
                     node->getName(), node->GetMapId(), nodes.size(), allInCluster.size());
            for (auto& rn : allInCluster)
                LOG_INFO("playerbots", "[RemoveLow]   - '{}' (map {}, {:.0f},{:.0f},{:.0f}) important={}",
                         rn->getName(), rn->GetMapId(), rn->getX(), rn->getY(), rn->getZ(), rn->isImportant());
            remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
        }
        else
            goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
    }

    LOG_INFO("playerbots", "[RemoveLow] {} overworld nodes evaluated, {} good, {} to remove",
             totalOverworld, goodNodes.size(), remNodes.size());

    for (auto& node : remNodes)
        TravelNodeMap::instance().removeNode(node);
}

void TravelNodeMap::removeUselessPaths()
{
    uint32 linksBefore = 0;
    for (auto& n : TravelNodeMap::instance().getNodes())
        linksBefore += n->getLinks()->size();

    // Clean up node links
    for (auto& startNode : TravelNodeMap::instance().getNodes())
    {
        for (auto& path : *startNode->getPaths())
            if (path.second.getComplete() && startNode->hasLinkTo(path.first))
                ASSERT(true);
    }
    uint32 it = 0, totalRemoved = 0;
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

        hasToSave = true;
        totalRemoved += rem;
        it++;

        LOG_INFO("playerbots", "[RemoveUseless] Iteration {}, removed {} links from nodes", it, rem);
    }

    uint32 linksAfter = 0;
    for (auto& n : TravelNodeMap::instance().getNodes())
        linksAfter += n->getLinks()->size();

    LOG_INFO("playerbots", "[RemoveUseless] Done: {} iterations, links {} → {} (removed {})",
             it, linksBefore, linksAfter, linksBefore - linksAfter);
}

void TravelNodeMap::calculatePathCosts()
{
    for (auto& startNode : TravelNodeMap::instance().getNodes())
    {
        for (auto& path : *startNode->getLinks())
        {
            TravelNodePath* nodePath = path.second;

            if (path.second->getPathType() != TravelNodePathType::walk)
                continue;

            if (nodePath->getCalculated())
                continue;

            nodePath->calculateCost();
        }
    }

    LOG_INFO("playerbots", ">> Calculated pathcost for {} nodes.", TravelNodeMap::instance().getNodes().size());
}

void TravelNodeMap::generatePaths(bool fullGen)
{
    uint32 totalLinks = 0;
    uint32 totalPathPoints = 0;
    for (auto& n : TravelNodeMap::instance().getNodes())
    {
        totalLinks += n->getLinks()->size();
        for (auto& l : *n->getLinks())
            totalPathPoints += l.second->GetPath().size();
    }
    LOG_INFO("playerbots", "[GenPaths] ENTRY (fullGen={}): {} nodes, {} links, {} path points",
             fullGen, TravelNodeMap::instance().getNodes().size(), totalLinks, totalPathPoints);

    LOG_INFO("playerbots", "-Calculating walkable paths");
    generateWalkPaths();

    totalLinks = 0;
    for (auto& n : TravelNodeMap::instance().getNodes())
        totalLinks += n->getLinks()->size();
    LOG_INFO("playerbots", "[GenPaths] After generateWalkPaths: {} nodes, {} links",
             TravelNodeMap::instance().getNodes().size(), totalLinks);

    if (fullGen)
    {
        LOG_INFO("playerbots", "-Removing useless nodes");
        removeLowNodes();

        totalLinks = 0;
        for (auto& n : TravelNodeMap::instance().getNodes())
            totalLinks += n->getLinks()->size();
        LOG_INFO("playerbots", "[GenPaths] After removeLowNodes: {} nodes, {} links",
                 TravelNodeMap::instance().getNodes().size(), totalLinks);

        LOG_INFO("playerbots", "-Removing useless paths");
        removeUselessPaths();

        totalLinks = 0;
        for (auto& n : TravelNodeMap::instance().getNodes())
            totalLinks += n->getLinks()->size();
        LOG_INFO("playerbots", "[GenPaths] After removeUselessPaths: {} nodes, {} links",
                 TravelNodeMap::instance().getNodes().size(), totalLinks);
    }
    else
        LOG_INFO("playerbots", "-Skipping node/path pruning (incremental generation)");

    LOG_INFO("playerbots", "-Calculating path costs");
    calculatePathCosts();
    LOG_INFO("playerbots", "-Generating taxi paths");
    generateTaxiPaths();

    totalLinks = 0;
    totalPathPoints = 0;
    for (auto& n : TravelNodeMap::instance().getNodes())
    {
        totalLinks += n->getLinks()->size();
        for (auto& l : *n->getLinks())
            totalPathPoints += l.second->GetPath().size();
    }
    LOG_INFO("playerbots", "[GenPaths] EXIT: {} nodes, {} links, {} path points",
             TravelNodeMap::instance().getNodes().size(), totalLinks, totalPathPoints);
}

void TravelNodeMap::generateAll()
{
    LOG_INFO("playerbots", "[GenerateAll] Regenerating: {} nodes", nodes.size());

    generatePaths(false);
    saveNodeStore();

    BuildZoneIndex();
    PrecomputeReachability();
    LOG_INFO("playerbots", "[GenerateAll] Done: {} nodes, indexes rebuilt.", nodes.size());
}

void TravelNodeMap::Init()
{
    LoadNodeStore();
    calcMapOffset();

    if (hasToGen || hasToFullGen)
    {
        LOG_INFO("playerbots", "[Init] Generating paths (fullGen={}, {} nodes)...", hasToFullGen, nodes.size());

        if (hasToFullGen)
            generateNodes();

        generatePaths(hasToFullGen);
        hasToGen = false;
        hasToFullGen = false;
        saveNodeStore();
    }

    BuildZoneIndex();
    PrecomputeReachability();
    InitTaxiGraph();
    LOG_INFO("playerbots", "TravelNodeMap initialized: {} nodes, zone index and reachability built.",
             nodes.size());
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

    //uint32 nr = 0; //not used, line marked for removal.

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

        /*
                out << std::fixed << std::setprecision(2) << "        nodes[" << i << "] =
           TravelNodeMap::instance().addNode(&WorldPosition(" << node->GetMapId() << "," << node->getX() << "f," << node->getY()
           << "f," << node->getZ() << "f,"<< node->getO() <<"f), \""
                    << name << "\", " << (node->isImportant() ? "true" : "false") << ", true";
                if (node->isTransport())
                    out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();

                out << ");";
                */
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

            // out << std::fixed << std::setprecision(1) << "        nodes[" << i << "]->setPathTo(nodes[" <<
            // saveNodes.find(Link.first)->second << "],TravelNodePath("; out << Link.second->print() << "), true);";
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
    {
        LOG_INFO("playerbots", "[SaveNodes] Skipped — hasToSave is false");
        return;
    }

    hasToSave = false;

    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();

    trans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE));
    trans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE_LINK));
    trans->Append(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_TRAVELNODE_PATH));

    std::unordered_map<TravelNode*, uint32> saveNodes;
    std::vector<TravelNode*> anodes = TravelNodeMap::instance().getNodes();

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
        trans->Append(stmt);

        saveNodes.insert(std::make_pair(node, i));
    }

    LOG_INFO("playerbots", ">> Saved {} travelNodes.", anodes.size());

    uint32 paths = 0;
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
            trans->Append(stmt);

            paths++;
        }
    }
    LOG_INFO("playerbots", ">> Saved {} travelNode links.", paths);

    // Path points: bulk raw SQL multi-row INSERTs (~500 rows each) instead of
    // 1M+ individual prepared statements.  Appended to the same transaction so
    // ordering is guaranteed.
    constexpr uint32 BATCH_SIZE = 500;
    uint32 points = 0;
    std::ostringstream ss;
    uint32 batchCount = 0;

    auto flushBatch = [&]()
    {
        if (batchCount == 0)
            return;

        std::string sql = ss.str();
        sql.back() = ';';  // Replace trailing comma
        trans->Append(sql.c_str());
        ss.str("");
        ss.clear();
        batchCount = 0;
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
                    flushBatch();
            }
        }
    }

    flushBatch();

    LOG_INFO("playerbots", ">> Saved {} travelNode path points.", points);

    PlayerbotsDatabase.CommitTransaction(trans);
}

void TravelNodeMap::LoadNodeStore()
{
    std::string const query = "SELECT id, name, map_id, x, y, z, linked FROM playerbots_travelnode";

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
                                           fields[1].Get<std::string>(), true);

                if (fields[6].Get<bool>())
                    node->setLinked(true);
                else
                {
                    hasToGen = true;
                    LOG_INFO("playerbots", "[LoadNodes] Node '{}' (id={}) is unlinked — will trigger generation",
                             fields[1].Get<std::string>(), fields[0].Get<uint32>());
                }

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
                {
                    hasToGen = true;
                    LOG_DEBUG("playerbots", "[LoadNodes] Link {}->{} not calculated — will trigger generation",
                              fields[0].Get<uint32>(), fields[1].Get<uint32>());
                }

            } while (result->NextRow());

            LOG_INFO("playerbots", ">> Loaded {} travelNode links.", result->GetRowCount());
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

    for (auto& node : nodes)
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
        for (auto& node : nodes)
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

    if (!startNode || !endNode || startNode->map_id != endNode->map_id)
        return {};

    auto cacheItr = m_taxiPathCache.find(fromNode);
    if (cacheItr == m_taxiPathCache.end())
        return {};

    auto toNodeItr = cacheItr->second.find(toNode);
    if (toNodeItr == cacheItr->second.end())
        return {};

    return toNodeItr->second;
}

void TravelNodeMap::BuildTaxiGraph()
{
    m_taxiGraph.clear();
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
        m_taxiGraph[node] = std::vector<uint32>(neighbors.begin(), neighbors.end());
}

void TravelNodeMap::ComputeAllPaths()
{
    std::set<uint32> allNodes;
    for (auto const& [source, neighbors] : m_taxiGraph)
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
                m_taxiPathCache[source][target] = path;
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

        for (uint32 next : m_taxiGraph.at(current))
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
                                              const std::unordered_map<uint32, uint32>& parentMap)
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

void TravelNodeMap::BuildZoneIndex()
{
    m_zoneIndex.clear();
    m_mapIndex.clear();

    for (auto* node : nodes)
    {
        if (!node)
            continue;

        WorldPosition* pos = node->getPosition();
        uint32 mapId = pos->GetMapId();

        m_mapIndex[mapId].push_back(node);

        uint32 zoneId = sMapMgr->GetZoneId(PHASEMASK_NORMAL, *pos);
        if (zoneId)
            m_zoneIndex[zoneId].push_back(node);
    }

    LOG_INFO("playerbots", "[ZoneIndex] Built index: {} zones, {} maps", m_zoneIndex.size(), m_mapIndex.size());
}

TravelNode* TravelNodeMap::GetNearestNodeInZone(WorldPosition pos, uint32 zoneId)
{
    auto it = m_zoneIndex.find(zoneId);
    if (it == m_zoneIndex.end() || it->second.empty())
        return GetNearestNodeOnMap(pos);  // Fallback to map-wide

    TravelNode* bestNode = nullptr;
    float bestDist = FLT_MAX;

    for (auto* node : it->second)
    {
        if (!node || node->GetMapId() != pos.GetMapId())
            continue;
        float dist = node->fDist(pos);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestNode = node;
        }
    }

    if (!bestNode)
        return GetNearestNodeOnMap(pos);

    return bestNode;
}

TravelNode* TravelNodeMap::GetNearestNodeOnMap(WorldPosition pos)
{
    auto it = m_mapIndex.find(pos.GetMapId());
    if (it == m_mapIndex.end() || it->second.empty())
        return nullptr;

    TravelNode* bestNode = nullptr;
    float bestDist = FLT_MAX;

    for (auto* node : it->second)
    {
        if (!node)
            continue;
        float d = node->fDist(pos);
        if (d < bestDist)
        {
            bestDist = d;
            bestNode = node;
        }
    }

    return bestNode;
}

void TravelNodeMap::PrecomputeReachability()
{
    // Find connected components via BFS
    std::unordered_set<TravelNode*> visited;
    std::vector<std::vector<TravelNode*>> components;

    for (auto* node : nodes)
    {
        if (!node || visited.count(node))
            continue;

        // BFS from this node
        std::vector<TravelNode*> component;
        std::queue<TravelNode*> q;
        q.push(node);
        visited.insert(node);

        while (!q.empty())
        {
            TravelNode* current = q.front();
            q.pop();
            component.push_back(current);

            for (auto const& link : *current->getLinks())
            {
                TravelNode* neighbor = link.first;
                if (neighbor && !visited.count(neighbor))
                {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }

        components.push_back(std::move(component));
    }

    // Populate routes: every node in a component can reach every other node
    // in the same component
    for (auto const& comp : components)
    {
        for (auto* node : comp)
        {
            node->clearRoutes();
            for (auto* other : comp)
                node->setRouteTo(other);
        }
    }

    uint32 totalNodes = 0;
    for (auto const& c : components)
        totalNodes += c.size();

    // Sort components by size descending for logging
    std::sort(components.begin(), components.end(),
              [](auto const& a, auto const& b) { return a.size() > b.size(); });

    uint32 singletons = 0;
    for (auto const& c : components)
    {
        if (c.size() == 1)
            singletons++;
    }

    LOG_INFO("playerbots", "[Reachability] {} nodes in {} connected components ({} singletons)",
             totalNodes, components.size(), singletons);

    // Log top 10 largest components
    for (uint32 i = 0; i < std::min<uint32>(10, components.size()); ++i)
    {
        auto const& c = components[i];
        std::string sampleName = c.front() ? c.front()->getName() : "?";
        uint32 mapId = c.front() ? c.front()->GetMapId() : 0;
        LOG_INFO("playerbots", "  Component {}: {} nodes, map={}, sample='{}'", i, c.size(), mapId, sampleName);
    }
    if (singletons > 0)
    {
        LOG_INFO("playerbots", "  Singleton nodes (no links):");
        uint32 logged = 0;
        for (auto const& c : components)
        {
            if (c.size() == 1 && logged < 20)
            {
                auto* n = c.front();
                LOG_INFO("playerbots", "    '{}' map={} pos=({:.0f},{:.0f},{:.0f})", n->getName(), n->GetMapId(),
                         n->getPosition()->GetPositionX(), n->getPosition()->GetPositionY(), n->getPosition()->GetPositionZ());
                logged++;
            }
        }
        if (singletons > 20)
            LOG_INFO("playerbots", "    ... and {} more singletons", singletons - 20);
    }
}
