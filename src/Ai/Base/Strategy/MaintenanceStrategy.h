/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_MAINTENANCESTRATEGY_H
#define PLAYERBOTS_MAINTENANCESTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class MaintenanceStrategy : public NonCombatStrategy
{
public:
    MaintenanceStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "maintenance"; }
    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
