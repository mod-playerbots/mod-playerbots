#ifndef _PLAYERBOT_RAIDMCSTRATEGY_H
#define _PLAYERBOT_RAIDMCSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class RaidMcStrategy : public Strategy
{
public:
    RaidMcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "moltencore"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
