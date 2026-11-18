/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_ANUBARAK_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_ANUBARAK_H

#include <unordered_map>
#include <vector>

#include "Multiplier.h"
#include "RaidBurstWindow.h"
#include "RaidCombatHelpers.h"
#include "RaidSplash.h"

// Keep tanks anchored on Anub'arak and his burrowers instead of drifting with the combat formation
class AnubarakControlTankMovementMultiplier : public Multiplier
{
public:
    AnubarakControlTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anubarak control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

// The spike's target moves only through the kite, and never feigns death. Attack actions stay so it
// keeps a target.
class AnubarakProtectSpikeKiteMultiplier : public Multiplier
{
public:
    AnubarakProtectSpikeKiteMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anubarak protect spike kite multiplier") {}
    float GetValue(Action* action) override;

private:
    // Fixed by the action's type and name, so worked out once per action
    static bool HeldWhileMarked(Action* action);

    RaidActionCache<bool> _held;
};

// A DPS bot's damage on him, and its area damage within 30 yd of him, while its threat cap latch holds.
// Heals and anything aimed elsewhere go through. Wired from the definition.
class AnubarakThreatCapMultiplier : public Multiplier
{
public:
    AnubarakThreatCapMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak threat cap") {}
    float GetValue(Action* action) override;

private:
    RaidSplashCache _splash;
};

// Only direct damage on the bot's own duty sphere and nothing else at a Frost Sphere. Heroic: no Starfall at
// all. Untargeted area spells stay unguarded. Wired from the definition.
class AnubarakSphereGuardMultiplier : public Multiplier
{
public:
    AnubarakSphereGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak sphere guard") {}
    float GetValue(Action* action) override;

private:
    // An action's name never changes, so neither do these
    struct Names
    {
        bool starfall = false;
        bool crowdControl = false;
    };

    RaidSplashCache _splash;
    RaidActionCache<Names> _names;
};

// A non-tank's burrower damage only on its own pick, so a multidot doesn't pull a loose one, and no area damage,
// taunt or Bladestorm that would pull one. Interrupts and stuns on a cast go through. Wired from the definition.
class AnubarakBurrowerGuardMultiplier : public Multiplier
{
public:
    AnubarakBurrowerGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak burrower guard") {}
    float GetValue(Action* action) override;

private:
    // An action's name never changes, so neither do these
    struct Names
    {
        bool bladestorm = false;
        bool mover = false;
    };

    RaidSplashCache _splash;
    RaidActionCache<Names> _names;
};

// A DPS bot's damage on a burrower it has 90% of the side tank's threat on, until under 80%, and its area damage
// near one. Heals pass and white swings go on. Wired from the definition.
class AnubarakBurrowerThreatCapMultiplier : public Multiplier
{
public:
    AnubarakBurrowerThreatCapMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak burrower threat cap") {}
    float GetValue(Action* action) override;

private:
    RaidSplashCache _splash;
};

// A side tank's area and chain spells only where AnubarakSideTankAreaAllowed passes them: none of the
// other side's burrowers hit, him only under the side tank's cap. Wired from the definition.
class AnubarakSideTankAreaGuardMultiplier : public Multiplier
{
public:
    AnubarakSideTankAreaGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak side tank area guard") {}
    float GetValue(Action* action) override;

private:
    RaidSplashCache _splash;
};

// Heroic phase 3: no heal that only feeds Leeching Swarm, on any non-tank without Penetrating Cold, the
// caster itself too, and no area heal. Wired from the definition.
class AnubarakSwarmHealMultiplier : public Multiplier
{
public:
    AnubarakSwarmHealMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubarak swarm heal") {}
    float GetValue(Action* action) override;

private:
    std::unordered_map<Action*, bool> _heals;
};

void AddToCAnubarakMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

RaidBurstWindow ToCAnubarakBurstWindow(PlayerbotAI* botAI);

#endif
