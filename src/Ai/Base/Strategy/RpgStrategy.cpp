/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RpgStrategy.h"

#include "Playerbots.h"
#include "RpgSubActions.h"

float RpgActionMultiplier::GetValue(Action* action)
{
    if (action == nullptr)
        return 1.0f;

    std::string const nextAction = AI_VALUE(std::string, "next rpg action");
    std::string const name = action->getName();

    if (!nextAction.empty() && dynamic_cast<RpgEnabled*>(action) && name != nextAction)
        return 0.0f;

    return 1.0f;
}

RpgStrategy::RpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> RpgStrategy::getDefaultActions()
{
    return {
        NextAction("rpg", 1.0f)
    };
}

void RpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no rpg target",
            {
                NextAction("choose rpg target", 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                NextAction("move random", 1.10f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "far from rpg target",
            {
                NextAction("move to rpg target", 5.0f)
            }
        )
    );

    // Sub actions
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                NextAction("rpg stay", 1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                NextAction("rpg work", 1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg",
            {
                NextAction("rpg emote", 1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "has rpg target",
            {
                NextAction("rpg cancel", 1.101f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg discover",
            {
                NextAction("rpg discover", 1.210f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg start quest",
            {
                NextAction("rpg start quest", 1.180f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg end quest",
            {
                NextAction("rpg end quest", 1.190f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy",
            {
                NextAction("rpg buy", 1.130f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg repair",
            {
                NextAction("rpg repair", 1.195f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg heal",
            {
                NextAction("rpg heal", 1.125f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg home bind",
            {
                NextAction("rpg home bind", 1.160f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg buy petition",
            {
                NextAction("rpg buy petition", 1.140f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rpg use",
            {
                NextAction("rpg use", 1.102f)
            }
        )
    );
}

void RpgStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(botAI));
}
