/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_RANGEDCOMBATSTRATEGY_H
#define PLAYERBOTS_RANGEDCOMBATSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class RangedCombatStrategy : public CombatStrategy
{
public:
    RangedCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_RANGED; }
    std::string const getName() override { return "ranged"; }
};

#endif
