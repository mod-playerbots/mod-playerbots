#ifndef PLAYERBOTS_HOSSTRATEGY_H
#define PLAYERBOTS_HOSSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class WotlkDungeonHoSStrategy : public Strategy
{
public:
    WotlkDungeonHoSStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "halls of stone"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
