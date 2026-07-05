/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DemonologyWarlockStrategy.h"
#include "Playerbots.h"

DemonologyWarlockStrategy::DemonologyWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

// ===== Default Actions =====
std::vector<NextAction> DemonologyWarlockStrategy::getDefaultActions()
{
    return {
       NextAction("corruption", 5.5f),
       NextAction("immolate", 5.4f),
       NextAction("shadow bolt", 5.3f),
       NextAction("incinerate", 5.2f),
       NextAction("shoot", 5.0f) };
}

// ===== Trigger Initialization ===
void DemonologyWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // High priority cooldowns
    triggers.push_back(
        new TriggerNode(
            "metamorphosis",
            {
                NextAction("metamorphosis", 28.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "demonic empowerment",
            {
                NextAction("demonic empowerment", 28.0f)
            }
        )
    );

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
            "immolate on attacker",
            {
                NextAction("immolate on attacker", 19.0f)
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
            "immolate",
            {
                NextAction("immolate", 17.5f)
            }
        )
    );

    // Procs
    triggers.push_back(
        new TriggerNode(
            "decimation",
            {
                NextAction("soul fire", 17.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "molten core",
            {
                NextAction("incinerate", 16.5f)
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
            "meta melee flee check",
            {
                NextAction("flee", 39.0f)
            }
        )
    );
}

// Combat strategy to run to melee for Immolation Aura
// Enabled by default for the Demonology spec
// To enable, type "co +meta melee"
// To disable, type "co -meta melee"
MetaMeleeAoeStrategy::MetaMeleeAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void MetaMeleeAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "immolation aura active",
            {
                NextAction("reach melee", 25.5f),
                NextAction("demon charge", 25.0f)
            }
        )
    );
}
