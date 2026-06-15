#ifndef PLAYERBOTS_FOSSTRATEGY_H
#define PLAYERBOTS_FOSSTRATEGY_H
#include "Multiplier.h"
#include "Strategy.h"

class WotlkDungeonFoSStrategy : public Strategy
{
public:
    WotlkDungeonFoSStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "forge of souls"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;

};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSSTRATEGY_H
