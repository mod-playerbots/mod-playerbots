/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

 #include "OffhealDruidCatStrategy.h"

#include "CreateNextAction.h"
#include "DruidActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "GenericActions.h"
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
            /*P*/ {},
            /*A*/ {},
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

    static ActionNode* shred([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastClawAction>(1.0f) },
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

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
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

    static ActionNode* savage_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ { CreateNextAction<CastCatFormAction>(1.0f) }
        );
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ { CreateNextAction<CastCatFormAction>(1.0f) }
        );
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ { CreateNextAction<CastCatFormAction>(1.0f) }
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
        CreateNextAction<CastMangleCatAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastShredAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastRakeAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT),
        CreateNextAction<CastCatFormAction>(ACTION_DEFAULT - 0.1f)
    };
}

void OffhealDruidCatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "cat form",
            {
                CreateNextAction<CastCatFormAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "savage roar",
            {
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
            "faerie fire (feral)",
            {
                CreateNextAction<CastFaerieFireFeralAction>(ACTION_NORMAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastFeralChargeCatAction>(ACTION_HIGH + 9.0f),
                CreateNextAction<CastDashAction>(ACTION_HIGH + 8)
            }
        )
    );
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
            "low energy",
            {
                CreateNextAction<CastTigersFuryAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastHealingTouchOnPartyAction>(ACTION_CRITICAL_HEAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastHealingTouchOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_LIGHT_HEAL + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                CreateNextAction<ReachPartyMemberToHealAction>(ACTION_EMERGENCY + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastInnervateAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
}
