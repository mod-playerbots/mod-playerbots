#ifndef PLAYERBOTS_AQ20STRATEGY_H
#define PLAYERBOTS_AQ20STRATEGY_H

#include "Strategy.h"

class RaidAq20Strategy : public Strategy
{
public:
    RaidAq20Strategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "aq20"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
