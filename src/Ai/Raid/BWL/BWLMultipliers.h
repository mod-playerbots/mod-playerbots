/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLMULTIPLIERS_H
#define PLAYERBOTS_BWLMULTIPLIERS_H

#include "Multiplier.h"

class RazorgoreTankMultiplier : public Multiplier
{
public:
    RazorgoreTankMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "razorgore tank multiplier") {}
    float GetValue(Action* action) override;
};

class VaelastraszTankMultiplier : public Multiplier
{
public:
    VaelastraszTankMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "vaelastrasz tank multiplier") {}
    float GetValue(Action* action) override;
};

class VaelastraszBurningAdrenalineMultiplier : public Multiplier
{
public:
    VaelastraszBurningAdrenalineMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "vaelastrasz burning adrenaline multiplier") {}
    float GetValue(Action* action) override;
};

class NefarianCorruptedHealingMultiplier : public Multiplier
{
public:
    NefarianCorruptedHealingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nefarian corrupted healing multiplier") {}
    float GetValue(Action* action) override;
};

class NefarianCorruptedTotemMultiplier : public Multiplier
{
public:
    NefarianCorruptedTotemMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nefarian corrupted totem multiplier") {}
    float GetValue(Action* action) override;
};

// DPS on a Wyrmguard waits for the tanks to reach their spots, with a time limit: a tank that cannot get there must not
// leave the raid idle.
class DeathTalonWyrmguardDpsHoldMultiplier : public Multiplier
{
public:
    DeathTalonWyrmguardDpsHoldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "death talon wyrmguard dps hold") {}
    float GetValue(Action* action) override;
};

class DeathTalonWyrmguardTankMultiplier : public Multiplier
{
public:
    DeathTalonWyrmguardTankMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "death talon wyrmguard tank") {}
    float GetValue(Action* action) override;
};

#endif
