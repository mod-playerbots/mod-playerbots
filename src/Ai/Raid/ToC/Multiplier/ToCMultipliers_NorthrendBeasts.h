/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_NORTHRENDBEASTS_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_NORTHRENDBEASTS_H

#include <vector>

#include "Multiplier.h"
#include "RaidBurstWindow.h"

// Keep beast tanks anchored instead of drifting with the combat formation
class NorthrendBeastsControlTankMovementMultiplier : public Multiplier
{
public:
    NorthrendBeastsControlTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "northrend beasts control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

// Class taunts on a beast only for its duty holder. Beasts obey taunt diminishing returns, and a
// generic taunt from the other tank undoes the deal.
class NorthrendBeastsTauntGuardMultiplier : public Multiplier
{
public:
    NorthrendBeastsTauntGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "northrend beasts taunt guard") {}
    float GetValue(Action* action) override;
};

void AddToCNorthrendBeastsMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

RaidBurstWindow ToCNorthrendBeastsBurstWindow(PlayerbotAI* botAI);

#endif
