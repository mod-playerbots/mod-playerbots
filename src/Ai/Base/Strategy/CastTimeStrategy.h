/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CASTTIMESTRATEGY_H
#define PLAYERBOTS_CASTTIMESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class CastTimeMultiplier : public Multiplier
{
public:
    CastTimeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cast time") {}

    float GetValue(Action* action) override;
};

class CastTimeStrategy : public Strategy
{
public:
    CastTimeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "cast time"; }
};

#endif
