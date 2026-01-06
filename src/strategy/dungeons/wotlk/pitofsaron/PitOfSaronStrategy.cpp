#include "PitOfSaronStrategy.h"
#include "PitOfSaronMultipliers.h"

void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ick and krick",
        { NextAction("ick and krick", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("tyrannus",
        { NextAction("tyrannus", ACTION_RAID + 5) }));
}

void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckAndKrickMultiplier(botAI));
}
