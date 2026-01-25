/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RestoShamanStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class RestoShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RestoShamanStrategyActionNodeFactory()
    {
        creators["mana tide totem"] = &mana_tide_totem;
        creators["call of the elements"] = &call_of_the_elements;
        creators["stoneclaw totem"] = &stoneclaw_totem;
        creators["riptide on party"] = &riptide_on_party;
        creators["chain heal on party"] = &chain_heal_on_party;
        creators["healing wave on party"] = &healing_wave_on_party;
        creators["lesser healing wave on party"] = &lesser_healing_wave_on_party;
        creators["earth shield on main tank"] = &earth_shield_on_main_tank;
        creators["cleanse spirit poison on party"] = &cleanse_spirit_poison_on_party;
        creators["cleanse spirit disease on party"] = &cleanse_spirit_disease_on_party;
        creators["cleanse spirit curse on party"] = &cleanse_spirit_curse_on_party;
        creators["cleansing totem"] = &cleansing_totem;
        creators["water shield"] = &water_shield;
        creators["flame shock"] = &flame_shock;
        creators["lava burst"] = &lava_burst;
        creators["lightning bolt"] = &lightning_bolt;
        creators["chain lightning"] = &chain_lightning;
    }

private:
    static ActionNode* mana_tide_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mana tide totem",
                              /*P*/ {},
                              /*A*/ { NextAction("mana potion") },
                              /*C*/ {});
    }
    static ActionNode* call_of_the_elements(PlayerbotAI*) { return new ActionNode("call of the elements", {}, {}, {}); }
    static ActionNode* stoneclaw_totem(PlayerbotAI*) { return new ActionNode("stoneclaw totem", {}, {}, {}); }
    static ActionNode* riptide_on_party(PlayerbotAI*) { return new ActionNode("riptide on party", {}, {}, {}); }
    static ActionNode* chain_heal_on_party(PlayerbotAI*) { return new ActionNode("chain heal on party", {}, {}, {}); }
    static ActionNode* healing_wave_on_party(PlayerbotAI*) { return new ActionNode("healing wave on party", {}, {}, {}); }
    static ActionNode* lesser_healing_wave_on_party(PlayerbotAI*) { return new ActionNode("lesser healing wave on party", {}, {}, {}); }
    static ActionNode* earth_shield_on_main_tank(PlayerbotAI*) { return new ActionNode("earth shield on main tank", {}, {}, {}); }
    static ActionNode* cleanse_spirit_poison_on_party(PlayerbotAI*) { return new ActionNode("cleanse spirit poison on party", {}, {}, {}); }
    static ActionNode* cleanse_spirit_disease_on_party(PlayerbotAI*) { return new ActionNode("cleanse spirit disease on party", {}, {}, {}); }
    static ActionNode* cleanse_spirit_curse_on_party(PlayerbotAI*) { return new ActionNode("cleanse spirit curse on party", {}, {}, {}); }
    static ActionNode* cleansing_totem(PlayerbotAI*) { return new ActionNode("cleansing totem", {}, {}, {}); }
    static ActionNode* water_shield(PlayerbotAI*) { return new ActionNode("water shield", {}, {}, {}); }
    static ActionNode* flame_shock(PlayerbotAI*) { return new ActionNode("flame shock", {}, {}, {}); }
    static ActionNode* lava_burst(PlayerbotAI*) { return new ActionNode("lava burst", {}, {}, {}); }
    static ActionNode* lightning_bolt(PlayerbotAI*) { return new ActionNode("lightning bolt", {}, {}, {}); }
    static ActionNode* chain_lightning(PlayerbotAI*) { return new ActionNode("chain lightning", {}, {}, {}); }
};

// ===== Single Target Strategy =====
RestoShamanStrategy::RestoShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new RestoShamanStrategyActionNodeFactory());
}

// ===== Trigger Initialization ===
void RestoShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Triggers
    triggers.push_back(new TriggerNode("call of the elements", { NextAction("call of the elements", 60.0f) }));
    triggers.push_back(new TriggerNode("low health", { NextAction("stoneclaw totem", 40.0f) }));
    triggers.push_back(new TriggerNode("medium mana", { NextAction("mana tide totem", ACTION_HIGH + 5) }));

    // Healing Triggers
    triggers.push_back(new TriggerNode("group heal setting", {
                                                         NextAction("riptide on party", 27.0f),
                                                         NextAction("chain heal on party", 26.0f) }));

    triggers.push_back(new TriggerNode("party member critical health", {
                                                                   NextAction("riptide on party", 25.0f),
                                                                   NextAction("healing wave on party", 24.0f),
                                                                   NextAction("lesser healing wave on party", 23.0f) }));

    triggers.push_back(new TriggerNode("party member low health", {
                                                              NextAction("riptide on party", 19.0f),
                                                              NextAction("healing wave on party", 18.0f),
                                                              NextAction("lesser healing wave on party", 17.0f) }));

    triggers.push_back(new TriggerNode("party member medium health", {
                                                                 NextAction("riptide on party", 16.0f),
                                                                 NextAction("healing wave on party", 15.0f),
                                                                 NextAction("lesser healing wave on party", 14.0f) }));

    triggers.push_back(new TriggerNode("party member almost full health", {
                                                                      NextAction("riptide on party", 12.0f),
                                                                      NextAction("lesser healing wave on party", 11.0f) }));

    triggers.push_back(new TriggerNode("earth shield on main tank", { NextAction("earth shield on main tank", ACTION_HIGH + 7) }));

    // Dispel Triggers
    triggers.push_back(new TriggerNode("party member cleanse spirit poison", { NextAction("cleanse spirit poison on party", ACTION_DISPEL + 2) }));
    triggers.push_back(new TriggerNode("party member cleanse spirit disease", { NextAction("cleanse spirit disease on party", ACTION_DISPEL + 2) }));
    triggers.push_back(new TriggerNode("party member cleanse spirit curse",{ NextAction("cleanse spirit curse on party", ACTION_DISPEL + 2) }));

    // Range/Mana Triggers
    triggers.push_back(new TriggerNode("enemy too close for spell", { NextAction("flee", ACTION_MOVE + 9) }));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", { NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1) }));
    triggers.push_back(new TriggerNode("water shield", { NextAction("water shield", 19.5f) }));
}

void ShamanHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("healer should attack",
                                       { NextAction("flame shock", ACTION_DEFAULT + 0.2f),
                                                         NextAction("lava burst", ACTION_DEFAULT + 0.1f),
                                                         NextAction("lightning bolt", ACTION_DEFAULT) }));

    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        { NextAction("chain lightning", ACTION_DEFAULT + 0.3f) }));
}
