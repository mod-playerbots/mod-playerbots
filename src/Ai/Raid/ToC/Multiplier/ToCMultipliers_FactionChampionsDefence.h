/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_FACTIONCHAMPIONSDEFENCE_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_FACTIONCHAMPIONSDEFENCE_H

#include <vector>

#include "Multiplier.h"
#include "RaidCombatHelpers.h"

// Melee hold rather than walk into a Bladestorm or Hellfire around their target, the dodge would only
// walk them back out
class FactionChampionsAoeGuardMultiplier : public Multiplier
{
public:
    FactionChampionsAoeGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions aoe guard multiplier") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _meleeMove;
};

// The focus node would pull physical attackers back onto a Hand of Protection kill target
class FactionChampionsPhysicalSwitchGuardMultiplier : public Multiplier
{
public:
    FactionChampionsPhysicalSwitchGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions physical switch guard multiplier") {}
    float GetValue(Action* action) override;
};

// Dispelling Unstable Affliction silences the dispeller and hits it hard, and a dispel picks its aura at
// random, so no magic dispel on its carrier
class FactionChampionsDispelGuardMultiplier : public Multiplier
{
public:
    FactionChampionsDispelGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions dispel guard multiplier") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _magicDispel;
};

// Class purge and spellsteal fire on any magic buff, Thorns included, so one duty bot purges instead
class FactionChampionsPurgeGuardMultiplier : public Multiplier
{
public:
    FactionChampionsPurgeGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions purge guard multiplier") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _purge;
};

// A root or stun refuses Death Grip's pull or drops the champion mid-flight; area and self-cast ones wait
// while any guard is up. The class pet toggle would turn a held autocast straight back on.
class FactionChampionsGripGuardMultiplier : public Multiplier
{
public:
    FactionChampionsGripGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions grip guard multiplier") {}
    float GetValue(Action* action) override;

private:
    struct Facts
    {
        bool petToggle = false;
        CastSpellAction* cast = nullptr;  // null for a pet toggle
        RaidSpellIdRef spellId;
    };

    RaidActionCache<Facts> _facts;
};

void AddToCFactionChampionsDefenceMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

#endif
