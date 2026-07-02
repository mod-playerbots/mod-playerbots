/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_USEFOODSTRATEGY_H
#define PLAYERBOTS_USEFOODSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class UseFoodStrategy : public Strategy
{
public:
    UseFoodStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "food"; }
};

#endif
