/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UPSTRATEGY_H
#define _PLAYERBOT_UPSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonUPStrategy : public Strategy
{
public:
    WotlkDungeonUPStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "utgarde pinnacle"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
