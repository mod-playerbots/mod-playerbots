/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_FACTIONCHAMPIONS_H

#include <vector>

#include "Multiplier.h"
#include "RaidAntiFear.h"
#include "RaidBurstWindow.h"
#include "RaidCombatHelpers.h"
#include "RaidSplash.h"

// Single-target only: Champion's Aegis 68595 is a flat -75% to AoE damage on every champion
class FactionChampionsSuppressAoeMultiplier : public Multiplier
{
public:
    FactionChampionsSuppressAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions suppress aoe multiplier") {}
    float GetValue(Action* action) override;

private:
    bool Splashes(Action* action);

    RaidSplashCache _splash;
    RaidActionCache<bool> _ownSplash;
};

// The script re-seeds threat every ~9 s, so a redirect onto the main tank buys nothing
class FactionChampionsThreatRedirectVetoMultiplier : public Multiplier
{
public:
    FactionChampionsThreatRedirectVetoMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions threat redirect veto multiplier") {}
    float GetValue(Action* action) override;

private:
    RaidActionCache<bool> _mainTankRedirect;
};

class FactionChampionsAntiFearTotemGuardMultiplier : public RaidAntiFearTotemGuardMultiplier
{
public:
    FactionChampionsAntiFearTotemGuardMultiplier(PlayerbotAI* botAI)
        : RaidAntiFearTotemGuardMultiplier(botAI, "faction champions anti fear totem guard multiplier")
    {
    }

protected:
    bool FearWindowActive() override;
};

// Class CC nodes would pick their own targets, so CC goes out only through the CC duty. A DoT spread
// goes on no champion but the kill target.
class FactionChampionsCcGuardMultiplier : public Multiplier
{
public:
    FactionChampionsCcGuardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions cc guard multiplier") {}
    float GetValue(Action* action) override;

private:
    struct Facts
    {
        bool classCc = false;
        bool spreadDot = false;
    };

    RaidActionCache<Facts> _facts;
};

// As in PvP, kicks wait for a champion cast's last LATE_INTERRUPT_MS so the cast is wasted, channels go at
// once. The mage due to Counterspell starts no cast meanwhile.
class FactionChampionsLateInterruptMultiplier : public Multiplier
{
public:
    FactionChampionsLateInterruptMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "faction champions late interrupt multiplier") {}
    float GetValue(Action* action) override;

private:
    struct Facts
    {
        CastSpellAction* cast = nullptr;
        bool healerKick = false;  // named "... on enemy healer"
        RaidSpellIdRef spellId;
    };

    RaidActionCache<Facts> _facts;
};

void AddToCFactionChampionsMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);

RaidBurstWindow ToCFactionChampionsBurstWindow(PlayerbotAI* botAI);

#endif
