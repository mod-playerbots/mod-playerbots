#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSSTRATEGY_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

class TbcDungeonHellfireRampartsStrategy : public Strategy
{
public:
    TbcDungeonHellfireRampartsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-hr"; }

    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
