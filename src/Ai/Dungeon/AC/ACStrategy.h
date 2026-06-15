/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACSTRATEGY_H
#define _PLAYERBOT_ACSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

class TbcDungeonAuchenaiCryptsStrategy : public Strategy
{
public:
    TbcDungeonAuchenaiCryptsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-ac"; }

    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
