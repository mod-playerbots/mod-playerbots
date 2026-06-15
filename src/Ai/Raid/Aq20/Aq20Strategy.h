/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AQ20STRATEGY_H
#define _PLAYERBOT_AQ20STRATEGY_H

#include "Strategy.h"

class RaidAq20Strategy : public Strategy
{
public:
    RaidAq20Strategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "aq20"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
