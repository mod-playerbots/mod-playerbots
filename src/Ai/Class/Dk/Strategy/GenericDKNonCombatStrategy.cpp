/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDKNonCombatStrategy.h"

class GenericDKNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKNonCombatStrategyActionNodeFactory()
    {
        creators["bone shield"] = &bone_shield;
        creators["horn of winter"] = &horn_of_winter;
    }

private:
    static ActionNode* bone_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("bone shield",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* horn_of_winter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("horn of winter",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
};

GenericDKNonCombatStrategy::GenericDKNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDKNonCombatStrategyActionNodeFactory());
}

void GenericDKNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("no pet", { NextAction("raise dead", ACTION_NORMAL + 1) }));
    triggers.push_back(
        new TriggerNode("horn of winter", { NextAction("horn of winter", 21.0f) }));
    triggers.push_back(
        new TriggerNode("bone shield", { NextAction("bone shield", 21.0f) }));
    triggers.push_back(
        new TriggerNode("has pet", { NextAction("toggle pet spell", 60.0f) }));
    triggers.push_back(
        new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));
}

void DKBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

}
