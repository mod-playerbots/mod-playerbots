
#include "AssassinationRogueStrategy.h"

#include "Playerbots.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
        creators["backstab"] = &backstab;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mutilate",
                              /*P*/ {},
                              /*A*/ { new NextAction("backstab") },
                              /*C*/ {});
    }
    static ActionNode* envenom([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("envenom",
                              /*P*/ {},
                              /*A*/ { new NextAction("rupture") },
                              /*C*/ {});
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("backstab",
                              /*P*/ {},
                              /*A*/ { new NextAction("sinister strike") },
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

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

std::vector<NextAction*> AssassinationRogueStrategy::getDefaultActions()
{
    return { new NextAction("melee", ACTION_DEFAULT) };
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("high energy available",
                                       { new NextAction("garrote", ACTION_HIGH + 7),
                                                         new NextAction("ambush", ACTION_HIGH + 6) }));

    triggers.push_back(new TriggerNode("high energy available",
                                       { new NextAction("mutilate", ACTION_NORMAL + 3) }));

    triggers.push_back(new TriggerNode(
        "hunger for blood", { new NextAction("hunger for blood", ACTION_HIGH + 6), }));

    triggers.push_back(new TriggerNode("slice and dice",
                                       { new NextAction("slice and dice", ACTION_HIGH + 5), }));

    triggers.push_back(new TriggerNode("combo points 3 available",
                                       { new NextAction("envenom", ACTION_HIGH + 5),
                                       new NextAction("eviscerate", ACTION_HIGH + 3) }));

    triggers.push_back(new TriggerNode("target with combo points almost dead",
                                       { new NextAction("envenom", ACTION_HIGH + 4),
                                       new NextAction("eviscerate", ACTION_HIGH + 2) }));

    triggers.push_back(
        new TriggerNode("expose armor", { new NextAction("expose armor", ACTION_HIGH + 3), }));

    triggers.push_back(
        new TriggerNode("medium threat", { new NextAction("vanish", ACTION_HIGH), }));

    triggers.push_back(
        new TriggerNode("low health", { new NextAction("evasion", ACTION_HIGH + 9),
                                                        new NextAction("feint", ACTION_HIGH + 8) }));

    triggers.push_back(new TriggerNode(
        "critical health", { new NextAction("cloak of shadows", ACTION_HIGH + 7) }));

    triggers.push_back(
        new TriggerNode("kick", { new NextAction("kick", ACTION_INTERRUPT + 2), }));

    triggers.push_back(
        new TriggerNode("kick on enemy healer",
                        { new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1), }));

    triggers.push_back(
        new TriggerNode("medium aoe", { new NextAction("fan of knives", ACTION_NORMAL + 5), }));

    triggers.push_back(new TriggerNode(
        "low tank threat",
        { new NextAction("tricks of the trade on main tank", ACTION_HIGH + 7), }));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        { new NextAction("stealth", ACTION_HIGH + 3), new NextAction("sprint", ACTION_HIGH + 2),
                          new NextAction("reach melee", ACTION_HIGH + 1), }));
}
