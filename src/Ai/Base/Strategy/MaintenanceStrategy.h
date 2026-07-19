/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
