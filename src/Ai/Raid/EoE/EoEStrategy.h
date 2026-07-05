#ifndef PLAYERBOTS_EOESTRATEGY_H
#define PLAYERBOTS_EOESTRATEGY_H

#include "Strategy.h"

class RaidEoEStrategy : public Strategy
{
public:
    RaidEoEStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "wotlk-eoe"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
