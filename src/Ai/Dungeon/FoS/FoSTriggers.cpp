/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "bronjahm");
    if (!boss)
        return false;

    if (!boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
        return false;

    if (!bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool SwitchToSoulFragment::IsActive()
{
    if (!FindThreatTarget(bot, "bronjahm"))
        return false;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (!activeSoulExists)
        return false;

    return true;
}

bool BronjahmPositionTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "bronjahm"))
        return false;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool DevourerOfSoulsTrigger::IsActive()
{
    return FindThreatTarget(bot, "devourer of souls");
}
