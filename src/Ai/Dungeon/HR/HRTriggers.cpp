#include "Playerbots.h"
#include "HRTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

// Watchkeeper Gargolmar

bool GargolmarTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
}

bool GargolmarHellfireWatchersAreActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "hellfire watcher");
}

// Omor the Unscarred

bool OmorTreacherousAuraTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA));
}

bool OmorBaneOfTreacheryAuraTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY));
}

bool OmorRangedSpreadTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "omor the unscarred");
}

bool OmorFiendishHoundIsActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "fiendish hound");
}

// Vazruden

bool VazrudenTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "vazruden");
}
