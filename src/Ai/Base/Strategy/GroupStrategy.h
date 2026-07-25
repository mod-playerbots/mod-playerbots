/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GROUPSTRATEGY_H
#define PLAYERBOTS_GROUPSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GroupStrategy : public NonCombatStrategy
{
public:
    GroupStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "group"; }
    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
