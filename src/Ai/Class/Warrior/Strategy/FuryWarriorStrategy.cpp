/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FuryWarriorStrategy.h"

class FuryWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FuryWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["intercept"] = &intercept;
        creators["piercing howl"] = &piercing_howl;
        creators["pummel"] = &pummel;
        creators["enraged regeneration"] = &enraged_regeneration;
    }

private:
    static ActionNode* charge(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "charge",
            /*P*/ {},
            /*A*/ { NextAction("intercept" )},
            /*C*/ {}
        );
    }

    static ActionNode* intercept(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "intercept",
            /*P*/ {},
            /*A*/ { NextAction("reach melee" )},
            /*C*/ {}
        );
    }

    static ActionNode* piercing_howl(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "piercing howl",
            /*P*/ {},
            /*A*/ { NextAction("hamstring" )},
            /*C*/ {}
        );
    }

    static ActionNode* pummel(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "pummel",
            /*P*/ {},
            /*A*/ { NextAction("intercept" )},
            /*C*/ {}
        );
    }

    static ActionNode* enraged_regeneration(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "enraged regeneration",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

FuryWarriorStrategy::FuryWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new FuryWarriorStrategyActionNodeFactory());
}

std::vector<NextAction> FuryWarriorStrategy::getDefaultActions()
{
    return {
        NextAction("bloodthirst", ACTION_DEFAULT + 0.5f),
        NextAction("whirlwind", ACTION_DEFAULT + 0.4f),
        NextAction("sunder armor", ACTION_DEFAULT + 0.3f),
        NextAction("execute", ACTION_DEFAULT + 0.2f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void FuryWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("charge", ACTION_MOVE + 9)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "berserker stance", {
                NextAction("berserker stance", ACTION_HIGH + 9)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "battle shout",
            {
                NextAction("battle shout", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pummel on enemy healer",
            {
                NextAction("pummel on enemy healer", ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pummel",
            {
                NextAction("pummel", ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "victory rush",
            {
                NextAction("victory rush", ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bloodthirst",
            {
                NextAction("bloodthirst", ACTION_HIGH + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "whirlwind",
            {
                NextAction("whirlwind", ACTION_HIGH + 6)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "instant slam",
            {
                NextAction("slam", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bloodrage",
            {
                NextAction("bloodrage", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium rage available",
            {
                NextAction("heroic strike", ACTION_DEFAULT + 0.1f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "death wish",
            {
                NextAction("death wish", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "recklessness",
            {
                NextAction("recklessness", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                NextAction("enraged regeneration", ACTION_EMERGENCY)
            }
        )
    );
}
