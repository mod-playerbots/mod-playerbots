/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Playerbots.h"
#include "VHTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool ErekemTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss) { return false; }

    return botAI->IsDps(bot);
}

bool IchoronTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return false; }

    return !botAI->IsHeal(bot);
}

bool VoidShiftTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    return bot->HasAura(SPELL_VOID_SHIFTED) && !botAI->IsHeal(bot);
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
    return !botAI->IsTank(bot) && !botAI->IsRangedDps(bot);
    // return botAI->IsMelee(bot) && !botAI->IsTank(bot);
}
