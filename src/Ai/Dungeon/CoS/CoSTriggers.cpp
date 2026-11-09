/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool ExplodeGhoulTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "salramm the fleshcrafter")) { return false; }

    float distance = 10.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_RISEN_GHOUL)
        {
            if (bot->GetExactDist2d(unit) < distance + distanceExtra)
            {
                return true;
            }
        }
    }
    return false;
}

bool EpochRangedTrigger::IsActive()
{
    return !PlayerbotAI::IsMelee(bot) && FindThreatTarget(bot, "chrono-lord epoch");
}
