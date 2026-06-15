/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSMULTIPLIERS_H
#define _PLAYERBOT_POSMULTIPLIERS_H

#include "Multiplier.h"

class IckAndKrickMultiplier : public Multiplier
{
    public:
    IckAndKrickMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ick and krick") {}

    public:
        virtual float GetValue(Action* action);
};

class GarfrostMultiplier : public Multiplier
{
public:
    GarfrostMultiplier(PlayerbotAI* ai) : Multiplier(ai, "garfrost") { }

    float GetValue(Action* action) override;
};

#endif
