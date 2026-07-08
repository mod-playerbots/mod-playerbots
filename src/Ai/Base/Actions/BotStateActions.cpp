/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotStateActions.h"

#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool SetCombatStateAction::Execute(Event event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    botAI->ChangeEngine(BOT_STATE_COMBAT);
    return true;
}

bool SetCombatStateAction::isUseful()
{
    return botAI->GetState() != BOT_STATE_COMBAT;
}
