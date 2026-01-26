#ifndef _PLAYERBOT_TAXIPATHFINDER_H
#define _PLAYERBOT_TAXIPATHFINDER_H

#include "DBCStores.h"

class TaxiPathFinder
{
public:
    static TaxiPathFinder* Instance()
    {
        static TaxiPathFinder instance;
        return &instance;
    }
    void Init();

    std::vector<uint32> FindTaxiPath(uint32 fromNode, uint32 toNode);

private:
    std::map<uint32, std::map<uint32, std::vector<uint32>>> taxiPathCache;
    std::unordered_map<uint32, std::vector<uint32>> taxiGraph;

    void BuildTaxiGraph();
    std::unordered_map<uint32, uint32> BFS(uint32 start);
    void ComputeAllPaths();
    std::vector<uint32> BuildPath(uint32 from, uint32 to,
                              const std::unordered_map<uint32, uint32>& parent);

};

#define sTaxiPathFinder TaxiPathFinder::Instance()
#endif
