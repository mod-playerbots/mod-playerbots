#include "RaidEoEStrategy.h"
#include "RaidEoEMultipliers.h"
#include "Strategy.h"

void RaidEoEStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("malygos",
        { NextAction("malygos position", ACTION_MOVE) }));
    triggers.push_back(new TriggerNode("malygos",
        { NextAction("malygos target", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("group flying",
        { NextAction("eoe fly drake", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("drake combat",
        { NextAction("eoe drake attack", ACTION_NORMAL + 5) }));
}

void RaidEoEStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new MalygosMultiplier(botAI));
}
