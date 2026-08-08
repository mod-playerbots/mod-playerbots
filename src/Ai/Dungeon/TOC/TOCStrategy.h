/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCSTRATEGY_H
#define PLAYERBOTS_TOCSTRATEGY_H

#include "Multiplier.h"
#include "Strategy.h"
#include "TOCMultipliers.h"

class WotlkDungeonToCStrategy : public Strategy
{
public:
    WotlkDungeonToCStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "trial of the champion"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
