/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampTriggers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"

using namespace EncounterHelpers;

// Watchkeeper Gargolmar

bool GargolmarHellfireWatchersAreActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "hellfire watcher");
}

// Omor the Unscarred

bool OmorTreacheryAuraTrigger::IsActive()
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    if (!omor)
        return false;

    Unit* omorVictim = omor->GetVictim();

    bool botIsOmorVictim = omorVictim && omorVictim->GetGUID() == bot->GetGUID();

    return !botIsOmorVictim && (bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
                                bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)));
}

bool OmorTankHasTreacheryAuraTrigger::IsActive()
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    if (!omor)
        return false;

    Unit* omorVictim = omor->GetVictim();

    if (!omorVictim || omorVictim->GetGUID() != bot->GetGUID())
        return false;

    return  bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
            bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA));
}

bool OmorRangedSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "omor the unscarred");
}

bool OmorFiendishHoundIsActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "fiendish hound");
}

// Vazruden

bool VazrudenTankPositionBossTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "vazruden");
}

bool VazrudenBossIsActiveTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "vazruden");
}
