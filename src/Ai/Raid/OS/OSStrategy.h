/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OSSTRATEGY_H
#define _PLAYERBOT_OSSTRATEGY_H

#include "Strategy.h"

class RaidOsStrategy : public Strategy
{
public:
    RaidOsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "wotlk-os"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
