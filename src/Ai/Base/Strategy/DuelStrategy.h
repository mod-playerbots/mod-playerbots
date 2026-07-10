/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUELSTRATEGY_H
#define PLAYERBOTS_DUELSTRATEGY_H

#include "PassThroughStrategy.h"

class PlayerbotAI;

class DuelStrategy : public PassThroughStrategy
{
public:
    DuelStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "duel"; }
};

class StartDuelStrategy : public Strategy
{
public:
    StartDuelStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "start duel"; }
};

#endif
