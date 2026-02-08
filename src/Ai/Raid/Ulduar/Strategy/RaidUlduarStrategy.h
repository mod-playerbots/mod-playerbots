
#ifndef _PLAYERBOT_RAIDULDUARSTRATEGY_H
#define _PLAYERBOT_RAIDULDUARSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"

class RaidUlduarStrategy : public Strategy
{
public:
    RaidUlduarStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "ulduar"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
