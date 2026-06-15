/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OCSTRATEGY_H
#define _PLAYERBOT_OCSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonOccStrategy : public Strategy
{
public:
    WotlkDungeonOccStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "oculus"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
