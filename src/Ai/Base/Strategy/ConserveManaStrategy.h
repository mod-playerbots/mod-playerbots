/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CONSERVEMANASTRATEGY_H
#define PLAYERBOTS_CONSERVEMANASTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class HealerAutoSaveManaMultiplier : public Multiplier
{
public:
    HealerAutoSaveManaMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "save mana") {}

    float GetValue(Action* action) override;
};

class HealerAutoSaveManaStrategy : public Strategy
{
public:
    HealerAutoSaveManaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "save mana"; }
};

#endif
