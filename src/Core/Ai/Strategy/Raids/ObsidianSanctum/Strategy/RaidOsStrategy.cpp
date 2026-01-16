#include "RaidOsStrategy.h"
#include "RaidOsMultipliers.h"
#include "Strategy.h"

void RaidOsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("sartharion tank",
                        { NextAction("sartharion tank position", ACTION_MOVE) }));
    triggers.push_back(
        new TriggerNode("twilight fissure",
                        { NextAction("avoid twilight fissure", ACTION_RAID + 2) }));
    triggers.push_back(
        new TriggerNode("flame tsunami",
                        { NextAction("avoid flame tsunami", ACTION_RAID + 1) }));
    triggers.push_back(
        new TriggerNode("sartharion dps",
                        { NextAction("sartharion attack priority", ACTION_RAID) }));
    // Flank dragon positioning
    triggers.push_back(new TriggerNode("sartharion melee positioning",
        { NextAction("rear flank", ACTION_MOVE + 4) }));

    triggers.push_back(new TriggerNode("twilight portal enter",
        { NextAction("enter twilight portal", ACTION_RAID + 1) }));
    triggers.push_back(new TriggerNode("twilight portal exit",
        { NextAction("exit twilight portal", ACTION_RAID + 1) }));
}

void RaidOsStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SartharionMultiplier(botAI));
}
