/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UseFoodStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "CreateNextAction.h"
#include "NonCombatActions.h"

void UseFoodStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);
    if (botAI->HasCheat(BotCheatMask::food))
    {
        triggers.push_back(
            new TriggerNode(
                "medium health",
                {
                    CreateNextAction<EatAction>(3.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<DrinkAction>(3.0f)
                }
            )
        );
    }
    else
    {
        triggers.push_back(
            new TriggerNode(
                "low health",
                {
                    CreateNextAction<EatAction>(3.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "low mana",
                {
                    CreateNextAction<DrinkAction>(3.0f)
                }
            )
        );
    }
}
