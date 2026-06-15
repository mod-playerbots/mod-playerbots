/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SSCSTRATEGY_H
#define PLAYERBOTS_SSCSTRATEGY_H

#include "Strategy.h"

class RaidSSCStrategy : public Strategy
{
public:
    RaidSSCStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "ssc"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
