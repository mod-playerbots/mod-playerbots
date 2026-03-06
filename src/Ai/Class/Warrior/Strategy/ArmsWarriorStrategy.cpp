/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ArmsWarriorStrategy.h"

class ArmsWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArmsWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["death wish"] = &death_wish;
        creators["piercing howl"] = &piercing_howl;
        creators["mocking blow"] = &mocking_blow;
        creators["heroic strike"] = &heroic_strike;
        creators["enraged regeneration"] = &enraged_regeneration;
        creators["retaliation"] = &retaliation;
        creators["shattering throw"] = &shattering_throw;
    }

private:
    static ActionNode* charge(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "charge",
            /*P*/ {},
            /*A*/ { NextAction("reach melee") },
            /*C*/ {}
        );
    }

    static ActionNode* death_wish(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "death wish",
            /*P*/ {},
            /*A*/ { NextAction("bloodrage") },
            /*C*/ {}
        );
    }

    static ActionNode* piercing_howl(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "piercing howl",
            /*P*/ {},
            /*A*/ { NextAction("mocking blow") },
            /*C*/ {}
        );
    }

    static ActionNode* mocking_blow(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mocking blow",
            /*P*/ {},
            /*A*/ { NextAction("hamstring") },
            /*C*/ {}
        );
    }

    static ActionNode* heroic_strike(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "heroic strike",
            /*P*/ {},
            /*A*/ { NextAction("melee") },
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

    static ActionNode* retaliation(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "retaliation",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shattering_throw(PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shattering throw",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

ArmsWarriorStrategy::ArmsWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new ArmsWarriorStrategyActionNodeFactory());
}

std::vector<NextAction> ArmsWarriorStrategy::getDefaultActions()
{
    return {
        NextAction("bladestorm", ACTION_DEFAULT + 0.2f),
        NextAction("mortal strike", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}
void ArmsWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("charge", ACTION_MOVE + 10)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "battle stance",
            {
                NextAction("battle stance", ACTION_HIGH + 10)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "battle shout",
            {
                NextAction("battle shout", ACTION_HIGH + 9)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "rend",
            {
                NextAction("rend", ACTION_HIGH + 8)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "rend on attacker",
            {
                NextAction("rend on attacker", ACTION_HIGH + 8)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mortal strike",
            {
                NextAction("mortal strike", ACTION_HIGH + 3)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                NextAction("execute", ACTION_HIGH + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "sudden death",
            {
                NextAction("execute", ACTION_HIGH + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hamstring",
            {
                NextAction("piercing howl", ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "overpower",
            {
                NextAction("overpower", ACTION_HIGH + 4)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "taste for blood",
            {
                NextAction("overpower", ACTION_HIGH + 4)
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
            "high rage available",
            {
                NextAction("heroic strike", ACTION_HIGH),
                NextAction("slam", ACTION_HIGH + 1)
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
            "death wish",
            {
                NextAction("death wish", ACTION_HIGH + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                NextAction("intimidating shout", ACTION_EMERGENCY)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium health",
            {
                NextAction("enraged regeneration", ACTION_EMERGENCY)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "almost full health",
            {
                NextAction("retaliation", ACTION_EMERGENCY + 1)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "shattering throw trigger",
            {
                NextAction("shattering throw", ACTION_INTERRUPT + 1)
            }
        )
    );
}
