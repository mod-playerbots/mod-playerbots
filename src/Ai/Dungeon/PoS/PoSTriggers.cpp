/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSTriggers.h"
#include "ThreatHelpers.h"

bool IckAndKrickTrigger::IsActive()
{
    return FindThreatTarget(bot, "ick");
}

bool TyrannusTrigger::IsActive()
{
    return FindThreatTarget(bot, "scourgelord tyrannus");
}
