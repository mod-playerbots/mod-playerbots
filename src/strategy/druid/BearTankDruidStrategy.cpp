/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BearTankDruidStrategy.h"

#include "Playerbots.h"

class BearTankDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BearTankDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["feral charge - bear"] = &feral_charge_bear;
        creators["swipe (bear)"] = &swipe_bear;
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["bear form"] = &bear_form;
        creators["dire bear form"] = &dire_bear_form;
        creators["mangle (bear)"] = &mangle_bear;
        creators["maul"] = &maul;
        creators["bash"] = &bash;
        creators["swipe"] = &swipe;
        creators["lacerate"] = &lacerate;
        creators["demoralizing roar"] = &demoralizing_roar;
        creators["taunt spell"] = &growl;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "melee",
            /*P*/ { NextAction("feral charge - bear") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* feral_charge_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "feral charge - bear",
            /*P*/ {},
            /*A*/ { NextAction("reach melee") },
            /*C*/ {}
        );
    }

    static ActionNode* swipe_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "swipe (bear)",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "faerie fire (feral)",
            /*P*/ { NextAction("feral charge - bear") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* bear_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "bear form",
            /*P*/ {},
            /*A*/ {},
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

    static ActionNode* mangle_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mangle (bear)",
            /*P*/ {},
            /*A*/ {},
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

    static ActionNode* growl([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "growl",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* demoralizing_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "demoralizing roar",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

BearTankDruidStrategy::BearTankDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new BearTankDruidStrategyActionNodeFactory());
}

std::vector<NextAction> BearTankDruidStrategy::getDefaultActions()
{
    return {
        NextAction("mangle (bear)", ACTION_DEFAULT + 0.5f),
        NextAction("faerie fire (feral)", ACTION_DEFAULT + 0.4f),
        NextAction("lacerate", ACTION_DEFAULT + 0.3f),
        NextAction("maul", ACTION_DEFAULT + 0.2f),
        NextAction("enrage", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void BearTankDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("feral charge - bear", ACTION_NORMAL + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bear form",
            {
                NextAction("dire bear form", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("frenzied regeneration", ACTION_HIGH + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "faerie fire (feral)",
            {
                NextAction("faerie fire (feral)", ACTION_HIGH + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                NextAction("growl", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("demoralizing roar", ACTION_HIGH + 6),
                NextAction("swipe (bear)", ACTION_HIGH + 6)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                NextAction("swipe (bear)", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash",
            {
                NextAction("bash", ACTION_INTERRUPT + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash on enemy healer",
            {
                NextAction("bash on enemy healer", ACTION_INTERRUPT + 1)
            }
        )
    );
}
