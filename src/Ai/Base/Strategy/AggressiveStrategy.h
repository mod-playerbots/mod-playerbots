/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AGGRESSIVESTRATEGY_H
#define PLAYERBOTS_AGGRESSIVESTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class AggressiveStrategy : public NonCombatStrategy
{
public:
    AggressiveStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "aggressive"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
