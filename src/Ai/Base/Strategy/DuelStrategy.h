/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
