/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NewRpgStrategy.h"

NewRpgStrategy::NewRpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> NewRpgStrategy::getDefaultActions()
{
    // the releavance should be greater than grind
    return {
        NextAction("new rpg status update", 11.0f)
    };
}

void NewRpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "go grind status",
            {
                NextAction("new rpg go grind", 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "go camp status",
            {
                NextAction("new rpg go camp", 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wander random status",
            {
                NextAction("new rpg wander random", 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wander npc status",
            {
                NextAction("new rpg wander npc", 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "do quest status",
            {
                NextAction("new rpg do quest", 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "travel flight status",
            {
                NextAction("new rpg travel flight", 3.0f)
            }
        )
    );
}

void NewRpgStrategy::InitMultipliers(std::vector<Multiplier*>&)
{
}
