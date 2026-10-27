/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RitualOfSoulsTriggers.h"

#include "Playerbots.h"

bool RitualPortalNearbyTrigger::IsActive()
{
    return !bot->IsInCombat() && AI_VALUE(ObjectGuid, "ritual of souls portal");
}

bool SoulwellNearbyTrigger::IsActive()
{
    return !bot->IsInCombat() && AI_VALUE(ObjectGuid, "ritual of souls soulwell");
}
