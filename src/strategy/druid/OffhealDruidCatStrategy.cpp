/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

 #include "OffhealDruidCatStrategy.h"

 #include "Playerbots.h"
 #include "Strategy.h"

 class OffhealDruidCatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    OffhealDruidCatStrategyActionNodeFactory()
    {
        creators["cat form"] = &cat_form;
        creators["mangle (cat)"] = &mangle_cat;
        creators["shred"] = &shred;
        creators["rake"] = &rake;
        creators["rip"] = &rip;
        creators["ferocious bite"] = &ferocious_bite;
        creators["savage roar"] = &savage_roar;
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["healing touch on party"] = &healing_touch_on_party;
        creators["regrowth on party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
    }

private:
    static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cat form",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mangle (cat)",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* shred([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("shred",
                              /*P*/ {},
                              /*A*/ { new NextAction("claw") },
                              /*C*/ {});
    }

    static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rake",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rip",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ferocious bite",
                              /*P*/ {},
                              /*A*/ { new NextAction("rip") },
                              /*C*/ {});
    }

    static ActionNode* savage_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("savage roar",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire (feral)",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch on party",
                              /*P*/ { new NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ { new NextAction("cat form") });
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ { new NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ { new NextAction("cat form") });
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ { new NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ { new NextAction("cat form") });
    }
};

OffhealDruidCatStrategy::OffhealDruidCatStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new OffhealDruidCatStrategyActionNodeFactory());
}

std::vector<NextAction*> OffhealDruidCatStrategy::getDefaultActions()
{
    return { new NextAction("mangle (cat)", ACTION_DEFAULT + 0.5f),
                             new NextAction("shred", ACTION_DEFAULT + 0.4f),
                             new NextAction("rake", ACTION_DEFAULT + 0.3f), new NextAction("melee", ACTION_DEFAULT),
                             new NextAction("cat form", ACTION_DEFAULT - 0.1f) };
}

void OffhealDruidCatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("cat form", { new NextAction("cat form", ACTION_HIGH + 8) }));
    triggers.push_back(
        new TriggerNode("savage roar", { new NextAction("savage roar", ACTION_HIGH + 7) }));
    triggers.push_back(new TriggerNode("combo points available",
                                       { new NextAction("rip", ACTION_HIGH + 6) }));
    triggers.push_back(new TriggerNode(
        "ferocious bite time", { new NextAction("ferocious bite", ACTION_HIGH + 5) }));
    triggers.push_back(
        new TriggerNode("target with combo points almost dead",
                        { new NextAction("ferocious bite", ACTION_HIGH + 4) }));
    triggers.push_back(new TriggerNode("mangle (cat)",
                                       { new NextAction("mangle (cat)", ACTION_HIGH + 3) }));
    triggers.push_back(new TriggerNode("rake", { new NextAction("rake", ACTION_HIGH + 2) }));
    triggers.push_back(new TriggerNode("almost full energy available",
                                       { new NextAction("shred", ACTION_DEFAULT + 0.4f) }));
    triggers.push_back(new TriggerNode("combo points not full",
                                       { new NextAction("shred", ACTION_DEFAULT + 0.4f) }));
    triggers.push_back(new TriggerNode(
        "faerie fire (feral)", { new NextAction("faerie fire (feral)", ACTION_NORMAL) }));
    triggers.push_back(new TriggerNode("enemy out of melee",
                                       { new NextAction("feral charge - cat", ACTION_HIGH + 9),
                                                         new NextAction("dash", ACTION_HIGH + 8) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { new NextAction("swipe (cat)", ACTION_HIGH + 3) }));
    triggers.push_back(new TriggerNode(
        "low energy", { new NextAction("tiger's fury", ACTION_NORMAL + 1) }));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        { new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 5) }));
    triggers.push_back(new TriggerNode(
        "party member low health",
        { new NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 5) }));
    triggers.push_back(
        new TriggerNode("party member medium health",
                        { new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 8) }));
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        { new NextAction("reach party member to heal", ACTION_EMERGENCY + 3) }));
    triggers.push_back(
        new TriggerNode("low mana", { new NextAction("innervate", ACTION_HIGH + 4) }));
}
