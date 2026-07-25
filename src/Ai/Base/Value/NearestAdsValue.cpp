/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestAdsValue.h"

#include "Playerbots.h"

bool NearestAddsValue::AcceptUnit(Unit* unit)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return unit != target;
}
