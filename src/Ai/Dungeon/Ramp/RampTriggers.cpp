/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObject.h"
#include "AiObjectContext.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampTriggers.h"

using namespace EncounterHelpers;

// Watchkeeper Gargolmar

bool GargolmarHellfireWatchersAreActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "hellfire watcher");
}

// Omor the Unscarred

bool OmorTreacheryAuraTrigger::IsActive()
{
    return !botAI->IsTank(bot) && (bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
                                   bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)));
}

bool OmorTankHasTreacheryAuraTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Player* tank = GetGroupMainTank(bot);
    if (!tank)
        return false;

    if (tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
        tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
    {
        return true;
    }

    return false;
}

bool OmorRangedSpreadTrigger::IsActive()
{
    return botAI->IsRanged(bot) && AI_VALUE2(Unit*, "find target", "omor the unscarred");
}

bool OmorFiendishHoundIsActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "fiendish hound");
}

// Vazruden

bool VazrudenTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) && AI_VALUE2(Unit*, "find target", "vazruden");
}

bool VazrudenBossIsActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "vazruden");
}
