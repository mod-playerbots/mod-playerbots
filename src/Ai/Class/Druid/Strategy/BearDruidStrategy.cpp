/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BearDruidStrategy.h"

#include "Playerbots.h"

class BearDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BearDruidStrategyActionNodeFactory()
    {
        creators["feral charge - bear"] = &feral_charge_bear;
        creators["dire bear form"] = &dire_bear_form;
        creators["maul"] = &maul;
        creators["bash"] = &bash;
        creators["swipe"] = &swipe;
        creators["lacerate"] = &lacerate;
    }

private:
    static ActionNode* feral_charge_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "feral charge - bear",
            /*P*/ {},
            /*A*/ { NextAction("reach melee") },
            /*C*/ {}
        );
    }

    static ActionNode* dire_bear_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "dire bear form",
            /*P*/ { NextAction("caster form") },
            /*A*/ { NextAction("bear form") },
            /*C*/ {}
        );
    }

    static ActionNode* maul([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "maul",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }

    static ActionNode* bash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "bash",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }

    static ActionNode* swipe([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "swipe",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
            /*C*/ {}
        );
    }

    static ActionNode* lacerate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "lacerate",
            /*P*/ {},
            /*A*/ { NextAction("maul") },
            /*C*/ {}
        );
    }
};

BearDruidStrategy::BearDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new BearDruidStrategyActionNodeFactory());
}

std::vector<NextAction> BearDruidStrategy::getDefaultActions()
{
    return {
        NextAction("maul",   5.2f),
        NextAction("enrage", 5.1f),
        NextAction("melee",  5.0f)
    };
}

void BearDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "bear form",
            { NextAction("dire bear form", 28.0f) }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium health",
            { NextAction("frenzied regeneration", 27.0f) }
        )
    );
    triggers.push_back(new TriggerNode(
        "mangle (bear)", { NextAction("mangle (bear)", 17.5f) }
    ));
    triggers.push_back(new TriggerNode(
        "faerie fire (feral)", { NextAction("faerie fire (feral)", 17.0f) }
    ));
    triggers.push_back(new TriggerNode(
        "lacerate", { NextAction("lacerate", 16.0f) }
    ));
    triggers.push_back(new TriggerNode(
        "demoralizing roar", { NextAction("demoralizing roar", 15.5f) }
    ));
    triggers.push_back(new TriggerNode("high aoe",    { NextAction("challenging roar", 26.5f) }));
    triggers.push_back(new TriggerNode("lose aggro",
        {
            NextAction("growl",               26.0f),
            NextAction("faerie fire (feral)", 25.5f)
        }
    ));
    triggers.push_back(new TriggerNode("berserk active", { NextAction("mangle (bear)", 25.0f) }));
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("demoralizing roar", 24.5f),
                NextAction("swipe (bear)",      24.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            { NextAction("swipe (bear)", 24.0f) }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash",
            { NextAction("bash", 42.0f) }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash on enemy healer",
            { NextAction("bash on enemy healer", 41.0f) }
        )
    );
}
