#ifndef PLAYERBOTS_AKSTRATEGY_H
#define PLAYERBOTS_AKSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class WotlkDungeonOKStrategy : public Strategy
{
public:
    WotlkDungeonOKStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "old kingdom"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
