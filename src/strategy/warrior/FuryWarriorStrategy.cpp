/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FuryWarriorStrategy.h"

#include "Playerbots.h"

class FuryWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FuryWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["intercept"] = &intercept;
        // creators["death wish"] = &death_wish;
        creators["piercing howl"] = &piercing_howl;
        // creators["bloodthirst"] = &bloodthirst;
        creators["pummel"] = &pummel;
        creators["enraged regeneration"] = &enraged_regeneration;
    }

private:
    ACTION_NODE_A(charge, "charge", "intercept");
    ACTION_NODE_A(intercept, "intercept", "reach melee");
    ACTION_NODE_A(piercing_howl, "piercing howl", "hamstring");
    // ACTION_NODE_A(death_wish, "death wish", "berserker rage");
    // ACTION_NODE_A(bloodthirst, "bloodthirst", "melee");
    ACTION_NODE_A(pummel, "pummel", "intercept");

    static ActionNode* enraged_regeneration(PlayerbotAI* botAI)
    {
        return new ActionNode("enraged regeneration",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
};

FuryWarriorStrategy::FuryWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new FuryWarriorStrategyActionNodeFactory());
}

std::vector<NextAction*> FuryWarriorStrategy::getDefaultActions()
{
    return {
        new NextAction("bloodthirst", ACTION_DEFAULT + 0.5f), new NextAction("whirlwind", ACTION_DEFAULT + 0.4f),
        new NextAction("sunder armor", ACTION_DEFAULT + 0.3f), new NextAction("execute", ACTION_DEFAULT + 0.2f),
        // new NextAction("overpower", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT)
    };
}

void FuryWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of melee",
                                       { new NextAction("charge", ACTION_MOVE + 9) }));
    triggers.push_back(new TriggerNode(
        "berserker stance", { new NextAction("berserker stance", ACTION_HIGH + 9) }));
    triggers.push_back(new TriggerNode("battle shout",
                                       { new NextAction("battle shout", ACTION_HIGH + 8) }));
    triggers.push_back(
        new TriggerNode("pummel on enemy healer",
                        { new NextAction("pummel on enemy healer", ACTION_INTERRUPT) }));
    triggers.push_back(
        new TriggerNode("pummel", { new NextAction("pummel", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "victory rush", { new NextAction("victory rush", ACTION_INTERRUPT) }));
    triggers.push_back(
        new TriggerNode("bloodthirst", { new NextAction("bloodthirst", ACTION_HIGH + 7) }));
    triggers.push_back(
        new TriggerNode("whirlwind", { new NextAction("whirlwind", ACTION_HIGH + 6) }));
    triggers.push_back(
        new TriggerNode("instant slam", { new NextAction("slam", ACTION_HIGH + 5) }));
    triggers.push_back(
        new TriggerNode("bloodrage", { new NextAction("bloodrage", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode("medium rage available",
                                       { new NextAction("heroic strike", ACTION_DEFAULT + 0.1f) }));

    triggers.push_back(
        new TriggerNode("death wish", { new NextAction("death wish", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("recklessness", { new NextAction("recklessness", ACTION_HIGH) }));
    triggers.push_back(new TriggerNode("critical health",
        { new NextAction("enraged regeneration", ACTION_EMERGENCY) }));
}
