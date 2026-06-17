#ifndef PLAYERBOTS_GRUULSTRATEGY_H
#define PLAYERBOTS_GRUULSTRATEGY_H

#include "Strategy.h"

class RaidGruulsLairStrategy : public Strategy
{
public:
    RaidGruulsLairStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "gruulslair"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
