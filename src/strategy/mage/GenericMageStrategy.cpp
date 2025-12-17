/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericMageStrategy.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RangedCombatStrategy.h"

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["ice lance"] = &ice_lance;
        creators["fire blast"] = &fire_blast;
        creators["scorch"] = &scorch;
        creators["frost nova"] = &frost_nova;
        creators["cone of cold"] = &cone_of_cold;
        creators["icy veins"] = &icy_veins;
        creators["combustion"] = &combustion;
        creators["evocation"] = &evocation;
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["fireball"] = &fireball;
    }

private:
    static ActionNode* frostbolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbolt",
                              /*P*/ {},
                              /*A*/ { new NextAction("shoot") },
                              /*C*/ {});
    }

    static ActionNode* frostfire_bolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostfire bolt",
                              /*P*/ {},
                              /*A*/ { new NextAction("fireball") },
                              /*C*/ {});
    }

    static ActionNode* ice_lance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice lance",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* fire_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fire blast",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* scorch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("scorch",
                              /*P*/ {},
                              /*A*/ { new NextAction("shoot") },
                              /*C*/ {});
    }

    static ActionNode* frost_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost nova",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cone_of_cold([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cone of cold",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* icy_veins([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy veins",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* combustion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("combustion",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* evocation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("evocation",
                              /*P*/ {},
                              /*A*/ { new NextAction("mana potion") },
                              /*C*/ {});
    }

    static ActionNode* dragons_breath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("dragon's breath",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* blast_wave([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("blast wave",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* remove_curse([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse",
                              /*P*/ {},
                              /*A*/ { new NextAction("remove lesser curse") },
                              /*C*/ {});
    }

    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse on party",
                              /*P*/ {},
                              /*A*/ { new NextAction("remove lesser curse on party") },
                              /*C*/ {});
    }
    static ActionNode* fireball([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fireball",
                              /*P*/ {},
                              /*A*/ { new NextAction("shoot") },
                              /*C*/ {});
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    // Threat Triggers
    triggers.push_back(new TriggerNode("high threat", { new NextAction("mirror image", 60.0f) }));
    triggers.push_back(new TriggerNode("medium threat", { new NextAction("invisibility", 30.0f) }));

    // Defensive Triggers
    triggers.push_back(new TriggerNode("critical health", { new NextAction("ice block", 90.0f) }));
    triggers.push_back(new TriggerNode("low health", { new NextAction("mana shield", 85.0f) }));
    triggers.push_back(new TriggerNode("fire ward", { new NextAction("fire ward", 90.0f) }));
    triggers.push_back(new TriggerNode("frost ward", { new NextAction("frost ward", 90.0f) }));
    triggers.push_back(new TriggerNode("enemy is close and no firestarter strategy", { new NextAction("frost nova", 50.0f) }));
    triggers.push_back(new TriggerNode("enemy too close for spell and no firestarter strategy", { new NextAction("blink back", 35.0f) }));

    // Mana Threshold Triggers
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(42985))  // Mana Sapphire
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana sapphire", 90.0f) }));
    else if (bot->HasSpell(27101))  // Mana Emerald
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana emerald", 90.0f) }));
    else if (bot->HasSpell(10054))  // Mana Ruby
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana ruby", 90.0f) }));
    else if (bot->HasSpell(10053))  // Mana Citrine
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana citrine", 90.0f) }));
    else if (bot->HasSpell(3552))  // Mana Jade
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana jade", 90.0f) }));
    else if (bot->HasSpell(759))  // Mana Agate
        triggers.push_back(new TriggerNode("high mana", { new NextAction("use mana agate", 90.0f) }));

    triggers.push_back(new TriggerNode("medium mana", { new NextAction("mana potion", 90.0f) }));
    triggers.push_back(new TriggerNode("low mana", { new NextAction("evocation", 90.0f) }));

    // Counterspell / Spellsteal Triggers
    triggers.push_back(new TriggerNode("spellsteal", { new NextAction("spellsteal", 40.0f) }));
    triggers.push_back(new TriggerNode("counterspell on enemy healer", { new NextAction("counterspell on enemy healer", 40.0f) }));
}

void MageCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("remove curse", { new NextAction("remove curse", 41.0f) }));
    triggers.push_back(new TriggerNode("remove curse on party", { new NextAction("remove curse on party", 40.0f) }));
}

void MageBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Arcane
    {
        triggers.push_back(new TriggerNode("arcane power", { new NextAction("arcane power", 29.0f) }));
        triggers.push_back(new TriggerNode("icy veins", { new NextAction("icy veins", 28.5f) }));
        triggers.push_back(new TriggerNode("mirror image", { new NextAction("mirror image", 28.0f) }));
    }
    else if (tab == 1)
    {
        if (bot->HasSpell(44614) /*Frostfire Bolt*/ && bot->HasAura(15047) /*Ice Shards*/)
        { // Frostfire
            triggers.push_back(new TriggerNode("combustion", { new NextAction("combustion", 18.0f) }));
            triggers.push_back(new TriggerNode("icy veins", { new NextAction("icy veins", 17.5f) }));
            triggers.push_back(new TriggerNode("mirror image", { new NextAction("mirror image", 17.0f) }));
        }
        else
        { // Fire
            triggers.push_back(new TriggerNode("combustion", { new NextAction("combustion", 18.0f) }));
            triggers.push_back(new TriggerNode("mirror image", { new NextAction("mirror image", 17.5f) }));
        }
    }
    else if (tab == 2)  // Frost
    {
        triggers.push_back(new TriggerNode("cold snap", { new NextAction("cold snap", 28.0f) }));
        triggers.push_back(new TriggerNode("icy veins", { new NextAction("icy veins", 27.5f) }));
        triggers.push_back(new TriggerNode("mirror image", { new NextAction("mirror image", 26.0f) }));
    }
}

void MageCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("polymorph", { new NextAction("polymorph", 30.0f) }));
}

void MageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("blizzard channel check", { new NextAction("cancel channel", 26.0f) }));

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Arcane
    {
        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { new NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("medium aoe", {
                                                     new NextAction("flamestrike", 23.0f),
                                                     new NextAction("blizzard", 22.0f) }));
        triggers.push_back(new TriggerNode("light aoe", { new NextAction("arcane explosion", 21.0f) }));
    }
    else if (tab == 1)  // Fire and Frostfire
    {
        triggers.push_back(
            new TriggerNode("medium aoe", {
                                      new NextAction("dragon's breath", 39.0f),
                                      new NextAction("blast wave", 38.0f),
                                      new NextAction("flamestrike", 23.0f),
                                      new NextAction("blizzard", 22.0f) }));

        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { new NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("firestarter", { new NextAction("flamestrike", 40.0f) }));
        triggers.push_back(new TriggerNode("living bomb on attackers", { new NextAction("living bomb on attackers", 21.0f) }));
    }
    else if (tab == 2)  // Frost
    {
        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { new NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("medium aoe", {
                                                     new NextAction("flamestrike", 23.0f),
                                                     new NextAction("blizzard", 22.0f) }));
        triggers.push_back(new TriggerNode("light aoe", { new NextAction("cone of cold", 21.0f) }));
    }
}
