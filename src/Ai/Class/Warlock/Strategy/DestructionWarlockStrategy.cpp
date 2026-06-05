/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DestructionWarlockStrategy.h"
#include "Playerbots.h"

DestructionWarlockStrategy::DestructionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

// ===== Default Actions =====
std::vector<NextAction> DestructionWarlockStrategy::getDefaultActions()
{
    return {
       NextAction("immolate", 5.9f),
       NextAction("conflagrate", 5.8f),
       NextAction("chaos bolt", 5.7f),
       NextAction("incinerate", 5.6f),
       NextAction("corruption", 5.3f),      // Note: Corruption and Shadow Bolt won't be used after the character learns Incinerate at level 64
       NextAction("shadow bolt", 5.2f),
       NextAction("shoot", 5.0f)
    };
}

// ===== Trigger Initialization ===
void DestructionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime + high priority cooldowns
    triggers.push_back(
        new TriggerNode(
            "immolate",
            {
                NextAction("immolate", 20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "conflagrate",
            {
                NextAction("conflagrate", 19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "chaos bolt",
            {
                NextAction("chaos bolt", 19.0f)
            }
        )
    );

    // Note: Corruption won't be used after the character learns Incinerate at level 64
    triggers.push_back(
        new TriggerNode(
            "corruption on attacker",
            {
                NextAction("corruption on attacker", 5.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "corruption",
            {
                NextAction("corruption", 5.4f)
            }
        )
    );

    // Shadowburn as execute if target is low HP
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                NextAction("shadowburn", 18.0f)
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
