/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CatDpsDruidStrategy.h"

#include "AiObjectContext.h"
#include "ActionNode.h"
#include "CreateNextAction.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "DruidCatActions.h"
#include "GenericActions.h"
#include "MovementActions.h"

class CatDpsDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatDpsDruidStrategyActionNodeFactory()
    {
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["melee"] = &melee;
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["cat form"] = &cat_form;
        creators["claw"] = &claw;
        creators["mangle (cat)"] = &mangle_cat;
        creators["rake"] = &rake;
        creators["ferocious bite"] = &ferocious_bite;
        creators["rip"] = &rip;
        creators["pounce"] = &pounce;
        creators["ravage"] = &ravage;
    }

private:
    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastFeralChargeCatAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* feral_charge_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<ReachMeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ { NextAction("bear form") },
            /*C*/ {}
        );
    }

    static ActionNode* claw([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRipAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* pounce([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRavageAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* ravage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShredAction>(1.0f) },
            /*C*/ {}
        );
    }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CatDpsDruidStrategyActionNodeFactory());
}

std::vector<NextAction> CatDpsDruidStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastTigersFuryAction>(ACTION_DEFAULT + 0.1f)
    };
}

void CatDpsDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    // Default priority
    triggers.push_back(
        new TriggerNode(
            "almost full energy available",
            {
                CreateNextAction<CastShredAction>(ACTION_DEFAULT + 0.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full",
            {
                CreateNextAction<CastShredAction>(ACTION_DEFAULT + 0.4f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available",
            {
                CreateNextAction<CastMangleCatAction>(ACTION_DEFAULT + 0.3f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full and high energy",
            {
                CreateNextAction<CastMangleCatAction>(ACTION_DEFAULT + 0.3f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "almost full energy available",
            {
                CreateNextAction<CastClawAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points not full and high energy",
            {
                CreateNextAction<CastClawAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "faerie fire (feral)",
            {
                CreateNextAction<CastFaerieFireFeralAction>(ACTION_DEFAULT)
            }
        )
    );

    // Main spell
    triggers.push_back(
        new TriggerNode(
            "cat form", {
                CreateNextAction<CastCatFormAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "savage roar", {
                CreateNextAction<CastSavageRoarAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combo points available",
            {
                CreateNextAction<CastRipAction>(ACTION_HIGH + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "ferocious bite time",
            {
                CreateNextAction<CastFerociousBiteAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                CreateNextAction<CastFerociousBiteAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mangle (cat)",
            {
                CreateNextAction<CastMangleCatAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rake",
            {
                CreateNextAction<CastRakeAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastCowerAction>(ACTION_HIGH + 1.0f)
            }
        )
    );

    // AOE
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastSwipeCatAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastRakeOnMeleeAttackersAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    // Reach target
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastFeralChargeCatAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastDashAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
}

void CatAoeDruidStrategy::InitTriggers(std::vector<TriggerNode*>&) {}
