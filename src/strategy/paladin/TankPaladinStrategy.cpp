/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankPaladinStrategy.h"

#include "Playerbots.h"

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankPaladinStrategyActionNodeFactory()
    {
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["taunt spell"] = &hand_of_reckoning;
    }

private:
    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of command",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of corruption") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of corruption",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of vengeance") },
                              /*C*/ {});
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of vengeance",
                              /*P*/ {},
                              /*A*/ { new NextAction("seal of righteousness") },
                              /*C*/ {});
    }
    ACTION_NODE_A(hand_of_reckoning, "hand of reckoning", "righteous defense");
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

std::vector<NextAction*> TankPaladinStrategy::getDefaultActions()
{
    return { new NextAction("shield of righteousness", ACTION_DEFAULT + 0.6f),
                             new NextAction("hammer of the righteous", ACTION_DEFAULT + 0.5f),
                             new NextAction("judgement of wisdom", ACTION_DEFAULT + 0.4f),
                             new NextAction("melee", ACTION_DEFAULT) };
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("seal", { new NextAction("seal of corruption", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("low mana", { new NextAction("seal of wisdom", ACTION_HIGH + 9) }));
    triggers.push_back(new TriggerNode(
        "light aoe", { new NextAction("avenger's shield", ACTION_HIGH + 5) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { new NextAction("consecration", ACTION_HIGH + 7),
                                                        new NextAction("avenger's shield", ACTION_HIGH + 6) }));
    triggers.push_back(new TriggerNode(
        "lose aggro", { new NextAction("hand of reckoning", ACTION_HIGH + 7) }));
    triggers.push_back(new TriggerNode("medium health",
                                       { new NextAction("holy shield", ACTION_HIGH + 4) }));
    triggers.push_back(
        new TriggerNode("low health", { new NextAction("holy shield", ACTION_HIGH + 4) }));
    triggers.push_back(new TriggerNode("critical health",
                                       { new NextAction("holy shield", ACTION_HIGH + 4) }));
    triggers.push_back(new TriggerNode(
        "avenging wrath", { new NextAction("avenging wrath", ACTION_HIGH + 2) }));
    triggers.push_back(
        new TriggerNode("target critical health",
                        { new NextAction("hammer of wrath", ACTION_CRITICAL_HEAL) }));
    triggers.push_back(new TriggerNode(
        "righteous fury", { new NextAction("righteous fury", ACTION_HIGH + 8) }));
    triggers.push_back(
        new TriggerNode("medium group heal setting",
                        { new NextAction("divine sacrifice", ACTION_HIGH + 5) }));
    triggers.push_back(new TriggerNode(
        "enough mana", { new NextAction("consecration", ACTION_HIGH + 4) }));
    triggers.push_back(new TriggerNode("not facing target",
                                       { new NextAction("set facing", ACTION_NORMAL + 7) }));
    triggers.push_back(new TriggerNode(
        "enemy out of melee", { new NextAction("reach melee", ACTION_HIGH + 1) }));
}
