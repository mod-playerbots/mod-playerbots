/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COSMULTIPLIERS_H
#define _PLAYERBOT_COSMULTIPLIERS_H

#include "Multiplier.h"

class EpochMultiplier : public Multiplier
{
    public:
        EpochMultiplier(PlayerbotAI* ai) : Multiplier(ai, "chrono-lord epoch") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
