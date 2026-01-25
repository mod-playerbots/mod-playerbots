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
    void Init() { BuildTaxiGraph(); }

    std::vector<uint32> FindTaxiPath(uint32 fromNode, uint32 toNode);

private:
    std::map<uint32, std::map<uint32, std::vector<uint32>>> taxiPathCache;
    std::unordered_map<uint32, std::vector<uint32>> taxiForward;
    std::unordered_map<uint32, std::vector<uint32>> taxiReverse;

    void BuildTaxiGraph();

};

#define sTaxiPathFinder TaxiPathFinder::Instance()
#endif
