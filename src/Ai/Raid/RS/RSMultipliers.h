/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSM_H
#define PLAYERBOTS_RSM_H

#include "Multiplier.h"

class RsSavianaBeaconMultiplier : public Multiplier
{
public:
    RsSavianaBeaconMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs saviana beacon") {}
    virtual float GetValue(Action* action);
};

class RsBaltharusBrandSafeMultiplier : public Multiplier
{
public:
    RsBaltharusBrandSafeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs baltharus brand safe") {}
    virtual float GetValue(Action* action);
};

class RsSavianaMeleeSpreadMultiplier : public Multiplier
{
public:
    RsSavianaMeleeSpreadMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs saviana melee spread") {}
    virtual float GetValue(Action* action);
};

class RsZarithrianAddsMultiplier : public Multiplier
{
public:
    RsZarithrianAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs zarithrian adds") {}
    virtual float GetValue(Action* action);
};

class RsZarithrianTankSwapMultiplier : public Multiplier
{
public:
    RsZarithrianTankSwapMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs zarithrian tank swap") {}
    virtual float GetValue(Action* action);
};

class RsHalionCombustionMultiplier : public Multiplier
{
public:
    RsHalionCombustionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion combustion safe") {}
    virtual float GetValue(Action* action);
};

class RsHalionMeteorMultiplier : public Multiplier
{
public:
    RsHalionMeteorMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion meteor") {}
    virtual float GetValue(Action* action);
};

class RsHalionMeleeFlankMultiplier : public Multiplier
{
public:
    RsHalionMeleeFlankMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion melee flank") {}
    virtual float GetValue(Action* action);
};

class RsHalionP2Multiplier : public Multiplier
{
public:
    RsHalionP2Multiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion p2") {}
    virtual float GetValue(Action* action);
};

class RsHalionHpBalanceMultiplier : public Multiplier
{
public:
    RsHalionHpBalanceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion hp balance") {}
    virtual float GetValue(Action* action);
};

class RsHalionRealmIsolationMultiplier : public Multiplier
{
public:
    RsHalionRealmIsolationMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion realm isolation") {}
    virtual float GetValue(Action* action);
};

class RsTrashAddsMultiplier : public Multiplier
{
public:
    RsTrashAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs trash adds") {}
    virtual float GetValue(Action* action);
};

#endif
