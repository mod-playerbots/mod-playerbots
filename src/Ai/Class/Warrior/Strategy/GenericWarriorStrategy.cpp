/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericWarriorStrategy.h"

#include "Playerbots.h"

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{

}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "enemy out of melee", { NextAction("reach melee", ACTION_HIGH + 1) }));
}

class WarrirorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarrirorAoeStrategyActionNodeFactory()
    {

    }

private:

};

WarrirorAoeStrategy::WarrirorAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new WarrirorAoeStrategyActionNodeFactory());
}

void WarrirorAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "light aoe", { NextAction("sweeping strikes", ACTION_HIGH + 7),
                                       NextAction("bladestorm", ACTION_HIGH + 6),
                                       NextAction("thunder clap", ACTION_HIGH + 5),
                                       NextAction("shockwave", ACTION_HIGH + 4),
                                       NextAction("demoralizing shout without life time check", ACTION_HIGH + 1),
                                       NextAction("cleave", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("shockwave on snare target",
                        { NextAction("shockwave on snare target", ACTION_HIGH + 5) }));

}
