#ifndef PLAYERBOTS_TOCSTRATEGY_H
#define PLAYERBOTS_TOCSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"
#include "TOCMultipliers.h"

class WotlkDungeonToCStrategy : public Strategy
{
public:
    WotlkDungeonToCStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "trial of the champion"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
