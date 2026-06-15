/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_EOEMULTIPLIERS_H
#define _PLAYERBOT_EOEMULTIPLIERS_H

#include "Multiplier.h"

class MalygosMultiplier : public Multiplier
{
public:
    MalygosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "malygos") {}

public:
    virtual float GetValue(Action* action);
};

#endif
