/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DPSASSISTSTRATEGY_H
#define PLAYERBOTS_DPSASSISTSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class DpsAssistStrategy : public NonCombatStrategy
{
public:
    DpsAssistStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "dps assist"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class DpsAoeStrategy : public NonCombatStrategy
{
public:
    DpsAoeStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "dps aoe"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
