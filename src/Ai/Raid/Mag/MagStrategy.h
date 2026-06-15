/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGSTRATEGY_H
#define _PLAYERBOT_MAGSTRATEGY_H

#include "Strategy.h"

class RaidMagtheridonStrategy : public Strategy
{
public:
    RaidMagtheridonStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "magtheridon"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
