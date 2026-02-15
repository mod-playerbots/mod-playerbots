/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OffhealRetPaladinStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"

class OffhealRetPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    OffhealRetPaladinStrategyActionNodeFactory()
    {
        creators["retribution aura"] = &retribution_aura;
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
        creators["divine plea"] = &divine_plea;
    }

private:
    static ActionNode* retribution_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastDevotionAuraAction>(1.0f) },
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
            /*A*/ { CreateNextAction<CastSealOfCommandAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfRighteousnessAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastBlessingOfKingsAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* crusader_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* divine_plea([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

OffhealRetPaladinStrategy::OffhealRetPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new OffhealRetPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> OffhealRetPaladinStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastHammerOfWrathAction>(ACTION_DEFAULT + 0.6f),
        CreateNextAction<CastJudgementOfWisdomAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastCrusaderStrikeAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastDivineStormAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void OffhealRetPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    // Damage Triggers
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
                CreateNextAction<CastSealOfWisdomAction>(ACTION_HIGH + 5.0f),
                CreateNextAction<CastDivinePleaAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "art of war",
            {
                CreateNextAction<CastExorcismAction>(ACTION_HIGH + 1.0f)
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
            "medium aoe",
            {
                CreateNextAction<CastDivineStormAction>(ACTION_HIGH + 4.0f),
                CreateNextAction<CastConsecrationAction>(ACTION_HIGH + 3.0f)
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
    triggers.push_back(
        new TriggerNode(
            "retribution aura",
            {
                CreateNextAction<CastRetributionAuraAction>(ACTION_NORMAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "blessing of might",
            {
                CreateNextAction<CastBlessingOfMightAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "low health",
            {
                CreateNextAction<CastHolyLightAction>(ACTION_CRITICAL_HEAL + 2.0f)
            }
        )
    );

    // Healing Triggers
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastHolyShockOnPartyAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastHolyLightOnPartyAction>(ACTION_CRITICAL_HEAL + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastHolyLightOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastFlashOfLightOnPartyAction>(ACTION_LIGHT_HEAL + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastFlashOfLightOnPartyAction>(ACTION_LIGHT_HEAL + 3.0f)
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
            "beacon of light on main tank",
            {
                CreateNextAction<CastBeaconOfLightOnMainTankAction>(ACTION_CRITICAL_HEAL + 7.0f)
            }
        )
    );
}
