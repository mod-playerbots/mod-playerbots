#ifndef PLAYERBOTS_MECHANARSTRATEGY_H
#define PLAYERBOTS_MECHANARSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

class TbcDungeonMechanarStrategy : public Strategy
{
public:
    TbcDungeonMechanarStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mechanar"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
