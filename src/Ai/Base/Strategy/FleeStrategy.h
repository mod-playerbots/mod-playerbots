/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FLEESTRATEGY_H
#define PLAYERBOTS_FLEESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class FleeStrategy : public Strategy
{
public:
    FleeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "flee"; };
};

class FleeFromAddsStrategy : public Strategy
{
public:
    FleeFromAddsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "flee from adds"; };
};

#endif
