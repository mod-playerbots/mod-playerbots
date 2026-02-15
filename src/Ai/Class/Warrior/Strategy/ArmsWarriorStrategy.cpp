/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ArmsWarriorStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"

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
    static ActionNode* charge(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<ReachMeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* death_wish(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastBloodrageAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* piercing_howl(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastMockingBlowAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* mocking_blow(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHamstringAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* heroic_strike(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* enraged_regeneration(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* retaliation(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shattering_throw(PlayerbotAI*)
    {
        return new ActionNode(
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
        CreateNextAction<CastBladestormAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastMortalStrikeAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}
void ArmsWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastChargeAction>(ACTION_MOVE + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "battle stance",
            {
                CreateNextAction<CastBattleStanceAction>(ACTION_HIGH + 10)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "battle shout",
            {
                CreateNextAction<CastBattleShoutAction>(ACTION_HIGH + 9.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "rend",
            {
                CreateNextAction<CastRendAction>(ACTION_HIGH + 8.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "rend on attacker",
            {
                CreateNextAction<CastRendOnAttackerAction>(ACTION_HIGH + 8.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mortal strike",
            {
                CreateNextAction<CastMortalStrikeAction>(ACTION_HIGH + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                CreateNextAction<CastExecuteAction>(ACTION_HIGH + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "sudden death",
            {
                CreateNextAction<CastExecuteAction>(ACTION_HIGH + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hamstring",
            {
                CreateNextAction<CastPiercingHowlAction>(ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "overpower",
            {
                CreateNextAction<CastOverpowerAction>(ACTION_HIGH + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "taste for blood",
            {
                CreateNextAction<CastOverpowerAction>(ACTION_HIGH + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "victory rush",
            {
                CreateNextAction<CastVictoryRushAction>(ACTION_INTERRUPT)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high rage available",
            {
                CreateNextAction<CastHeroicStrikeAction>(ACTION_HIGH),
                CreateNextAction<CastSlamAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bloodrage",
            {
                CreateNextAction<CastBloodrageAction>(ACTION_HIGH + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "death wish",
            {
                CreateNextAction<CastDeathWishAction>(ACTION_HIGH + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastIntimidatingShoutAction>(ACTION_EMERGENCY)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium health",
            {
                CreateNextAction<CastEnragedRegenerationAction>(ACTION_EMERGENCY)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "almost full health",
            {
                CreateNextAction<CastRetaliationAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "shattering throw trigger",
            {
                CreateNextAction<CastShatteringThrowAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
}
