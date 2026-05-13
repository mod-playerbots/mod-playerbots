
#ifndef _PLAYERBOT_RAIDVOASTRATEGY_H
#define _PLAYERBOT_RAIDVOASTRATEGY_H

#include "Strategy.h"

class RaidVoAStrategy : public Strategy
{
public:
    RaidVoAStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "voa"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
