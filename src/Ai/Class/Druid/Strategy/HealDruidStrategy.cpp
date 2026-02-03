/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HealDruidStrategy.h"
#include "CreateNextAction.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "MovementActions.h"

class HealDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealDruidStrategyActionNodeFactory() {
        creators["nourish on party"] = &nourtish_on_party;
    }

private:
    static ActionNode* nourtish_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHealingTouchOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
};

HealDruidStrategy::HealDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new HealDruidStrategyActionNodeFactory());
}

void HealDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                CreateNextAction<HealPartyMemberAction>(ACTION_CRITICAL_HEAL + 9.0f)
            }
        )
    );

    // CRITICAL
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastTreeFormAction>(ACTION_CRITICAL_HEAL + 4.1f),
                CreateNextAction<CastPartySwiftmendAction>(ACTION_CRITICAL_HEAL + 4.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_CRITICAL_HEAL + 3.0f),
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_CRITICAL_HEAL + 2.0f),
                CreateNextAction<CastPartyNourishAction>(ACTION_CRITICAL_HEAL + 1.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastNaturesSwiftnessAction>(ACTION_CRITICAL_HEAL + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                CreateNextAction<CastTreeFormAction>(ACTION_MEDIUM_HEAL + 2.3f),
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 2.2f),
                CreateNextAction<CastRejuvenationOnNotFullAction>(ACTION_MEDIUM_HEAL + 2.1f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                CreateNextAction<CastTreeFormAction>(ACTION_CRITICAL_HEAL + 0.6f),
                CreateNextAction<CastTranquilityAction>(ACTION_CRITICAL_HEAL + 0.5f)
            }
        )
    );

    // LOW
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastTreeFormAction>(ACTION_MEDIUM_HEAL + 1.5f),
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 1.4f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 1.3f),
                CreateNextAction<CastPartySwiftmendAction>(ACTION_MEDIUM_HEAL + 1.2f),
                CreateNextAction<CastPartyNourishAction>(ACTION_MEDIUM_HEAL + 1.1f),
            }
        )
    );

    // MEDIUM
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastTreeFormAction>(ACTION_MEDIUM_HEAL + 0.5f),
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 0.4f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 0.3f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 0.2f),
                CreateNextAction<CastPartyNourishAction>(ACTION_MEDIUM_HEAL + 0.1f)
            }
        )
    );

    // almost full
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_LIGHT_HEAL + 0.3f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_LIGHT_HEAL + 0.2f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_LIGHT_HEAL + 0.1f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastInnervateAction>(ACTION_HIGH + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell",
            {
                CreateNextAction<FleeAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
}
