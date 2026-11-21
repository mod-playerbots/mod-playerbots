/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SELLJUNKSTRATEGY_H
#define PLAYERBOTS_SELLJUNKSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class SellJunkStrategy : public NonCombatStrategy
{
public:
    SellJunkStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "sell junk"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
