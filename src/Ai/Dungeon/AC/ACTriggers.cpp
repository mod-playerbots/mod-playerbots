/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Playerbots.h"
#include "ACTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

// Shirrak the Dead Watcher

bool ShirrakTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
}

bool ShirrakFleeFocusFireTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "shirrak the dead watcher"))
        return false;

        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
            return true;
    }
    return false;
}

bool ShirrakRangedKeepDistanceTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
}
