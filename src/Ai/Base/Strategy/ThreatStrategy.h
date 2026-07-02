/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_THREATSTRATEGY_H
#define PLAYERBOTS_THREATSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class ThreatMultiplier : public Multiplier
{
public:
    ThreatMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "threat") {}

    float GetValue(Action* action) override;
};

class ThreatStrategy : public Strategy
{
public:
    ThreatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "threat"; }
};

class FocusMultiplier : public Multiplier
{
public:
    FocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "focus") {}

    float GetValue(Action* action) override;
};

class FocusStrategy : public Strategy
{
public:
    FocusStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "focus"; }
};

#endif
