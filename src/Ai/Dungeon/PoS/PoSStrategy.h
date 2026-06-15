#ifndef _PLAYERBOT_POSSTRATEGY_H
#define _PLAYERBOT_POSSTRATEGY_H
#include "Multiplier.h"
#include "Strategy.h"

class WotlkDungeonPoSStrategy : public Strategy
{
public:
    WotlkDungeonPoSStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "pit of saron"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;

};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSSTRATEGY_H
