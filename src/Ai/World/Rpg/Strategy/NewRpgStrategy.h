/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGSTRATEGY_H
#define PLAYERBOTS_NEWRPGSTRATEGY_H

#include "Strategy.h"
#include "TravelMgr.h"
#include "NewRpgInfo.h"

class PlayerbotAI;

class NewRpgStrategy : public Strategy
{
public:
    NewRpgStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "new rpg"; }
    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
