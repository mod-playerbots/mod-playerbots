/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
