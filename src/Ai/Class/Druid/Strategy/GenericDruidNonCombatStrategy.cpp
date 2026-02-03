/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDruidNonCombatStrategy.h"

#include "AiFactory.h"
#include "CreateNextAction.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "GenericActions.h"
#include "ImbueAction.h"

class GenericDruidNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidNonCombatStrategyActionNodeFactory()
    {
        creators["thorns"] = &thorns;
        creators["thorns on party"] = &thorns_on_party;
        creators["mark of the wild"] = &mark_of_the_wild;
        creators["mark of the wild on party"] = &mark_of_the_wild_on_party;
        // creators["innervate"] = &innervate;
        creators["regrowth_on_party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["revive"] = &revive;
    }

private:
    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* thorns_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mark_of_the_wild([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mark_of_the_wild_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* revive([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "mark of the wild",
            {
                CreateNextAction<CastMarkOfTheWildAction>(14.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member cure poison",
            {
                CreateNextAction<CastAbolishPoisonOnPartyAction>(20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member dead",
            {
                CreateNextAction<CastReviveAction>(ACTION_CRITICAL_HEAL + 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<ImbueWithOilAction>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 7.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 6.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 4.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 3.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 3.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 2.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 1.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_LIGHT_HEAL + 3.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_LIGHT_HEAL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member remove curse",
            {
                CreateNextAction<CastDruidRemoveCurseOnPartyAction>(ACTION_DISPEL + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "new pet",
            {
                CreateNextAction<SetPetStanceAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 7.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 6.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 4.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 3.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 3.0f),
                CreateNextAction<CastRegrowthOnPartyAction>(ACTION_MEDIUM_HEAL + 2.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_MEDIUM_HEAL + 1.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastWildGrowthOnPartyAction>(ACTION_LIGHT_HEAL + 3.0f),
                CreateNextAction<CastRejuvenationOnPartyAction>(ACTION_LIGHT_HEAL + 2.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member remove curse",
            {
                CreateNextAction<CastDruidRemoveCurseOnPartyAction>(ACTION_DISPEL + 7.0f),
            }
        )
    );

    int specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (specTab == 0 || specTab == 2) // Balance or Restoration
        triggers.push_back(new TriggerNode("often", { CreateNextAction<ImbueWithOilAction>(1.0f) }));
    if (specTab == 1) // Feral
        triggers.push_back(new TriggerNode("often", { CreateNextAction<ImbueWithStoneAction>(1.0f) }));

}

GenericDruidBuffStrategy::GenericDruidBuffStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "mark of the wild on party",
            {
                CreateNextAction<CastMarkOfTheWildOnPartyAction>(13.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "thorns on main tank",
            {
                CreateNextAction<CastThornsOnMainTankAction>(11.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "thorns",
            {
                CreateNextAction<CastThornsAction>(10.0f),
            }
        )
    );
}
