/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Playerbots.h"
#include "PoSTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool IckAndKrickTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "Ick");
    if (!boss)
        return false;

    return true;
}

bool TyrannusTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    return true;
}
