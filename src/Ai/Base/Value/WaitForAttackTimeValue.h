/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAITFORATTACKTIMEVALUE_H
#define PLAYERBOTS_WAITFORATTACKTIMEVALUE_H

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
