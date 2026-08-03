/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSSTRATEGY_H
#define PLAYERBOTS_POSSTRATEGY_H
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
