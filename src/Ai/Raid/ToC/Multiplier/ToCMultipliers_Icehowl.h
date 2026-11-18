/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_ICEHOWL_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_ICEHOWL_H

#include "Multiplier.h"
#include "RaidCombatHelpers.h"

// From the gaze to the end of the charge only attacks and the dodge move anyone. Anything else can walk
// a bot into the line, like melee chasing him to where the jump back lands.
class IcehowlChargeGuardMultiplier : public Multiplier
{
public:
    IcehowlChargeGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icehowl charge guard") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _competes;
};

// Keeps the combat formation off a bot holding an Arctic Breath bearing: each breath turns his facing for
// 5 s, and `set behind` would follow it off the bearing.
class IcehowlBreathSpreadGuardMultiplier : public Multiplier
{
public:
    IcehowlBreathSpreadGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icehowl breath spread guard") {}
    float GetValue(Action* action) override;
};

#endif
