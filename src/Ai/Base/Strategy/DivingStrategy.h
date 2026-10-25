/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DIVINGSTRATEGY_H
#define PLAYERBOTS_DIVINGSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// Always-on emergency surfacing for bots under water: prevents drowning when air runs low and
// gets idle bots out of deep water before their next path drags them through terrain.
class DivingStrategy : public Strategy
{
public:
    DivingStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "diving"; }
};

#endif
