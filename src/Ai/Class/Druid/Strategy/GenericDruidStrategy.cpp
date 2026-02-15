/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDruidStrategy.h"

#include "CreateNextAction.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "GenericActions.h"

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["caster form"] = &caster_form;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["rebirth"] = &rebirth;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["innervate"] = &innervate;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* caster_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* rebirth([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<UseManaPotion>(1.0f) },
            /*C*/ {}
        );
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastBarkskinAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combat party member dead",
            {
                CreateNextAction<CastRebirthAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "being attacked",
            {
                CreateNextAction<CastNaturesGraspAction>(ACTION_HIGH + 1.0f)
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
}

void DruidCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "party member cure poison",
            {
                CreateNextAction<CastAbolishPoisonOnPartyAction>(ACTION_DISPEL + 1.0f)
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
}

void DruidBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "nature's swiftness",
            {
                CreateNextAction<CastNaturesSwiftnessAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
}

void DruidCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "entangling roots",
            {
                CreateNextAction<CastEntanglingRootsCcAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "entangling roots kite",
            {
                CreateNextAction<CastEntanglingRootsAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hibernate",
            {
                CreateNextAction<CastHibernateCcAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
}

void DruidHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "healer should attack",
            {
                CreateNextAction<CastCancelTreeFormAction>(ACTION_DEFAULT + 0.3f),
                CreateNextAction<CastMoonfireAction>(ACTION_DEFAULT + 0.2f),
                CreateNextAction<CastWrathAction>(ACTION_DEFAULT + 0.1f),
                CreateNextAction<CastStarfireAction>(ACTION_DEFAULT),
            }
        )
    );
}
