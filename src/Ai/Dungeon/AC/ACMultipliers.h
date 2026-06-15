/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACMULTIPLIERS_H
#define _PLAYERBOT_ACMULTIPLIERS_H

#include "Multiplier.h"

class ShirrakFleeFocusFireMultiplier : public Multiplier
{
public:
    ShirrakFleeFocusFireMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shirrak flee focus fire") {}
    float GetValue(Action* action) override;
};

#endif
