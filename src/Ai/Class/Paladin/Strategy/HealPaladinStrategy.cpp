/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HealPaladinStrategy.h"
#include "CreateNextAction.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"

class HealPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
};

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new HealPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> HealPaladinStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastJudgementOfLightAction>(ACTION_DEFAULT)
    };
}

void HealPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                CreateNextAction<CastSealOfWisdomAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastDivineIlluminationAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastDivineFavorAction>(ACTION_HIGH + 1.0f)
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
            "medium group heal setting",
            {
                CreateNextAction<CastDivineSacrificeAction>(ACTION_CRITICAL_HEAL + 5.0f),
                CreateNextAction<CastAvengingWrathAction>(ACTION_HIGH + 4),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastHolyShockOnPartyAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastDivineSacrificeAction>(ACTION_CRITICAL_HEAL + 5.0f),
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
                CreateNextAction<CastHolyLightOnPartyAction>(ACTION_LIGHT_HEAL + 9.0f),
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
            "beacon of light on main tank",
            {
                CreateNextAction<CastBeaconOfLightOnMainTankAction>(ACTION_CRITICAL_HEAL + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sacred shield on main tank",
            {
                CreateNextAction<CastSacredShieldOnMainTankAction>(ACTION_CRITICAL_HEAL + 6.0f)
            }
        )
);
}
