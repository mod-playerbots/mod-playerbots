/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ArmsWarriorStrategy.h"

#include "Playerbots.h"

class ArmsWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArmsWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["death wish"] = &death_wish;
        creators["piercing howl"] = &piercing_howl;
        creators["mocking blow"] = &mocking_blow;
        creators["heroic strike"] = &heroic_strike;
        creators["enraged regeneration"] = &enraged_regeneration;
        creators["retaliation"] = &retaliation;
        creators["shattering throw"] = &shattering_throw;
    }

private:
    ACTION_NODE_A(charge, "charge", "reach melee");
    ACTION_NODE_A(death_wish, "death wish", "bloodrage");
    ACTION_NODE_A(piercing_howl, "piercing howl", "mocking blow");
    ACTION_NODE_A(mocking_blow, "mocking blow", "hamstring");
    ACTION_NODE_A(heroic_strike, "heroic strike", "melee");

    static ActionNode* enraged_regeneration(PlayerbotAI* botAI)
    {
        return new ActionNode("enraged regeneration",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* retaliation(PlayerbotAI* botAI)
    {
        return new ActionNode("retaliation",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* shattering_throw(PlayerbotAI* botAI)
    {
        return new ActionNode("shattering throw",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
};

ArmsWarriorStrategy::ArmsWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new ArmsWarriorStrategyActionNodeFactory());
}

std::vector<NextAction*> ArmsWarriorStrategy::getDefaultActions()
{
    return { new NextAction("bladestorm", ACTION_DEFAULT + 0.2f),
                            new NextAction("mortal strike", ACTION_DEFAULT + 0.1f),
                            new NextAction("melee", ACTION_DEFAULT) };
}

void ArmsWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of melee",
        { new NextAction("charge", ACTION_MOVE + 10) }));

    triggers.push_back(new TriggerNode("battle stance",
        { new NextAction("battle stance", ACTION_HIGH + 10) }));

    triggers.push_back(new TriggerNode("battle shout",
        { new NextAction("battle shout", ACTION_HIGH + 9) }));

    triggers.push_back(new TriggerNode("rend",
        { new NextAction("rend", ACTION_HIGH + 8) }));

    triggers.push_back(new TriggerNode("rend on attacker",
        { new NextAction("rend on attacker", ACTION_HIGH + 8) }));

    triggers.push_back(new TriggerNode("mortal strike",
        { new NextAction("mortal strike", ACTION_HIGH + 3) }));

    triggers.push_back(new TriggerNode("target critical health",
        { new NextAction("execute", ACTION_HIGH + 5) }));

    triggers.push_back(new TriggerNode("sudden death",
        { new NextAction("execute", ACTION_HIGH + 5) }));

    triggers.push_back(new TriggerNode("hamstring",
        { new NextAction("piercing howl", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("overpower",
        { new NextAction("overpower", ACTION_HIGH + 4) }));

    triggers.push_back(new TriggerNode("taste for blood",
        { new NextAction("overpower", ACTION_HIGH + 4) }));

    triggers.push_back(new TriggerNode("victory rush",
        { new NextAction("victory rush", ACTION_INTERRUPT) }));

    triggers.push_back(new TriggerNode("high rage available",
        { new NextAction("heroic strike", ACTION_HIGH),
                             new NextAction("slam", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("bloodrage", { new NextAction("bloodrage", ACTION_HIGH + 2) }));

    triggers.push_back(
        new TriggerNode("death wish", { new NextAction("death wish", ACTION_HIGH + 2) }));

    triggers.push_back(new TriggerNode("critical health",
        { new NextAction("intimidating shout", ACTION_EMERGENCY) }));

    triggers.push_back(new TriggerNode("medium health",
        { new NextAction("enraged regeneration", ACTION_EMERGENCY) }));

    triggers.push_back(new TriggerNode("almost full health",
        { new NextAction("retaliation", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("shattering throw trigger",
        { new NextAction("shattering throw", ACTION_INTERRUPT + 1) }));

}
