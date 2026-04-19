/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WAITFORATTACKTIMEVALUE_H
#define _PLAYERBOT_WAITFORATTACKTIMEVALUE_H

#include "Value.h"

class PlayerbotAI;

class WaitForAttackTimeValue : public ManualSetValue<uint8>
{
public:
    WaitForAttackTimeValue(PlayerbotAI* botAI) : ManualSetValue<uint8>(botAI, 10, "wait for attack time") {}
};

class CombatStartTimeValue : public ManualSetValue<time_t>
{
public:
    CombatStartTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0, "combat start time") {}
};

#endif
