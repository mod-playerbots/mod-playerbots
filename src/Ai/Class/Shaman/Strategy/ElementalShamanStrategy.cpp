/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ElementalShamanStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class ElementalShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ElementalShamanStrategyActionNodeFactory()
    {
        creators["flame shock"] = &flame_shock;
        creators["earth shock"] = &earth_shock;
        creators["lava burst"] = &lava_burst;
        creators["lightning bolt"] = &lightning_bolt;
        creators["call of the elements"] = &call_of_the_elements;
        creators["elemental mastery"] = &elemental_mastery;
        creators["stoneclaw totem"] = &stoneclaw_totem;
        creators["water shield"] = &water_shield;
        creators["thunderstorm"] = &thunderstorm;
    }

private:
    static ActionNode* flame_shock(PlayerbotAI*) { return new ActionNode("flame shock", {}, {}, {}); }
    static ActionNode* earth_shock(PlayerbotAI*) { return new ActionNode("earth shock", {}, {}, {}); }
    static ActionNode* lava_burst(PlayerbotAI*) { return new ActionNode("lava burst", {}, {}, {}); }
    static ActionNode* lightning_bolt(PlayerbotAI*) { return new ActionNode("lightning bolt", {}, {}, {}); }
    static ActionNode* call_of_the_elements(PlayerbotAI*) { return new ActionNode("call of the elements", {}, {}, {}); }
    static ActionNode* elemental_mastery(PlayerbotAI*) { return new ActionNode("elemental mastery", {}, {}, {}); }
    static ActionNode* stoneclaw_totem(PlayerbotAI*) { return new ActionNode("stoneclaw totem", {}, {}, {}); }
    static ActionNode* water_shield(PlayerbotAI*) { return new ActionNode("water shield", {}, {}, {}); }
    static ActionNode* thunderstorm(PlayerbotAI*) { return new ActionNode("thunderstorm", {}, {}, {}); }
};

// ===== Single Target Strategy =====
ElementalShamanStrategy::ElementalShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new ElementalShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> ElementalShamanStrategy::getDefaultActions()
{
    return {
        NextAction("lava burst", 5.2f),
        NextAction("lightning bolt", 5.0f)
    };
}

// ===== Trigger Initialization ===
void ElementalShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Triggers
    triggers.push_back(
        new TriggerNode(
            "call of the elements",
            {
                NextAction("call of the elements", 60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("stoneclaw totem", 40.0f)
            }
        )
    );

    // Cooldown Trigger
    triggers.push_back(
        new TriggerNode(
            "elemental mastery",
            {
                NextAction("elemental mastery", 29.0f)
            }
        )
    );

    // Damage Triggers
    triggers.push_back(
        new TriggerNode(
            "earth shock execute",
            {
                NextAction("earth shock", 5.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flame shock",
            {
                NextAction("flame shock", 5.3f)
            }
        )
    );

    // Mana Triggers
    triggers.push_back(
        new TriggerNode(
            "water shield",
            {
                NextAction("water shield", 19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high mana",
            {
                NextAction("thunderstorm", 19.0f)
            }
        )
    );

    // Range Triggers
    triggers.push_back(
        new TriggerNode(
            "enemy is close",
            {
                NextAction("thunderstorm", 19.0f)
            }
        )
    );
}
