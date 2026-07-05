/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AfflictionWarlockStrategy.h"
#include "Playerbots.h"

AfflictionWarlockStrategy::AfflictionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

// ===== Default Actions =====
std::vector<NextAction> AfflictionWarlockStrategy::getDefaultActions()
{
    return {
       NextAction("corruption", 5.5f),
       NextAction("unstable affliction", 5.4f),
       NextAction("haunt", 5.3f),
       NextAction("shadow bolt", 5.2f),
       NextAction("shoot", 5.0f)
    };
}

// ===== Trigger Initialization ===
void AfflictionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime
    triggers.push_back(
        new TriggerNode(
            "corruption on attacker",
            {
                NextAction("corruption on attacker", 19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unstable affliction on attacker",
            {
                NextAction("unstable affliction on attacker", 19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "corruption",
            {
                NextAction("corruption", 18.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unstable affliction",
            {
                NextAction("unstable affliction", 17.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "haunt",
            {
                NextAction("haunt", 16.5f)
            }
        )
    );

    // Drain Soul as execute if target is low HP // Shadow Trance for free casts
    triggers.push_back(
        new TriggerNode(
            "shadow trance",
            {
                NextAction("shadow bolt", 16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                NextAction("drain soul", 15.5f)
            }
        )
    );

    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(
        new TriggerNode(
            "life tap glyph buff",
            {
                NextAction("life tap", 29.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "life tap",
            {
                NextAction("life tap", 5.1f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell",
            {
                NextAction("flee", 39.0f)
            }
        )
    );
}
