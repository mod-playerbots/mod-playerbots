/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BWLSTRATEGY_H
#define _PLAYERBOT_BWLSTRATEGY_H

#include "Strategy.h"

class RaidBwlStrategy : public Strategy
{
public:
    RaidBwlStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "bwl"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
