/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MCMULTIPLIERS_H
#define _PLAYERBOT_MCMULTIPLIERS_H

#include "Multiplier.h"

class GarrDisableDpsAoeMultiplier : public Multiplier
{
public:
    GarrDisableDpsAoeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "garr disable dps aoe multiplier") {}
    float GetValue(Action* action) override;
};

class BaronGeddonAbilityMultiplier : public Multiplier
{
public:
    BaronGeddonAbilityMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "baron geddon ability multiplier") {}
    float GetValue(Action* action) override;
};

class GolemaggMultiplier : public Multiplier
{
public:
    GolemaggMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "golemagg multiplier") {}
    float GetValue(Action* action) override;
};

#endif
