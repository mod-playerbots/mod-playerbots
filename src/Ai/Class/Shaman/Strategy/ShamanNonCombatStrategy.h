/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHAMANNONCOMBATSTRATEGY_H
#define PLAYERBOTS_SHAMANNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class ShamanNonCombatStrategy : public NonCombatStrategy
{
public:
    ShamanNonCombatStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "nc"; }
};

#endif
