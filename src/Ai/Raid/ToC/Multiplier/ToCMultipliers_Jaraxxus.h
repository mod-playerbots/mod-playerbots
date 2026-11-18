/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_JARAXXUS_H

#include "Multiplier.h"
#include "RaidBurstWindow.h"
#include "RaidSplash.h"

// Keep tanks anchored on Jaraxxus and his adds instead of drifting with the combat formation
class JaraxxusControlTankMovementMultiplier : public Multiplier
{
public:
    JaraxxusControlTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "jaraxxus control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

// No class taunt off another tank holding Jaraxxus
class JaraxxusTauntGuardMultiplier : public Multiplier
{
public:
    JaraxxusTauntGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "jaraxxus taunt guard") {}
    float GetValue(Action* action) override;
};

// Mistress' Kiss punishes a cast still running on its 0.5 s check, so a kissed bot casts instants only
class JaraxxusKissCastHoldMultiplier : public Multiplier
{
public:
    JaraxxusKissCastHoldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "jaraxxus kiss cast hold") {}
    float GetValue(Action* action) override;
};

// AvoidAoeAction flees a flame by FleePosition out to 5.5 yd (GetDistance takes off both combat
// reaches), past where our own dodge starts, so the flames stay ours alone
class JaraxxusAvoidAoeGuardMultiplier : public Multiplier
{
public:
    JaraxxusAvoidAoeGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "jaraxxus avoid aoe guard") {}
    float GetValue(Action* action) override;
};

// A DPS bot's damage on him, and its area damage within 30 yd of him, while its threat cap latch holds.
// Heals and anything aimed elsewhere go through. Wired from the definition.
class JaraxxusThreatCapMultiplier : public Multiplier
{
public:
    JaraxxusThreatCapMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "jaraxxus threat cap") {}
    float GetValue(Action* action) override;

private:
    RaidSplashCache _splash;
};

RaidBurstWindow ToCJaraxxusBurstWindow(PlayerbotAI* botAI);

#endif
