/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BearTankDruidStrategy.h"

#include "CreateNextAction.h"
#include "ActionNode.h"
#include "GenericActions.h"
#include "ReachTargetActions.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "DruidBearActions.h"

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
            /*P*/ { CreateNextAction<CastFeralChargeBearAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* feral_charge_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "feral charge - bear",
            /*P*/ {},
            /*A*/ { CreateNextAction<ReachMeleeAction>(1.0f) },
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
            /*P*/ { CreateNextAction<CastFeralChargeBearAction>(1.0f) },
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
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ { CreateNextAction<CastBearFormAction>(1.0f) },
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
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* bash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "bash",
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* swipe([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "swipe",
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* lacerate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "lacerate",
            /*P*/ {},
            /*A*/ { CreateNextAction<CastMaulAction>(1.0f) },
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
        CreateNextAction<CastMangleBearAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastFaerieFireFeralAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastLacerateAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastMaulAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastEnrageAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void BearTankDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastFeralChargeBearAction>(ACTION_NORMAL + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bear form",
            {
                CreateNextAction<CastDireBearFormAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastFrenziedRegenerationAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "faerie fire (feral)",
            {
                CreateNextAction<CastFaerieFireFeralAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                CreateNextAction<CastGrowlAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastDemoralizingRoarAction>(ACTION_HIGH + 6.0f),
                CreateNextAction<CastSwipeBearAction>(ACTION_HIGH + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastSwipeBearAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash",
            {
                CreateNextAction<CastBashAction>(ACTION_INTERRUPT + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bash on enemy healer",
            {
                CreateNextAction<CastBashOnEnemyHealerAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
}
