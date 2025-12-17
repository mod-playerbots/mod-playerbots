/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankWarriorStrategy.h"

#include "Playerbots.h"

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["sunder armor"] = &sunder_armor;
        creators["commanding shout"] = &commanding_shout;
        // creators["shield slam"] = &shield_slam;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
        creators["heroic throw taunt"] = &heroic_throw_taunt;
        creators["taunt"] = &taunt;
        creators["taunt spell"] = &taunt;
        creators["vigilance"] = &vigilance;
        creators["enraged regeneration"] = &enraged_regeneration;
    }

private:
    // ACTION_NODE_A(charge, "charge", "intercept with stance");
    ACTION_NODE_A(charge, "charge", "reach melee");
    ACTION_NODE_A(sunder_armor, "sunder armor", "melee");
    ACTION_NODE_A(commanding_shout, "commanding shout", "battle shout");
    // ACTION_NODE_A(shield_slam, "shield slam", "heroic strike");
    ACTION_NODE_A(devastate, "devastate", "sunder armor");
    ACTION_NODE_A(last_stand, "last stand", "intimidating shout");
    ACTION_NODE_A(heroic_throw_on_snare_target, "heroic throw on snare target", "taunt on snare target");
    ACTION_NODE_A(heroic_throw_taunt, "heroic throw", "shield slam");
    static ActionNode* taunt(PlayerbotAI* botAI)
    {
        return new ActionNode("taunt",
                              /*P*/ {},
                              /*A*/ { new NextAction("heroic throw taunt") },
                              /*C*/ {});
    }

    static ActionNode* vigilance(PlayerbotAI* botAI)
    {
        return new ActionNode("vigilance",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* enraged_regeneration(PlayerbotAI* botAI)
    {
        return new ActionNode("enraged regeneration",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

std::vector<NextAction*> TankWarriorStrategy::getDefaultActions()
{
    return {
        new NextAction("devastate", ACTION_DEFAULT + 0.3f), new NextAction("revenge", ACTION_DEFAULT + 0.2f),
        new NextAction("demoralizing shout", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT)
    };
}

void TankWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "vigilance",
        { new NextAction("vigilance", ACTION_HIGH + 7) }));
    triggers.push_back(
        new TriggerNode("enemy out of melee", { new NextAction("heroic throw", ACTION_MOVE + 11),
                                                                new NextAction("charge", ACTION_MOVE + 10) }));
                                                                
    triggers.push_back(new TriggerNode(
        "thunder clap and rage", { new NextAction("thunder clap", ACTION_MOVE + 11) }));
    triggers.push_back(new TriggerNode(
        "defensive stance", { new NextAction("defensive stance", ACTION_HIGH + 9) }));
    triggers.push_back(new TriggerNode(
        "commanding shout", { new NextAction("commanding shout", ACTION_HIGH + 8) }));
    triggers.push_back(
        new TriggerNode("bloodrage", { new NextAction("bloodrage", ACTION_HIGH + 2) }));
    triggers.push_back(
        new TriggerNode("sunder armor", { new NextAction("devastate", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode("medium rage available",
        { new NextAction("shield slam", ACTION_HIGH + 2),
                             new NextAction("devastate", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "shield block", { new NextAction("shield block", ACTION_INTERRUPT + 1) }));
    triggers.push_back(
        new TriggerNode("revenge", { new NextAction("revenge", ACTION_HIGH + 2) }));
    triggers.push_back(
        new TriggerNode("disarm", { new NextAction("disarm", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("lose aggro", { new NextAction("taunt", ACTION_INTERRUPT + 1) }));
    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        { new NextAction("heroic throw on snare target", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "low health", { new NextAction("shield wall", ACTION_MEDIUM_HEAL) }));
    triggers.push_back(new TriggerNode("critical health",
        { new NextAction("last stand", ACTION_EMERGENCY + 3),
                             new NextAction("enraged regeneration", ACTION_EMERGENCY + 2) }));
    triggers.push_back(new TriggerNode(
        "high aoe", { new NextAction("challenging shout", ACTION_HIGH + 3) }));
    triggers.push_back(new TriggerNode(
        "concussion blow", { new NextAction("concussion blow", ACTION_INTERRUPT) }));
    triggers.push_back(
        new TriggerNode("shield bash", { new NextAction("shield bash", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        { new NextAction("shield bash on enemy healer", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode(
        "spell reflection", { new NextAction("spell reflection", ACTION_INTERRUPT + 1) }));
    triggers.push_back(new TriggerNode(
        "victory rush", { new NextAction("victory rush", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode("sword and board",
                                       { new NextAction("shield slam", ACTION_INTERRUPT) }));
    triggers.push_back(
        new TriggerNode("rend", { new NextAction("rend", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode(
        "rend on attacker", { new NextAction("rend on attacker", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("protect party member",
                                       { new NextAction("intervene", ACTION_EMERGENCY) }));
    triggers.push_back(new TriggerNode(
        "high rage available", { new NextAction("heroic strike", ACTION_HIGH) }));
    triggers.push_back(new TriggerNode("medium rage available",
                                       { new NextAction("thunder clap", ACTION_HIGH + 1) }));
}
