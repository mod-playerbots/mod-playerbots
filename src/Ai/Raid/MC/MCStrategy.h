/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MCSTRATEGY_H
#define _PLAYERBOT_MCSTRATEGY_H

#include "Strategy.h"

class RaidMcStrategy : public Strategy
{
public:
    RaidMcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "moltencore"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
