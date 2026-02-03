/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FuryWarriorStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "WarriorActions.h"

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
    static ActionNode* charge(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastInterceptAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* intercept(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<ReachMeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* piercing_howl(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHamstringAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* pummel(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastInterceptAction>(1.0f) },
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
};

FuryWarriorStrategy::FuryWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new FuryWarriorStrategyActionNodeFactory());
}

std::vector<NextAction> FuryWarriorStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastBloodthirstAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastWhirlwindAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastSunderArmorAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastExecuteAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void FuryWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastChargeAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "berserker stance", {
                CreateNextAction<CastBerserkerStanceAction>(ACTION_HIGH + 9)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "battle shout",
            {
                CreateNextAction<CastBattleShoutAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pummel on enemy healer",
            {
                CreateNextAction<CastPummelOnEnemyHealerAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pummel",
            {
                CreateNextAction<CastPummelAction>(ACTION_INTERRUPT)
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
            "bloodthirst",
            {
                CreateNextAction<CastBloodthirstAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "whirlwind",
            {
                CreateNextAction<CastWhirlwindAction>(ACTION_HIGH + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "instant slam",
            {
                CreateNextAction<CastSlamAction>(ACTION_HIGH + 5.0f)
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
            "medium rage available",
            {
                CreateNextAction<CastHeroicStrikeAction>(ACTION_DEFAULT + 0.1f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "death wish",
            {
                CreateNextAction<CastDeathWishAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "recklessness",
            {
                CreateNextAction<CastRecklessnessAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastEnragedRegenerationAction>(ACTION_EMERGENCY)
            }
        )
    );
}
