/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ANTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool KrikthirWebWrapTrigger::IsActive()
{
    if (!PlayerbotAI::IsDps(bot)) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_WEB_WRAP)
        {
            return true;
        }
    }

    return false;
}

bool KrikthirWatchersTrigger::IsActive()
{
    if (!PlayerbotAI::IsDps(bot)) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_KRIKTHIR)
        {
            return true;
        }
    }
    return false;
}

bool AnubarakPoundTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "anub'arak");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POUND);
}
