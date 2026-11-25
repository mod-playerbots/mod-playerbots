/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_GORMOK_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_GORMOK_H

#include "Multiplier.h"
#include "RaidCombatHelpers.h"

// Holds generic movers off a ranged dps or healer carrying a snobold, so it stays in Gormok's melee
// until the snobold dies
class GormokSnoboldCarrierMultiplier : public Multiplier
{
public:
    GormokSnoboldCarrierMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gormok snobold carrier") {}
    float GetValue(Action* action) override;

private:
    struct Facts
    {
        bool spellMover = false;
        bool mover = false;
        bool staysFree = false;
    };

    RaidActionCache<Facts> _facts;
};

#endif
