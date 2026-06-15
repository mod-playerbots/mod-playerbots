/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OSMULTIPLIERS_H
#define _PLAYERBOT_OSMULTIPLIERS_H

#include "Multiplier.h"

class SartharionMultiplier : public Multiplier
{
public:
    SartharionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sartharion") {}

public:
    virtual float GetValue(Action* action);
};

#endif
