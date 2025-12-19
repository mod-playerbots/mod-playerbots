#include "ForgeOfSoulsStrategy.h"
#include "ForgeOfSoulsMultipliers.h"

void WotlkDungeonFoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("move from bronjahm",
        { NextAction("move from bronjahm", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("switch to soul fragment",
        { NextAction("attack corrupted soul fragment", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("bronjahm position",
        { NextAction("bronjahm group position", ACTION_RAID + 1) }));
    triggers.push_back(new TriggerNode("devourer of souls",
        { NextAction("devourer of souls", ACTION_RAID + 1) }));
}

void WotlkDungeonFoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BronjahmMultiplier(botAI));
}
