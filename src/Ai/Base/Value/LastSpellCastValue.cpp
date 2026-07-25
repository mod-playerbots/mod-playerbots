/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LastSpellCastValue.h"

#include "Playerbots.h"

LastSpellCast::LastSpellCast() : id(0), timer(0) {}

void LastSpellCast::Set(uint32 newId, ObjectGuid newTarget, time_t newTime)
{
    id = newId;
    target = newTarget;
    timer = newTime;
}

void LastSpellCast::Reset()
{
    id = 0;
    target.Clear();
    timer = 0;
}
