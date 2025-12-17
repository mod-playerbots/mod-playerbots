/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DpsRogueStrategy.h"

#include "Playerbots.h"

class DpsRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["backstab"] = &backstab;
        creators["melee"] = &melee;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ {},
                              /*A*/ { new NextAction("mutilate") },
                              /*C*/ {});
    }
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mutilate",
                              /*P*/ {},
                              /*A*/ { new NextAction("sinister strike") },
                              /*C*/ {});
    }
    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("sinister strike",
                              /*P*/ {},
                              /*A*/ { new NextAction("melee") },
                              /*C*/ {});
    }
    static ActionNode* kick([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("kick",
                              /*P*/ {},
                              /*A*/ { new NextAction("kidney shot") },
                              /*C*/ {});
    }
    static ActionNode* kidney_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("kidney shot",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("backstab",
                              /*P*/ {},
                              /*A*/ { new NextAction("mutilate") },
                              /*C*/ {});
    }
    static ActionNode* rupture([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rupture",
                              /*P*/ {},
                              /*A*/ { new NextAction("eviscerate") },
                              /*C*/ {});
    }
};

DpsRogueStrategy::DpsRogueStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new DpsRogueStrategyActionNodeFactory());
}

std::vector<NextAction*> DpsRogueStrategy::getDefaultActions()
{
    return { new NextAction("killing spree", ACTION_DEFAULT + 0.1f),
                             new NextAction("melee", ACTION_DEFAULT) };
}

void DpsRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("high energy available",
                                       { new NextAction("garrote", ACTION_HIGH + 7),
                                                         new NextAction("ambush", ACTION_HIGH + 6) }));

    triggers.push_back(new TriggerNode(
        "high energy available", { new NextAction("sinister strike", ACTION_NORMAL + 3) }));

    triggers.push_back(new TriggerNode(
        "slice and dice", { new NextAction("slice and dice", ACTION_HIGH + 2) }));

    triggers.push_back(new TriggerNode("combo points available",
                                       { new NextAction("rupture", ACTION_HIGH + 1),
                                                         new NextAction("eviscerate", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("target with combo points almost dead",
                                       { new NextAction("eviscerate", ACTION_HIGH + 2) }));

    triggers.push_back(
        new TriggerNode("medium threat", { new NextAction("vanish", ACTION_HIGH) }));

    triggers.push_back(
        new TriggerNode("low health", { new NextAction("evasion", ACTION_HIGH + 9),
                                                        new NextAction("feint", ACTION_HIGH + 8) }));

    triggers.push_back(new TriggerNode(
        "critical health", { new NextAction("cloak of shadows", ACTION_HIGH + 7) }));

    triggers.push_back(
        new TriggerNode("kick", { new NextAction("kick", ACTION_INTERRUPT + 2) }));

    triggers.push_back(
        new TriggerNode("kick on enemy healer",
                        { new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1) }));

    // triggers.push_back(new TriggerNode(
    //     "behind target",
    //     { new NextAction("backstab", ACTION_NORMAL) }));

    triggers.push_back(
        new TriggerNode("light aoe", { new NextAction("blade flurry", ACTION_HIGH + 3) }));

    triggers.push_back(new TriggerNode("blade flurry",
                                       { new NextAction("blade flurry", ACTION_HIGH + 2) }));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        { new NextAction("stealth", ACTION_HIGH + 3), new NextAction("sprint", ACTION_HIGH + 2),
                          new NextAction("reach melee", ACTION_HIGH + 1) }));

    triggers.push_back(new TriggerNode("expose armor",
                                       { new NextAction("expose armor", ACTION_HIGH + 3) }));

    triggers.push_back(new TriggerNode(
        "low tank threat",
        { new NextAction("tricks of the trade on main tank", ACTION_HIGH + 7) }));
}

class StealthedRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    StealthedRogueStrategyActionNodeFactory()
    {
        creators["ambush"] = &ambush;
        creators["cheap shot"] = &cheap_shot;
        creators["garrote"] = &garrote;
        creators["sap"] = &sap;
        creators["sinister strike"] = &sinister_strike;
    }

private:
    static ActionNode* ambush([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ambush",
                              /*P*/ {},
                              /*A*/ { new NextAction("garrote") },
                              /*C*/ {});
    }

    static ActionNode* cheap_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cheap shot",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* garrote([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("garrote",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* sap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("sap",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("sinister strike",
                              /*P*/ {},
                              /*A*/ { new NextAction("cheap shot") },
                              /*C*/ {});
    }
};

StealthedRogueStrategy::StealthedRogueStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new StealthedRogueStrategyActionNodeFactory());
}

std::vector<NextAction*> StealthedRogueStrategy::getDefaultActions()
{
    return {
        new NextAction("ambush", ACTION_NORMAL + 4), new NextAction("backstab", ACTION_NORMAL + 3),
        new NextAction("cheap shot", ACTION_NORMAL + 2), new NextAction("sinister strike", ACTION_NORMAL + 1),
        new NextAction("melee", ACTION_NORMAL) };
}

void StealthedRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("combo points available",
                                       { new NextAction("eviscerate", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("kick", { new NextAction("cheap shot", ACTION_INTERRUPT) }));
    triggers.push_back(new TriggerNode("kick on enemy healer",
                                       { new NextAction("cheap shot", ACTION_INTERRUPT) }));
    triggers.push_back(
        new TriggerNode("behind target", { new NextAction("ambush", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("not behind target", { new NextAction("cheap shot", ACTION_HIGH) }));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       { new NextAction("sprint", ACTION_EMERGENCY + 1) }));
    triggers.push_back(
        new TriggerNode("unstealth", { new NextAction("unstealth", ACTION_NORMAL) }));
    /*triggers.push_back(new TriggerNode("low health", { new NextAction("food", ACTION_EMERGENCY +
     * 1) }));*/
    triggers.push_back(new TriggerNode(
        "no stealth", { new NextAction("check stealth", ACTION_EMERGENCY) }));
    triggers.push_back(
        new TriggerNode("sprint", { new NextAction("sprint", ACTION_INTERRUPT) }));
}

void StealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("stealth", { new NextAction("stealth", ACTION_INTERRUPT) }));
}

void RogueAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("light aoe", { new NextAction("blade flurry", ACTION_HIGH) }));
    triggers.push_back(new TriggerNode(
        "medium aoe", { new NextAction("fan of knives", ACTION_NORMAL + 5) }));
}

void RogueBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "adrenaline rush", { new NextAction("adrenaline rush", ACTION_HIGH + 2) }));
}

void RogueCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sap", { new NextAction("stealth", ACTION_INTERRUPT),
                                                                new NextAction("sap", ACTION_INTERRUPT) }));
}
