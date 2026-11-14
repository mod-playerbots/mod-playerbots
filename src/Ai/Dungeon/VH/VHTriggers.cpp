/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool ErekemTargetTrigger::IsActive()
{
    return FindThreatTarget(bot, "erekem") && PlayerbotAI::IsDps(bot);
}

bool IchoronTargetTrigger::IsActive()
{
    return FindThreatTarget(bot, "ichoron") && !PlayerbotAI::IsHeal(bot);
}

bool VoidShiftTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "zuramat the obliterator")) { return false; }

    return bot->HasAura(SPELL_VOID_SHIFTED) && !PlayerbotAI::IsHeal(bot);
}

bool ShroudOfDarknessTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "zuramat the obliterator");
    return boss && boss->HasAura(SPELL_SHROUD_OF_DARKNESS);
}

bool CyanigosaPositioningTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "cyanigosa")) { return false; }

    // Include healers here for now, otherwise they stand in things
    return !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsRangedDps(bot);
}
