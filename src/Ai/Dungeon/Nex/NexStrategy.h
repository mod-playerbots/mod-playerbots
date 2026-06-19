#ifndef PLAYERBOTS_NEXSTRATEGY_H
#define PLAYERBOTS_NEXSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonNexStrategy : public Strategy
{
public:
    WotlkDungeonNexStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "nexus"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
