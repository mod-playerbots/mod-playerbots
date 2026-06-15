#ifndef PLAYERBOTS_ACSTRATEGY_H
#define PLAYERBOTS_ACSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

class TbcDungeonAuchenaiCryptsStrategy : public Strategy
{
public:
    TbcDungeonAuchenaiCryptsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-ac"; }

    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
