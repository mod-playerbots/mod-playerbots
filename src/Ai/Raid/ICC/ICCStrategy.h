#ifndef PLAYERBOTS_ICCSTRATEGY_H
#define PLAYERBOTS_ICCSTRATEGY_H

#include "Strategy.h"

class RaidIccStrategy : public Strategy
{
public:
    RaidIccStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "icc"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
