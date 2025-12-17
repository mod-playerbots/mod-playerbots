/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostMageStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class FrostMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostMageStrategyActionNodeFactory()
    {
        creators["cold snap"] = &cold_snap;
        creators["ice barrier"] = &ice_barrier;
        creators["summon water elemental"] = &summon_water_elemental;
        creators["deep freeze"] = &deep_freeze;
        creators["icy veins"] = &icy_veins;
        creators["frostbolt"] = &frostbolt;
        creators["ice lance"] = &ice_lance;
        creators["fire blast"] = &fire_blast;
        creators["fireball"] = &fireball;
        creators["frostfire bolt"] = &frostfire_bolt;
    }

private:
    static ActionNode* cold_snap(PlayerbotAI*) { return new ActionNode("cold snap", {}, {}, {}); }
    static ActionNode* ice_barrier(PlayerbotAI*) { return new ActionNode("ice barrier", {}, {}, {}); }
    static ActionNode* summon_water_elemental(PlayerbotAI*) { return new ActionNode("summon water elemental", {}, {}, {}); }
    static ActionNode* deep_freeze(PlayerbotAI*) { return new ActionNode("deep freeze", {}, {}, {}); }
    static ActionNode* icy_veins(PlayerbotAI*) { return new ActionNode("icy veins", {}, {}, {}); }
    static ActionNode* frostbolt(PlayerbotAI*) { return new ActionNode("frostbolt", {}, {}, {}); }
    static ActionNode* ice_lance(PlayerbotAI*) { return new ActionNode("ice lance", {}, {}, {}); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", {}, {}, {}); }
    static ActionNode* fireball(PlayerbotAI*) { return new ActionNode("fireball", {}, {}, {}); }
    static ActionNode* frostfire_bolt(PlayerbotAI*) { return new ActionNode("frostfire bolt", {}, {}, {}); }
};

// ===== Single Target Strategy =====
FrostMageStrategy::FrostMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction*> FrostMageStrategy::getDefaultActions()
{
    return {
        new NextAction("frostbolt", 5.4f),
        new NextAction("ice lance", 5.3f),   // cast during movement
        new NextAction("fire blast", 5.2f),  // cast during movement if ice lance is not learned
        new NextAction("shoot", 5.1f),
        new NextAction("fireball", 5.0f)
    };
}

// ===== Trigger Initialization ===
void FrostMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Pet/Defensive triggers
    triggers.push_back(new TriggerNode("no pet", { new NextAction("summon water elemental", 30.0f) }));
    triggers.push_back(new TriggerNode("has pet", { new NextAction("toggle pet spell", 60.0f) }));
    triggers.push_back(new TriggerNode("new pet", { new NextAction("set pet stance", 60.0f) }));
    triggers.push_back(new TriggerNode("medium health", { new NextAction("ice barrier", 29.0f) }));
    triggers.push_back(new TriggerNode("being attacked", { new NextAction("ice barrier", 29.0f) }));

    // Proc/Freeze triggers
    triggers.push_back(new TriggerNode("brain freeze", { new NextAction("frostfire bolt", 19.5f) }));
    triggers.push_back(new TriggerNode("fingers of frost", {
                                                       new NextAction("deep freeze", 19.0f),
                                                       new NextAction("frostbolt", 18.0f) }));

    triggers.push_back(new TriggerNode("frostbite on target", {
                                                          new NextAction("deep freeze", 19.0f),
                                                          new NextAction("frostbolt", 18.0f) }));

    triggers.push_back(new TriggerNode("frost nova on target", {
                                                           new NextAction("deep freeze", 19.0f),
                                                           new NextAction("frostbolt", 18.0f) }));

}
