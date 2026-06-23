#include "Playerbots.h"
#include "HRTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "RaidBossHelpers.h"

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

bool OmorTreacheryAuraTrigger::IsActive()
{
    return (botAI->IsHeal(bot) || botAI->IsDps(bot)) &&
           (bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
            bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)));
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

bool OmorTankHasTreacheryAuraTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Player* tank = GetGroupMainTank(botAI, bot);
    if (!tank)
        return false;

    if (tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)) ||
        tank->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
    {
        return true;
    }

    return false;
}

// Vazruden

bool VazrudenTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "vazruden");
}
