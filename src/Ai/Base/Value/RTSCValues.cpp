/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RTSCValues.h"

#include "Playerbots.h"

bool SeeSpellLocationValue::EqualToLast(WorldPosition value) { return value == lastValue; }

WorldPosition SeeSpellLocationValue::Calculate() { return value; }

std::string const RTSCSavedLocationValue::Save() { return value.to_string(); }

bool RTSCSavedLocationValue::Load(std::string const text)
{
    value = WorldPosition(text);
    return true;
}
