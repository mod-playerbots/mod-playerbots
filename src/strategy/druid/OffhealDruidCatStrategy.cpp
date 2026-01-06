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
        return new ActionNode(
            "cat form",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mangle (cat)",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shred([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shred",
            /*P*/ {},
            /*A*/ { NextAction("claw") },
            /*C*/ {}
        );
    }

    static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rake",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rip",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "ferocious bite",
            /*P*/ {},
            /*A*/ { NextAction("rip") },
            /*C*/ {}
        );
    }

    static ActionNode* savage_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "savage roar",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "faerie fire (feral)",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "healing touch on party",
            /*P*/ { NextAction("caster form") },
            /*A*/ {},
            /*C*/ { NextAction("cat form") }
        );
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "regrowth on party",
            /*P*/ { NextAction("caster form") },
            /*A*/ {},
            /*C*/ { NextAction("cat form") }
        );
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rejuvenation on party",
            /*P*/ { NextAction("caster form") },
            /*A*/ {},
            /*C*/ { NextAction("cat form") }
        );
    }
};

OffhealDruidCatStrategy::OffhealDruidCatStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new OffhealDruidCatStrategyActionNodeFactory());
}

std::vector<NextAction> OffhealDruidCatStrategy::getDefaultActions()
{
    return {
        NextAction("mangle (cat)", ACTION_DEFAULT + 0.5f),
        NextAction("shred", ACTION_DEFAULT + 0.4f),
        NextAction("rake", ACTION_DEFAULT + 0.3f),
        NextAction("melee", ACTION_DEFAULT),
        NextAction("cat form", ACTION_DEFAULT - 0.1f)
    };
}

void OffhealDruidCatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "cat form",
            {
                NextAction("cat form", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "savage roar",
            {
                NextAction("savage roar", ACTION_HIGH + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points available",
            {
                NextAction("rip", ACTION_HIGH + 6)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ferocious bite time",
            {
                NextAction("ferocious bite", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                NextAction("ferocious bite", ACTION_HIGH + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mangle (cat)",
            {
                NextAction("mangle (cat)", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rake",
            {
                NextAction("rake", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available",
            {
                NextAction("shred", ACTION_DEFAULT + 0.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full",
            {
                NextAction("shred", ACTION_DEFAULT + 0.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "faerie fire (feral)",
            {
                NextAction("faerie fire (feral)", ACTION_NORMAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("feral charge - cat", ACTION_HIGH + 9),
                NextAction("dash", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("swipe (cat)", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low energy",
            {
                NextAction("tiger's fury", ACTION_NORMAL + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_EMERGENCY + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("innervate", ACTION_HIGH + 4)
            }
        )
    );
}
