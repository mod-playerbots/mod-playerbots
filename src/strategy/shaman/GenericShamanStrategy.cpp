/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericShamanStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "AiFactory.h"

class GenericShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericShamanStrategyActionNodeFactory()
    {
        creators["totem of wrath"] = &totem_of_wrath;
        creators["flametongue totem"] = &flametongue_totem;
        creators["magma totem"] = &magma_totem;
        creators["searing totem"] = &searing_totem;
        creators["strength of earth totem"] = &strength_of_earth_totem;
        creators["stoneskin totem"] = &stoneskin_totem;
        creators["cleansing totem"] = &cleansing_totem;
        creators["mana spring totem"] = &mana_spring_totem;
        creators["healing stream totem"] = &healing_stream_totem;
        creators["wrath of air totem"] = &wrath_of_air_totem;
        creators["windfury totem"] = &windfury_totem;
        creators["grounding totem"] = &grounding_totem;
        creators["wind shear"] = &wind_shear;
        creators["purge"] = &purge;
    }

private:
    // Passthrough totems are set up so lower level shamans will still cast totems.
    // Totem of Wrath -> Flametongue Totem -> Searing Totem
    // Magma Totem -> Searing Totem
    // Strength of Earth Totem -> Stoneskin Totem
    // Cleansing Totem -> Mana Spring Totem
    // Wrath of Air Totem -> Windfury Totem -> Grounding Totem

    static ActionNode* totem_of_wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("totem of wrath",
                              /*P*/ {},
                              /*A*/ { new NextAction("flametongue totem") },
                              /*C*/ {});
    }
    static ActionNode* flametongue_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("searing totem") },
                              /*C*/ {});
    }
    static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("magma totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("searing totem") },
                              /*C*/ {});
    }
    static ActionNode* searing_totem(PlayerbotAI*) { return new ActionNode("searing totem", {}, {}, {}); }
    static ActionNode* strength_of_earth_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("strength of earth totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("stoneskin totem") },
                              /*C*/ {});
    }
    static ActionNode* stoneskin_totem(PlayerbotAI*) { return new ActionNode("stoneskin totem", {}, {}, {}); }
    static ActionNode* cleansing_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cleansing totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("mana spring totem") },
                              /*C*/ {});
    }
    static ActionNode* mana_spring_totem(PlayerbotAI*) { return new ActionNode("mana spring totem", {}, {}, {}); }
    static ActionNode* healing_stream_totem(PlayerbotAI*) { return new ActionNode("healing stream totem", {}, {}, {}); }
    static ActionNode* wrath_of_air_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wrath of air totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("windfury totem") },
                              /*C*/ {});
    }
    static ActionNode* windfury_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury totem",
                              /*P*/ {},
                              /*A*/ { new NextAction("grounding totem") },
                              /*C*/ {});
    }
    static ActionNode* grounding_totem(PlayerbotAI*) { return new ActionNode("grounding totem", {}, {}, {}); }
    static ActionNode* wind_shear(PlayerbotAI*) { return new ActionNode("wind shear", {}, {}, {}); }
    static ActionNode* purge(PlayerbotAI*) { return new ActionNode("purge", {}, {}, {}); }
};

GenericShamanStrategy::GenericShamanStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericShamanStrategyActionNodeFactory());
}

void GenericShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("wind shear", { new NextAction("wind shear", 23.0f), }));
    triggers.push_back(new TriggerNode("wind shear on enemy healer", { new NextAction("wind shear on enemy healer", 23.0f), }));
    triggers.push_back(new TriggerNode("purge", { new NextAction("purge", ACTION_DISPEL), }));
    triggers.push_back(new TriggerNode("medium mana", { new NextAction("mana potion", ACTION_DISPEL), }));
    triggers.push_back(new TriggerNode("new pet", { new NextAction("set pet stance", 65.0f), }));
}

void ShamanCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("cure poison", { new NextAction("cure poison", 21.0f), }));
    triggers.push_back(new TriggerNode("party member cure poison", { new NextAction("cure poison on party", 21.0f), }));
    triggers.push_back(new TriggerNode("cleanse spirit poison", { new NextAction("cleanse spirit", 24.0f), }));
    triggers.push_back(new TriggerNode("party member cleanse spirit poison", { new NextAction("cleanse spirit poison on party", 23.0f), }));
    triggers.push_back(new TriggerNode("cure disease", { new NextAction("cure disease", 31.0f), }));
    triggers.push_back(new TriggerNode("party member cure disease", { new NextAction("cure disease on party", 30.0f), }));
    triggers.push_back(new TriggerNode("cleanse spirit disease", { new NextAction("cleanse spirit", 24.0f), }));
    triggers.push_back(new TriggerNode("party member cleanse spirit disease", { new NextAction("cleanse spirit disease on party", 23.0f), }));
    triggers.push_back(new TriggerNode("cleanse spirit curse", { new NextAction("cleanse spirit", 24.0f), }));
    triggers.push_back(new TriggerNode("party member cleanse spirit curse", { new NextAction("cleanse spirit curse on party", 23.0f), }));
}

void ShamanBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("heroism", { new NextAction("heroism", 30.0f), }));
    triggers.push_back(new TriggerNode("bloodlust", { new NextAction("bloodlust", 30.0f), }));

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(new TriggerNode("fire elemental totem", { new NextAction("fire elemental totem", 23.0f), }));
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(new TriggerNode("fire elemental totem", { new NextAction("fire elemental totem melee", 24.0f), }));
    }
}

void ShamanAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(new TriggerNode("medium aoe",{ new NextAction("fire nova", 23.0f), }));
        triggers.push_back(new TriggerNode("chain lightning no cd", { new NextAction("chain lightning", 5.6f), }));
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(new TriggerNode("medium aoe",{
                                                    new NextAction("magma totem", 24.0f),
                                                    new NextAction("fire nova", 23.0f), }));

        triggers.push_back(new TriggerNode("maelstrom weapon 5 and medium aoe", { new NextAction("chain lightning", 22.0f), }));
        triggers.push_back(new TriggerNode("maelstrom weapon 4 and medium aoe", { new NextAction("chain lightning", 21.0f), }));
        triggers.push_back(new TriggerNode("enemy within melee", { new NextAction("fire nova", 5.1f), }));
    }
    else if (tab == 2)  // Restoration
    {
        // Handled by "Healer DPS" Strategy
    }
}
