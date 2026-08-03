/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICHUNTERNONCOMBATSTRATEGY_H
#define PLAYERBOTS_GENERICHUNTERNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericHunterNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericHunterNonCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class HunterPetStrategy : public Strategy
{
public:
    HunterPetStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "pet"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
