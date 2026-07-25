/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICPALADINNONCOMBATSTRATEGY_H
#define PLAYERBOTS_GENERICPALADINNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericPaladinNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericPaladinNonCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
