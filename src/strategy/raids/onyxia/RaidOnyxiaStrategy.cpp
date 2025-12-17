#include "RaidOnyxiaStrategy.h"

void RaidOnyxiaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ----------- Phase 1 (100% - 65%) -----------

    triggers.push_back(new TriggerNode(
        "ony near tail", { new NextAction("ony move to side", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "ony avoid eggs", { new NextAction("ony avoid eggs move", ACTION_EMERGENCY + 5) }));

    // ----------- Phase 2 (65% - 40%) -----------

    triggers.push_back(
        new TriggerNode("ony deep breath warning",
                        { new NextAction("ony move to safe zone", ACTION_EMERGENCY + 5) }));

    triggers.push_back(
        new TriggerNode("ony fireball splash incoming",
                        { new NextAction("ony spread out", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode(
        "ony whelps spawn", { new NextAction("ony kill whelps", ACTION_RAID + 1) }));
}

void RaidOnyxiaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Empty for now
}
