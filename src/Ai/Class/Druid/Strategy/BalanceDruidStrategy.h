/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BALANCEDRUIDSTRATEGY_H
#define _PLAYERBOT_BALANCEDRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"

class PlayerbotAI;

class BalanceDruidStrategy : public GenericDruidStrategy
{
public:
    BalanceDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "balance"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

#endif
