
#ifndef PLAYERBOTS_ULDSTRATEGY_H
#define PLAYERBOTS_ULDSTRATEGY_H

#include "Strategy.h"

class RaidUlduarStrategy : public Strategy
{
public:
    RaidUlduarStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "ulduar"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
