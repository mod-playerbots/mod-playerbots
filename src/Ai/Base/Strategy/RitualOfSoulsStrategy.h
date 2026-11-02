/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RITUALOFSOULSSTRATEGY_H
#define PLAYERBOTS_RITUALOFSOULSSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

// Joins a group member's ritual and takes a healthstone from the soulwell. Warlocks get
// WarlockRitualOfSoulsStrategy under the same name instead, which also casts it.
class RitualOfSoulsStrategy : public NonCombatStrategy
{
public:
    RitualOfSoulsStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "ritualofsouls"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
