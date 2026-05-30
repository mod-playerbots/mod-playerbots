#include "BWLStrategy.h"

void RaidBwlStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", {
        NextAction("bwl check onyxia scale cloak", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl suppression device", {
        NextAction("bwl turn off suppression device", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl affliction bronze", {
        NextAction("bwl use hourglass sand", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl wild magic", {
        NextAction("ice block", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl nefarian fear ward", {
        NextAction("bwl nefarian fear ward", ACTION_RAID) }));
}
