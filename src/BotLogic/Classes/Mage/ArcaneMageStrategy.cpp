/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ArcaneMageStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArcaneMageStrategyActionNodeFactory()
    {
        creators["arcane blast"] = &arcane_blast;
        creators["arcane barrage"] = &arcane_barrage;
        creators["arcane missiles"] = &arcane_missiles;
        creators["fire blast"] = &fire_blast;
        creators["frostbolt"] = &frostbolt;
        creators["arcane power"] = &arcane_power;
        creators["icy veins"] = &icy_veins;
    }

private:
    static ActionNode* arcane_blast(PlayerbotAI*) { return new ActionNode("arcane blast", {}, {}, {}); }
    static ActionNode* arcane_barrage(PlayerbotAI*) { return new ActionNode("arcane barrage", {}, {}, {}); }
    static ActionNode* arcane_missiles(PlayerbotAI*) { return new ActionNode("arcane missiles", {}, {}, {}); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", {}, {}, {}); }
    static ActionNode* frostbolt(PlayerbotAI*) { return new ActionNode("frostbolt", {}, {}, {}); }
    static ActionNode* arcane_power(PlayerbotAI*) { return new ActionNode("arcane power", {}, {}, {}); }
    static ActionNode* icy_veins(PlayerbotAI*) { return new ActionNode("icy veins", {}, {}, {}); }
};

// ===== Single Target Strategy =====
ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new ArcaneMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> ArcaneMageStrategy::getDefaultActions()
{
    return {
        NextAction("arcane blast", 5.6f),
        NextAction("arcane missiles", 5.5f),
        NextAction("arcane barrage", 5.4f),   // cast while moving
        NextAction("fire blast", 5.3f),       // cast while moving if arcane barrage isn't available/learned
        NextAction("frostbolt", 5.2f),        // for arcane immune targets
        NextAction("shoot", 5.1f)
    };
}

// ===== Trigger Initialization ===
void ArcaneMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Proc Trigger
    triggers.push_back(
        new TriggerNode(
            "arcane blast 4 stacks and missile barrage",
            {
                NextAction("arcane missiles", 15.0f)
            }
        )
    );
}
