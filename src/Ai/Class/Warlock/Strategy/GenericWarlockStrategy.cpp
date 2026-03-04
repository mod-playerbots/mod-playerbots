/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericWarlockStrategy.h"

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["banish on cc"] = &banish_on_cc;
        creators["fear on cc"] = &fear_on_cc;
        creators["spell lock"] = &spell_lock;
        creators["devour magic purge"] = &devour_magic_purge;
        creators["devour magic cleanse"] = &devour_magic_cleanse;
    }

private:
    static ActionNode* banish_on_cc(PlayerbotAI*) { return new ActionNode("banish on cc", {}, {}, {}); }
    static ActionNode* fear_on_cc(PlayerbotAI*) { return new ActionNode("fear on cc", {}, {}, {}); }
    static ActionNode* spell_lock(PlayerbotAI*) { return new ActionNode("spell lock", {}, {}, {}); }
    static ActionNode* devour_magic_purge(PlayerbotAI*) { return new ActionNode("devour magic purge", {}, {}, {}); }
    static ActionNode* devour_magic_cleanse(PlayerbotAI*) { return new ActionNode("devour magic cleanse", {}, {}, {}); }
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

std::vector<NextAction> GenericWarlockStrategy::getDefaultActions()
{
    return {};
}

void GenericWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("life tap", 95.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                NextAction("soulshatter", 55.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "spell lock",
            {
                NextAction("spell lock", 40.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no soul shard",
            {
                NextAction("create soul shard", 60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "too many soul shards",
            {
                NextAction("destroy soul shard", 60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "devour magic purge",
            {
                NextAction("devour magic purge", 50.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "devour magic cleanse",
            {
                NextAction("devour magic cleanse", 50.0f)
            }
        )
    );
}

// ===== AoE Strategy, 3+ enemies =====

void AoEWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("immolation aura", 26.0f),
                NextAction("shadowfury", 23.0f),
                NextAction("shadowflame", 22.5f),
                NextAction("seed of corruption on attacker", 22.0f),
                NextAction("seed of corruption", 21.5f),
                NextAction("rain of fire", 21.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode("rain of fire channel check",
            {
                NextAction("cancel channel", 21.5f)
            }
        )
    );
}

void WarlockBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Placeholder for future boost triggers
}

void WarlockPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Placeholder for future pet triggers
}

void WarlockCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "banish",
            {
                NextAction("banish on cc", 33.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "fear",
            {
                NextAction("fear on cc", 32.0f)
            }
        )
    );
}

// Combat strategy for using Curse of Agony
// Enabled by default for the Affliction spec
// To enable, type "co +curse of agony"
// To disable, type "co -curse of agony"
void WarlockCurseOfAgonyStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of agony on attacker",
            {
                NextAction("curse of agony on attacker", 18.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "curse of agony",
            {
                NextAction("curse of agony", 17.0f)
            }
        )
    );
}

// Combat strategy for using Curse of the Elements
// Enabled by default for the Destruction spec
// To enable, type "co +curse of elements"
// To disable, type "co -curse of elements"
void WarlockCurseOfTheElementsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of the elements",
            {
                NextAction("curse of the elements", 29.0f)
            }
        )
    );
}

// Combat strategy for using Curse of Doom
// Disabled by default
// To enable, type "co +curse of doom"
// To disable, type "co -curse of doom"
void WarlockCurseOfDoomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of doom",
            {
                NextAction("curse of doom", 29.0f)
            }
        )
    );
}

// Combat strategy for using Curse of Exhaustion
// Disabled by default
// To enable, type "co +curse of exhaustion"
// To disable, type "co -curse of exhaustion"
void WarlockCurseOfExhaustionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of exhaustion",
            {
                NextAction("curse of exhaustion", 29.0f)
            }
        )
    );
}

// Combat strategy for using Curse of Tongues
// Disabled by default
// To enable, type "co +curse of tongues"
// To disable, type "co -curse of tongues"
void WarlockCurseOfTonguesStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of tongues",
            {
                NextAction("curse of tongues", 29.0f)
            }
        )
    );
}

// Combat strategy for using Curse of Weakness
// Disabled by default
// To enable, type "co +curse of weakness"
// To disable, type "co -curse of weakness"
void WarlockCurseOfWeaknessStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "curse of weakness",
            {
                NextAction("curse of weakness", 29.0f)
            }
        )
    );
}
