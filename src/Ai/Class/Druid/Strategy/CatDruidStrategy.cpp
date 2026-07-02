/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CatDruidStrategy.h"

#include "AiObjectContext.h"

class CatDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatDruidStrategyActionNodeFactory()
    {
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["cat form"] = &cat_form;
        creators["claw"] = &claw;
        creators["pounce"] = &pounce;
        creators["ravage"] = &ravage;
        creators["prowl"] = &prowl;
    }

private:
    static ActionNode* feral_charge_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "feral charge - cat",
            /*P*/ {},
            /*A*/ { NextAction("reach melee") },
            /*C*/ {}
        );
    }

    static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "cat form",
            /*P*/ { NextAction("caster form") },
            /*A*/ { NextAction("bear form") },
            /*C*/ {}
        );
    }

    static ActionNode* claw([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "claw",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }

    static ActionNode* ravage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "ravage",
            /*P*/ {},
            /*A*/ { NextAction("pounce") },
            /*C*/ {}
        );
    }

    static ActionNode* pounce([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "pounce",
            /*P*/ {},
            /*A*/ { NextAction("shred") },
            /*C*/ {}
        );
    }

    static ActionNode* prowl([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "prowl",
            /*P*/ { NextAction("cat form") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

CatDruidStrategy::CatDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CatDruidStrategyActionNodeFactory());
}

std::vector<NextAction> CatDruidStrategy::getDefaultActions()
{
    return {
        NextAction("melee", ACTION_DEFAULT)
    };
}

void CatDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "healer low mana", {
                NextAction("innervate on healer", 35.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "prowl", {
                NextAction("prowl", 29.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee", {
                NextAction("dash", 28.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "cat form", {
                NextAction("cat form", 28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low energy", {
                NextAction("tiger's fury", 27.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "savage roar", {
                NextAction("savage roar", 26.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "combo points 5 available", {
                NextAction("rip", 23.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points 5 available", {
                NextAction("maim", 23.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ferocious bite execute", {
                NextAction("ferocious bite", 24.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "clearcasting", {
                NextAction("shred", 24.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ferocious bite time", {
                NextAction("ferocious bite", 22.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mangle (cat)", {
                NextAction("mangle (cat)", 22.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rake", {
                NextAction("rake", 21.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium threat", {
                NextAction("cower", 21.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "almost full energy available", {
                NextAction("ravage", 5.6f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full", {
                NextAction("ravage", 5.6f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available", {
                NextAction("pounce", 5.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full", {
                NextAction("pounce", 5.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available", {
                NextAction("shred", 5.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full", {
                NextAction("shred", 5.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available", {
                NextAction("mangle (cat)", 5.3f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full and high energy", {
                NextAction("mangle (cat)", 5.3f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available", {
                NextAction("claw", 5.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full and high energy", {
                NextAction("claw", 5.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "faerie fire (feral)", {
                NextAction("faerie fire (feral)", 5.0f)
            }
        )
    );
}

// ============================================================
// CatOffhealStrategy
// Additive overlay — only the healing triggers. Designed to be
// stacked on top of "cat" so the bot stays in cat form for DPS
// but shifts out to heal when the party needs it.
// ============================================================

class CatOffhealStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatOffhealStrategyActionNodeFactory()
    {
        creators["healing touch on party"] = &healing_touch_on_party;
        creators["regrowth on party"]       = &regrowth_on_party;
        creators["rejuvenation on party"]   = &rejuvenation_on_party;
    }

private:
    // P: shift to caster form before casting   C: shift back to cat form afterwards
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

CatOffhealStrategy::CatOffhealStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new CatOffhealStrategyActionNodeFactory());
}

void CatOffhealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("regrowth on party",       36.0f),
                NextAction("healing touch on party",  35.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("healing touch on party", 25.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("rejuvenation on party", 18.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", 93.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("innervate", 24.0f)
            }
        )
    );
}
