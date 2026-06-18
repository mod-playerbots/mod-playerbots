
#ifndef _PLAYERBOT_RAIDBWLSTRATEGY_H
#define _PLAYERBOT_RAIDBWLSTRATEGY_H

#include "Strategy.h"

class RaidBwlStrategy : public Strategy
{
public:
    RaidBwlStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "bwl"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
