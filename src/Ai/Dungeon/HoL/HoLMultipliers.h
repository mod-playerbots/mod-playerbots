/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HOLMULTIPLIERS_H
#define _PLAYERBOT_HOLMULTIPLIERS_H

#include "Multiplier.h"

class BjarngrimMultiplier : public Multiplier
{
    public:
        BjarngrimMultiplier(PlayerbotAI* ai) : Multiplier(ai, "general bjarngrim") {}

    public:
        virtual float GetValue(Action* action);
};

class VolkhanMultiplier : public Multiplier
{
    public:
        VolkhanMultiplier(PlayerbotAI* ai) : Multiplier(ai, "volkhan") {}

    public:
        virtual float GetValue(Action* action);
};

class IonarMultiplier : public Multiplier
{
    public:
        IonarMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ionar") {}

    public:
        virtual float GetValue(Action* action);
};

class LokenMultiplier : public Multiplier
{
    public:
        LokenMultiplier(PlayerbotAI* ai) : Multiplier(ai, "loken") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
