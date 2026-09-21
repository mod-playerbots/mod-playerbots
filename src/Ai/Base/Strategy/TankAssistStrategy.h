/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TANKASSISTSTRATEGY_H
#define PLAYERBOTS_TANKASSISTSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class TankAssistStrategy : public Strategy
{
public:
    TankAssistStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tank assist"; }
    uint32 GetType() const override { return STRATEGY_TYPE_TANK; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
