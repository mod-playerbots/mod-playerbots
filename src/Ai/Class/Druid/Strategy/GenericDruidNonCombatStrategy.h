/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICDRUIDNONCOMBATSTRATEGY_H
#define PLAYERBOTS_GENERICDRUIDNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericDruidNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericDruidNonCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class GenericDruidBuffStrategy : public NonCombatStrategy
{
public:
    GenericDruidBuffStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "buff"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
