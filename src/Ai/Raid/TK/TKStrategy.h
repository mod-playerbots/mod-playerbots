#ifndef PLAYERBOTS_TKSTRATEGY_H
#define PLAYERBOTS_TKSTRATEGY_H

#include "Strategy.h"

class RaidTempestKeepStrategy : public Strategy
{
public:
    RaidTempestKeepStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tempestkeep"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
