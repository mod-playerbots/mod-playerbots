/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UseFoodStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

void UseFoodStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);
    if (botAI->HasCheat(BotCheatMask::food))
    {
        triggers.push_back(new TriggerNode("medium health", { NextAction("food", 3.0f) }));
        triggers.push_back(new TriggerNode("high mana", { NextAction("drink", 3.0f) }));
    }
    else
    {
        triggers.push_back(new TriggerNode("low health", { NextAction("food", 3.0f) }));
        triggers.push_back(new TriggerNode("low mana", { NextAction("drink", 3.0f) }));
    }
}
