#include "TaxiPathFinder.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include "Log.h"

void TaxiPathFinder::Init()
{
    BuildTaxiGraph();
    ComputeAllPaths();
    LOG_INFO("playerbots", "Playerbots Taxi graph built successfully.");
}

void TaxiPathFinder::BuildTaxiGraph()
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

std::unordered_map<uint32, uint32> TaxiPathFinder::BFS(uint32 fromNode)
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

std::vector<uint32> TaxiPathFinder::BuildPath(uint32 fromNode, uint32 toNode,
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

void TaxiPathFinder::ComputeAllPaths()
{
    using Clock = std::chrono::high_resolution_clock;
    auto start = Clock::now();
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
    auto end = Clock::now();
    auto elapsedMs =
     std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    LOG_ERROR("playerbots", "Taxi graph calculation took {} ms", elapsedMs);
}


std::vector<uint32> TaxiPathFinder::FindTaxiPath(uint32 fromNode, uint32 toNode)
{
    std::vector<uint32> path;

    if (fromNode == toNode)
        return path;

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    TaxiNodesEntry const* endNode = sTaxiNodesStore.LookupEntry(toNode);

    if (!startNode || !endNode || startNode->map_id != endNode->map_id)
        return path;

    //Lets check the cache first.
    auto cacheItr = taxiPathCache.find(fromNode);
    if (cacheItr != taxiPathCache.end())
    {
        auto toNodeItr = cacheItr->second.find(toNode);
        if (toNodeItr != cacheItr->second.end())
            return toNodeItr->second;
    }
    return path;
}