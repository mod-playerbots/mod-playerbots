/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RETURNSTRATEGY_H
#define PLAYERBOTS_RETURNSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class ReturnStrategy : public NonCombatStrategy
{
public:
    ReturnStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "return"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
