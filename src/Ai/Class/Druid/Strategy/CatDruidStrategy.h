/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CATDRUIDSTRATEGY_H
#define _PLAYERBOT_CATDRUIDSTRATEGY_H

#include "FeralDruidStrategy.h"

class PlayerbotAI;

class CatDruidStrategy : public FeralDruidStrategy
{
public:
    CatDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
};

// Optional additive strategy. Layers emergency heals on top of the "cat" strategy.
// Enable : co +offheal
// Disable: co -offheal
class CatOffhealStrategy : public CombatStrategy
{
public:
    CatOffhealStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "offheal"; }
};

#endif
