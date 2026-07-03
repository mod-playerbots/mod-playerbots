/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGMULTIPLIERS_H
#define PLAYERBOTS_MAGMULTIPLIERS_H

#include "Multiplier.h"

class MagtheridonUseManticronCubeMultiplier : public Multiplier
{
public:
    MagtheridonUseManticronCubeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon use manticron cube multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonWaitToAttackMultiplier : public Multiplier
{
public:
    MagtheridonWaitToAttackMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon wait to attack multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonControlTankActionsMultiplier : public Multiplier
{
public:
    MagtheridonControlTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon control tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonDebrisDangerMultiplier : public Multiplier
{
public:
    MagtheridonDebrisDangerMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon debris danger multiplier") {}
    float GetValue(Action* action) override;
};

#endif
