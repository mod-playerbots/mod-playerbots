/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ReactionStrategy.h"

#include "Playerbots.h"

void ReactionStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    // Upstream cmangos also routes combat end, death and resurrect through here; this port
    // keeps those transitions inline in PlayerbotAI and only promotes combat start, so bots
    // enter the combat engine within reactDelay even while the main AI is delayed
    // (eating, drinking, casting, long action durations).
    triggers.push_back(
        new TriggerNode(
            "combat start",
            {
                NextAction("set combat state", ACTION_PASSTROUGH + 10)
            }
        )
    );
}
