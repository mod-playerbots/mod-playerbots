/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EnhancementShamanStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class EnhancementShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    EnhancementShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["feral spirit"] = &feral_spirit;
        creators["lightning bolt"] = &lightning_bolt;
        creators["earth shock"] = &earth_shock;
        creators["flame shock"] = &flame_shock;
        creators["shamanistic rage"] = &shamanistic_rage;
        creators["call of the elements"] = &call_of_the_elements;
        creators["lightning shield"] = &lightning_shield;
    }

private:
    static ActionNode* stormstrike(PlayerbotAI*) { return new ActionNode("stormstrike", {}, {}, {}); }
    static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "lava lash",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }
    static ActionNode* feral_spirit(PlayerbotAI*) { return new ActionNode("feral spirit", {}, {}, {}); }
    static ActionNode* lightning_bolt(PlayerbotAI*) { return new ActionNode("lightning bolt", {}, {}, {}); }
    static ActionNode* earth_shock(PlayerbotAI*) { return new ActionNode("earth shock", {}, {}, {}); }
    static ActionNode* flame_shock(PlayerbotAI*) { return new ActionNode("flame shock", {}, {}, {}); }
    static ActionNode* shamanistic_rage(PlayerbotAI*) { return new ActionNode("shamanistic rage", {}, {}, {}); }
    static ActionNode* call_of_the_elements(PlayerbotAI*) { return new ActionNode("call of the elements", {}, {}, {}); }
    static ActionNode* lightning_shield(PlayerbotAI*) { return new ActionNode("lightning shield", {}, {}, {}); }
};

// ===== Single Target Strategy =====
EnhancementShamanStrategy::EnhancementShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new EnhancementShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> EnhancementShamanStrategy::getDefaultActions()
{
    return {
       NextAction("stormstrike", 5.5f),
       NextAction("feral spirit", 5.4f),
       NextAction("earth shock", 5.3f),
       NextAction("lava lash", 5.2f),
       NextAction("melee", 5.0f)
    };
}

// ===== Trigger Initialization ===
void EnhancementShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Trigger
    triggers.push_back(
        new TriggerNode(
            "call of the elements and enemy within melee",
            {
                NextAction("call of the elements", 60.0f)
            }
        )
    );

    // Spirit Walk Trigger
    triggers.push_back(
        new TriggerNode(
            "spirit walk ready",
            {
                NextAction("spirit walk", 50.0f)
            }
        )
    );

    // Damage Triggers
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("reach melee", 40.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "maelstrom weapon 5",
            {
                NextAction("lightning bolt", 20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "maelstrom weapon 4",
            {
                NextAction("lightning bolt", 19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flame shock",
            {
                NextAction("flame shock", 19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lightning shield",
            {
                NextAction("lightning shield", 18.5f)
            }
        )
    );

    // Health/Mana Triggers
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                NextAction("shamanistic rage", 23.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("shamanistic rage", 23.0f)
            }
        )
    );
}
