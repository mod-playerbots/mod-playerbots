/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AKTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool NadoxGuardianTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot)) { return false; }

    return FindThreatTarget(bot, "elder nadox") && FindThreatTarget(bot, "ahn'kahar guardian");
}

bool JedogaVolunteerTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "jedoga shadowseeker")) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_TWILIGHT_VOLUNTEER)
        {
            return true;
        }
    }
    return false;
}

bool ShadowCrashTrigger::IsActive()
{
    return FindThreatTarget(bot, "forgotten one") && !PlayerbotAI::IsMelee(bot);
}
