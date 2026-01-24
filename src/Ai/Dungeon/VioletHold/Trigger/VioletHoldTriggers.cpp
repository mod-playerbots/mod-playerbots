#include "Playerbots.h"
#include "BotRoleService.h"
#include "VioletHoldTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool ErekemTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss) { return false; }

    return BotRoleService::IsDpsStatic(bot);
}

bool IchoronTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return false; }

    return !BotRoleService::IsHealStatic(bot);
}

bool VoidShiftTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    return bot->HasAura(SPELL_VOID_SHIFTED) && !BotRoleService::IsHealStatic(bot);
}

bool ShroudOfDarknessTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    return boss->HasAura(SPELL_SHROUD_OF_DARKNESS);
}

bool CyanigosaPositioningTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "cyanigosa");
    if (!boss) { return false; }

    // Include healers here for now, otherwise they stand in things
    return !BotRoleService::IsTankStatic(bot) && !BotRoleService::IsRangedDpsStatic(bot);
    // return BotRoleService::IsMeleeStatic(bot) && !BotRoleService::IsTankStatic(bot);
}
