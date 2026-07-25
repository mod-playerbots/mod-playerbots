/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObject.h"

#include "Playerbots.h"

AiObject::AiObject(PlayerbotAI* botAI)
    : PlayerbotAIAware(botAI), bot(botAI->GetBot()), context(botAI->GetAiObjectContext()), chat(botAI->GetChatHelper())
{
}

Player* AiObject::GetMaster() { return botAI->GetMaster(); }
