/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GDMULTIPLIERS_H
#define _PLAYERBOT_GDMULTIPLIERS_H

#include "Multiplier.h"

class SladranMultiplier : public Multiplier
{
    public:
        SladranMultiplier(PlayerbotAI* ai) : Multiplier(ai, "slad'ran") {}

    public:
        virtual float GetValue(Action* action);
};

class GaldarahMultiplier : public Multiplier
{
    public:
        GaldarahMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gal'darah") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
