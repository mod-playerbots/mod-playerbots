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
    float GetValue(Action* action) override;
};

class RsBaltharusBrandSafeMultiplier : public Multiplier
{
public:
    RsBaltharusBrandSafeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs baltharus brand safe") {}
    float GetValue(Action* action) override;
};

class RsSavianaMeleeSpreadMultiplier : public Multiplier
{
public:
    RsSavianaMeleeSpreadMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs saviana melee spread") {}
    float GetValue(Action* action) override;
};

class RsZarithrianAddsMultiplier : public Multiplier
{
public:
    RsZarithrianAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs zarithrian adds") {}
    float GetValue(Action* action) override;
};

class RsZarithrianTankSwapMultiplier : public Multiplier
{
public:
    RsZarithrianTankSwapMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs zarithrian tank swap") {}
    float GetValue(Action* action) override;
};

class RsHalionCombustionMultiplier : public Multiplier
{
public:
    RsHalionCombustionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion combustion safe") {}
    float GetValue(Action* action) override;
};

class RsHalionMeteorMultiplier : public Multiplier
{
public:
    RsHalionMeteorMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion meteor") {}
    float GetValue(Action* action) override;
};

class RsHalionMeleeFlankMultiplier : public Multiplier
{
public:
    RsHalionMeleeFlankMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion melee flank") {}
    float GetValue(Action* action) override;
};

class RsHalionP2Multiplier : public Multiplier
{
public:
    RsHalionP2Multiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion p2") {}
    float GetValue(Action* action) override;
};

class RsHalionHpBalanceMultiplier : public Multiplier
{
public:
    RsHalionHpBalanceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion hp balance") {}
    float GetValue(Action* action) override;
};

class RsHalionRealmIsolationMultiplier : public Multiplier
{
public:
    RsHalionRealmIsolationMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs halion realm isolation") {}
    float GetValue(Action* action) override;
};

class RsTrashAddsMultiplier : public Multiplier
{
public:
    RsTrashAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rs trash adds") {}
    float GetValue(Action* action) override;
};

#endif
