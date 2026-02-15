/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankPaladinStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankPaladinStrategyActionNodeFactory()
    {
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["taunt spell"] = &hand_of_reckoning;
    }

private:
    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfCorruptionAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfVengeanceAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfRighteousnessAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* hand_of_reckoning([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRighteousDefenseAction>(1.0f) },
            /*C*/ {}
        );
    }
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> TankPaladinStrategy::getDefaultActions()
{
    return {
        CreateNextAction<ShieldOfRighteousnessAction>(ACTION_DEFAULT + 0.6f),
        CreateNextAction<CastHammerOfTheRighteousAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastJudgementOfWisdomAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                CreateNextAction<CastSealOfCorruptionAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastSealOfWisdomAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
    triggers.push_back(new TriggerNode(
        "light aoe",
        {
            CreateNextAction<CastAvengersShieldAction>(ACTION_HIGH + 5.0f)
        }
    )
);
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastConsecrationAction>(ACTION_HIGH + 7.0f),
                CreateNextAction<CastAvengersShieldAction>(ACTION_HIGH + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                CreateNextAction<CastHandOfReckoningAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium health",
                { CreateNextAction<CastHolyShieldAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastHolyShieldAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastHolyShieldAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "avenging wrath",
        {
            CreateNextAction<CastAvengingWrathAction>(ACTION_HIGH + 2.0f)
        }
    )
);
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                CreateNextAction<CastHammerOfWrathAction>(ACTION_CRITICAL_HEAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "righteous fury",
            {
                CreateNextAction<CastRighteousFuryAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                CreateNextAction<CastDivineSacrificeAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enough mana",
            {
                CreateNextAction<CastConsecrationAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                CreateNextAction<SetFacingTargetAction>(ACTION_NORMAL + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}
