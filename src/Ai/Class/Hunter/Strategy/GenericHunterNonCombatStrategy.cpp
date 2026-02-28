/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterNonCombatStrategy.h"

#include "Playerbots.h"

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterNonCombatStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
    }

private:
    static ActionNode* rapid_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rapid fire",
                              /*P*/ {},
                              /*A*/ { NextAction("readiness")},
                              /*C*/ {});
    }

    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the cheetah")},
                              /*C*/ {});
    }
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("trueshot aura", { NextAction("trueshot aura", 2.0f)}));
    triggers.push_back(new TriggerNode("often", {
                       NextAction("apply stone", 1.0f),
                       NextAction("apply oil", 1.0f),
                       }));
    triggers.push_back(new TriggerNode("low ammo", { NextAction("say::low ammo", ACTION_NORMAL)}));
    triggers.push_back(new TriggerNode("no track", { NextAction("track humanoids", ACTION_NORMAL)}));
    triggers.push_back(new TriggerNode("no ammo", { NextAction("equip upgrades packet action", ACTION_HIGH + 1)}));
}

void HunterPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("call pet", 60.0f)}));
    triggers.push_back(new TriggerNode("has pet", { NextAction("toggle pet spell", 60.0f)}));
    triggers.push_back(new TriggerNode("new pet", { NextAction("set pet stance", 60.0f)}));
    triggers.push_back(new TriggerNode("pet not happy", { NextAction("feed pet", 60.0f)}));
    triggers.push_back(new TriggerNode("hunters pet medium health", { NextAction("mend pet", 60.0f)}));
    triggers.push_back(new TriggerNode("hunters pet dead", { NextAction("revive pet", 60.0f)}));
}
