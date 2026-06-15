/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UPMULTIPLIERS_H
#define _PLAYERBOT_UPMULTIPLIERS_H

#include "Multiplier.h"

class SkadiMultiplier : public Multiplier
{
    public:
        SkadiMultiplier(PlayerbotAI* ai) : Multiplier(ai, "skadi the ruthless") {}

    public:
        virtual float GetValue(Action* action);
};

class YmironMultiplier : public Multiplier
{
    public:
        YmironMultiplier(PlayerbotAI* ai) : Multiplier(ai, "king ymiron") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
