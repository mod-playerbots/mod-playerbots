/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DestructionWarlockStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class DestructionWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DestructionWarlockStrategyActionNodeFactory()
    {
        creators["immolate"] = &immolate;
        creators["conflagrate"] = &conflagrate;
        creators["chaos bolt"] = &chaos_bolt;
        creators["incinerate"] = &incinerate;
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption_on_attacker;
        creators["shadow bolt"] = &shadow_bolt;
        creators["shadowburn"] = &shadowburn;
        creators["life tap"] = &life_tap;
        creators["shadowfury"] = &shadowfury;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["seed of corruption on attacker"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
    }

private:
    static ActionNode* immolate(PlayerbotAI*) { return new ActionNode("immolate", {}, {}, {}); }
    static ActionNode* conflagrate(PlayerbotAI*) { return new ActionNode("conflagrate", {}, {}, {}); }
    static ActionNode* chaos_bolt(PlayerbotAI*) { return new ActionNode("chaos bolt", {}, {}, {}); }
    static ActionNode* incinerate(PlayerbotAI*) { return new ActionNode("incinerate", {}, {}, {}); }
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", {}, {}, {}); }
    static ActionNode* corruption_on_attacker(PlayerbotAI*) { return new ActionNode("corruption on attacker", {}, {}, {}); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", {}, {}, {}); }
    static ActionNode* shadowburn(PlayerbotAI*) { return new ActionNode("shadowburn", {}, {}, {}); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", {}, {}, {}); }
    static ActionNode* shadowfury(PlayerbotAI*) { return new ActionNode("shadowfury", {}, {}, {}); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", {}, {}, {}); }
    static ActionNode* seed_of_corruption(PlayerbotAI*) { return new ActionNode("seed of corruption", {}, {}, {}); }
    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*) { return new ActionNode("seed of corruption on attacker", {}, {}, {}); }
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", {}, {}, {}); }
};

// ===== Single Target Strategy =====
DestructionWarlockStrategy::DestructionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DestructionWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction*> DestructionWarlockStrategy::getDefaultActions()
{
    return {
       new NextAction("immolate", 5.9f),
       new NextAction("conflagrate", 5.8f),
       new NextAction("chaos bolt", 5.7f),
       new NextAction("incinerate", 5.6f),
       new NextAction("corruption", 5.3f),      // Note: Corruption and Shadow Bolt won't be used after the character learns Incinerate at level 64
       new NextAction("shadow bolt", 5.2f),
       new NextAction("shoot", 5.0f)
    };
}

// ===== Trigger Initialization ===
void DestructionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime + high priority cooldowns
    triggers.push_back(new TriggerNode("immolate", { new NextAction("immolate", 20.0f) }));
    triggers.push_back(new TriggerNode("conflagrate", { new NextAction("conflagrate", 19.5f) }));
    triggers.push_back(new TriggerNode("chaos bolt", { new NextAction("chaos bolt", 19.0f) }));

    // Note: Corruption won't be used after the character learns Incinerate at level 64
    triggers.push_back(new TriggerNode("corruption on attacker", { new NextAction("corruption on attacker", 5.5f) }));
    triggers.push_back(new TriggerNode("corruption", { new NextAction("corruption", 5.4f) }));

    // Shadowburn as execute if target is low HP
    triggers.push_back(new TriggerNode("target critical health", { new NextAction("shadowburn", 18.0f) }));

    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(new TriggerNode("life tap glyph buff", { new NextAction("life tap", 29.5f) }));
    triggers.push_back(new TriggerNode("life tap", { new NextAction("life tap", 5.1f) }));

    triggers.push_back(new TriggerNode("enemy too close for spell", { new NextAction("flee", 39.0f) }));
}
