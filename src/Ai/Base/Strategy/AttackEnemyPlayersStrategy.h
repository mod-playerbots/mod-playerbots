/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ATTACKENEMYPLAYERSSTRATEGY_H
#define PLAYERBOTS_ATTACKENEMYPLAYERSSTRATEGY_H

#include "NonCombatStrategy.h"

class AttackEnemyPlayersStrategy : public NonCombatStrategy
{
public:
    AttackEnemyPlayersStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "pvp"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
