
// RaidOnyxiaStrategy.h
#ifndef PLAYERBOTS_ONYSTRATEGY_H
#define PLAYERBOTS_ONYSTRATEGY_H

#include "Strategy.h"

class RaidOnyxiaStrategy : public Strategy
{
public:
    RaidOnyxiaStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    std::string const getName() override { return "onyxia"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
