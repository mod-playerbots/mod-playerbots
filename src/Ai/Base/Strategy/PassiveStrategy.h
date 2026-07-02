/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PASSIVESTRATEGY_H
#define PLAYERBOTS_PASSIVESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class PassiveStrategy : public Strategy
{
public:
    PassiveStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "passive"; }
};

#endif
