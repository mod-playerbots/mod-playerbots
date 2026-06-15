/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VHMULTIPLIERS_H
#define _PLAYERBOT_VHMULTIPLIERS_H

#include "Multiplier.h"

class ErekemMultiplier : public Multiplier
{
    public:
        ErekemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "erekem") {}

    public:
        virtual float GetValue(Action* action);
};

class IchoronMultiplier : public Multiplier
{
    public:
        IchoronMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ichoron") {}

    public:
        virtual float GetValue(Action* action);
};

class ZuramatMultiplier : public Multiplier
{
    public:
        ZuramatMultiplier(PlayerbotAI* ai) : Multiplier(ai, "zuramat the obliterator") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
