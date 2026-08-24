/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACSTRATEGY_H
#define PLAYERBOTS_ACSTRATEGY_H

#include "Multiplier.h"
#include "Strategy.h"

class TbcDungeonAuchenaiCryptsStrategy : public Strategy
{
public:
    TbcDungeonAuchenaiCryptsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-ac"; }

    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
