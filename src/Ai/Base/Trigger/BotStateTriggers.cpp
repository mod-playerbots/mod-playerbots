/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotStateTriggers.h"

#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool CombatStartTrigger::IsActive()
{
    if (botAI->GetState() == BOT_STATE_COMBAT || botAI->GetState() == BOT_STATE_DEAD)
        return false;

    // Attackers (bot's own and nearby group members') rather than the raw combat flag:
    // guarantees the combat engine will have a target to select, so switching early can't
    // ping-pong back through "invalid target" -> "drop target".
    return !AI_VALUE(GuidVector, "attackers").empty();
}
