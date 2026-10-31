/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WARLOCKVALUES_H
#define PLAYERBOTS_WARLOCKVALUES_H

#include "Value.h"

class Player;
class PlayerbotAI;

// Highest Ritual of Souls rank the warlock can cast right now, 0 when none.
uint32 ReadyRitualOfSoulsRank(Player* warlock);

// The bot is the group's chosen ritual caster and enough nearby members lack a healthstone.
class ShouldCastRitualOfSoulsValue : public BoolCalculatedValue
{
public:
    ShouldCastRitualOfSoulsValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should cast ritual of souls", 5) {}

    bool Calculate() override;
};

#endif
