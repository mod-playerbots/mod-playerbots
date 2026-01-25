#include "TaxiPathFinder.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include "Log.h"
#include <chrono>

void TaxiPathFinder::BuildTaxiGraph()
{
    taxiForward.clear();
    taxiReverse.clear();

    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* path = sTaxiPathStore.LookupEntry(i);
        if (!path)
            continue;

        if (path->to == 0 || path->to == uint32(-1))
            continue;
        taxiForward[path->from].push_back(path->to);
        taxiReverse[path->to].push_back(path->from);
    }
    LOG_INFO("playerbots", "Playerbots Taxi graph built successfully.");
}

std::vector<uint32> TaxiPathFinder::FindTaxiPath(uint32 fromNode, uint32 toNode)
{
    std::vector<uint32> pathFromStart;

    if (fromNode == toNode)
        return pathFromStart;

    TaxiNodesEntry const* startNode = sTaxiNodesStore.LookupEntry(fromNode);
    TaxiNodesEntry const* endNode = sTaxiNodesStore.LookupEntry(toNode);

    if (!startNode || !endNode || startNode->map_id != endNode->map_id)
        return pathFromStart;

    //Lets check the cache first.
    auto cacheItr = taxiPathCache.find(fromNode);
    if (cacheItr != taxiPathCache.end())
    {
        auto toNodeItr = cacheItr->second.find(toNode);
        if (toNodeItr != cacheItr->second.end())
            return toNodeItr->second;
    }

    std::queue<uint32> workQueue;
    std::map <uint32, uint32> parentMap;

    workQueue.push(fromNode);
    parentMap[fromNode] = 0;

    bool found = false;

    using Clock = std::chrono::high_resolution_clock;
    auto start = Clock::now();
    size_t iterations = 0;

    while (!workQueue.empty())
    {
        ++iterations;
        uint32 current = workQueue.front();
        workQueue.pop();
        if (current == toNode)
        {
            found = true;
            break;
        }
        for (uint32 neighbor : taxiForward[current])
        {
            if (parentMap.find(neighbor) == parentMap.end())
            {
                workQueue.push(neighbor);
                parentMap[neighbor] = current;
            }
        }
    }

    if (found)
    {
        uint32 curr = toNode;
        while (curr != 0)
        {
            pathFromStart.push_back(curr);
            curr = parentMap[curr];
        }
        std::reverse(pathFromStart.begin(), pathFromStart.end());
    }

    return pathFromStart;
}