#ifndef PLAYERBOTS_COSSTRATEGY_H
#define PLAYERBOTS_COSSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonCoSStrategy : public Strategy
{
public:
    WotlkDungeonCoSStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "culling of stratholme"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
