/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FOSMULTIPLIERS_H
#define _PLAYERBOT_FOSMULTIPLIERS_H

#include "Multiplier.h"

class BronjahmMultiplier : public Multiplier
{
    public:
    BronjahmMultiplier(PlayerbotAI* ai) : Multiplier(ai, "bronjahm") {}

    public:
        virtual float GetValue(Action* action);
};

class AttackFragmentMultiplier : public Multiplier
{
public:
    AttackFragmentMultiplier(PlayerbotAI* ai) : Multiplier(ai, "attack fragment") { }

    float GetValue(Action* action) override;
};

#endif
