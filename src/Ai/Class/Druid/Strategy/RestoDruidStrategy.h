/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RESTODRUIDSTRATEGY_H
#define _PLAYERBOT_RESTODRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class RestoDruidStrategy : public GenericDruidStrategy
{
public:
    RestoDruidStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "resto"; }
    uint32 GetType() const override { return STRATEGY_TYPE_RANGED | STRATEGY_TYPE_HEAL; }
};

class DruidBlanketStrategy : public Strategy
{
public:
    DruidBlanketStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "blanketing"; }
};

class DruidTranquilityStrategy : public Strategy
{
public:
    DruidTranquilityStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tranquility"; }
};

#endif
