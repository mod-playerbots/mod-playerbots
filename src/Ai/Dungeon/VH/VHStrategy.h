#ifndef PLAYERBOTS_VHSTRATEGY_H
#define PLAYERBOTS_VHSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonVHStrategy : public Strategy
{
public:
    WotlkDungeonVHStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "violet hold"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
