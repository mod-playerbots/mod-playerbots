/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCSTRATEGY_H
#define PLAYERBOTS_RAIDTOCSTRATEGY_H

#include "Strategy.h"

// Trial of the Crusader, all four modes (10/25 man, normal/heroic share map 649).
// Covers the Faction Champions kiting, the Twin Val'kyr portal dance, the
// off-heal switch while a healer is busy, and the Anub'arak Starfall lockout.
class RaidTocStrategy : public Strategy
{
public:
    RaidTocStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "toc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
