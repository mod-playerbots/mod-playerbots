/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGMULTIPLIERS_H
#define _PLAYERBOT_MAGMULTIPLIERS_H

#include "Multiplier.h"

class MagtheridonUseManticronCubeMultiplier : public Multiplier
{
public:
    MagtheridonUseManticronCubeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon use manticron cube multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonWaitToAttackMultiplier : public Multiplier
{
public:
    MagtheridonWaitToAttackMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon wait to attack multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonDisableOffTankAssistMultiplier : public Multiplier
{
public:
    MagtheridonDisableOffTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon disable off tank assist multiplier") {}
    float GetValue(Action* action) override;
};

#endif
