#ifndef PLAYERBOTS_KARASTRATEGY_H
#define PLAYERBOTS_KARASTRATEGY_H

#include "Strategy.h"

class RaidKarazhanStrategy : public Strategy
{
public:
    RaidKarazhanStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "karazhan"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
