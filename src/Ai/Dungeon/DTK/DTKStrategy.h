/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKSTRATEGY_H
#define PLAYERBOTS_DTKSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class WotlkDungeonDTKStrategy : public Strategy
{
public:
    WotlkDungeonDTKStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "drak'tharon keep"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
