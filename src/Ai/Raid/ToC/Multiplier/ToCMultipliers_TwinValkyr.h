/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_TWINVALKYR_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_TWINVALKYR_H

#include <vector>

#include "Multiplier.h"
#include "RaidBurstWindow.h"
#include "RaidCombatHelpers.h"

// Keeps a twin's tank on her instead of drifting with the combat formation
class TwinValkyrControlTankMovementMultiplier : public Multiplier
{
public:
    TwinValkyrControlTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "twin valkyr control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

// No taunting the twin another living tank holds
class TwinValkyrTauntGuardMultiplier : public Multiplier
{
public:
    TwinValkyrTauntGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "twin valkyr taunt guard multiplier") {}
    float GetValue(Action* action) override;
};

enum class TwinInterruptKind : uint8
{
    None,
    Kick,
    Refused
};

// Holds kicks while a shield makes the twin immune to them, and silences and stuns, which never land
// on the twins
class TwinValkyrInterruptHoldMultiplier : public Multiplier
{
public:
    TwinValkyrInterruptHoldMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "twin valkyr interrupt hold multiplier") {}
    float GetValue(Action* action) override;

private:
    // Read off the action's name, which never changes
    RaidActionCache<TwinInterruptKind> _kinds;
};

// The class redirects pick one tank for the whole raid, and the opener's fires while the twins
// are still flying in. Here each twin has her own tank, and the encounter's redirect node casts them.
class TwinValkyrRedirectGuardMultiplier : public Multiplier
{
public:
    TwinValkyrRedirectGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "twin valkyr redirect guard multiplier") {}
    float GetValue(Action* action) override;

private:
    // Whether the action is one of the class redirects, fixed by its type
    RaidActionCache<bool> _redirects;
};

void AddToCTwinValkyrMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

RaidBurstWindow ToCTwinValkyrBurstWindow(PlayerbotAI* botAI);

#endif
