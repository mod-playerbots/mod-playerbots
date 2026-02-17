#ifndef _PLAYERBOT_RAIDSSCSTRATEGY_H_
#define _PLAYERBOT_RAIDSSCSTRATEGY_H_

#include "Strategy.h"
#include "Multiplier.h"

class RaidSSCStrategy : public Strategy
{
public:
    RaidSSCStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "ssc"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
