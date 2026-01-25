/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DemonologyWarlockStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class DemonologyWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DemonologyWarlockStrategyActionNodeFactory()
    {
        creators["metamorphosis"] = &metamorphosis;
        creators["demonic empowerment"] = &demonic_empowerment;
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption_on_attacker;
        creators["immolate"] = &immolate;
        creators["immolate on attacker"] = &immolate_on_attacker;
        creators["incinerate"] = &incinerate;
        creators["soul fire"] = &soul_fire;
        creators["shadow bolt"] = &shadow_bolt;
        creators["life tap"] = &life_tap;
        creators["immolation aura"] = &immolation_aura;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption on attacker"] = &seed_of_corruption_on_attacker;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
        creators["demon charge"] = &demon_charge;
    }

private:
    static ActionNode* metamorphosis(PlayerbotAI*) { return new ActionNode("metamorphosis", {}, {}, {}); }
    static ActionNode* demonic_empowerment(PlayerbotAI*) { return new ActionNode("demonic empowerment", {}, {}, {}); }
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", {}, {}, {}); }
    static ActionNode* corruption_on_attacker(PlayerbotAI*) { return new ActionNode("corruption on attacker", {}, {}, {}); }
    static ActionNode* immolate(PlayerbotAI*) { return new ActionNode("immolate", {}, {}, {}); }
    static ActionNode* immolate_on_attacker(PlayerbotAI*) { return new ActionNode("immolate on attacker", {}, {}, {}); }
    static ActionNode* incinerate(PlayerbotAI*) { return new ActionNode("incinerate", {}, {}, {}); }
    static ActionNode* soul_fire(PlayerbotAI*) { return new ActionNode("soul fire", {}, {}, {}); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", {}, {}, {}); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", {}, {}, {}); }
    static ActionNode* immolation_aura(PlayerbotAI*) { return new ActionNode("immolation aura", {}, {}, {}); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", {}, {}, {}); }
    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*) { return new ActionNode("seed of corruption on attacker", {}, {}, {}); }
    static ActionNode* seed_of_corruption(PlayerbotAI*) { return new ActionNode("seed of corruption", {}, {}, {}); }
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", {}, {}, {}); }
    static ActionNode* demon_charge(PlayerbotAI*) { return new ActionNode("demon charge", {}, {}, {}); }
};

// ===== Single Target Strategy =====
DemonologyWarlockStrategy::DemonologyWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DemonologyWarlockStrategyActionNodeFactory());
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
