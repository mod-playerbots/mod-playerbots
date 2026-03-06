#ifndef _PLAYERBOT_RAIDTEMPESTKEEPSTRATEGY_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPSTRATEGY_H_

#include "Strategy.h"
#include "Multiplier.h"

class RaidTempestKeepStrategy : public Strategy
{
public:
    RaidTempestKeepStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tempestkeep"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
