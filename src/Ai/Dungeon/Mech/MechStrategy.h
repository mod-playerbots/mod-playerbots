#ifndef PLAYERBOTS_MECHSTRATEGY_H
#define PLAYERBOTS_MECHSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

class TbcDungeonMechanarStrategy : public Strategy
{
public:
    TbcDungeonMechanarStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tbc-mech"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
