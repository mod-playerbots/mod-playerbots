/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinNonCombatStrategy.h"

#include "CreateNextAction.h"
#include "GenericPaladinStrategyActionNodeFactory.h"
#include "AiFactory.h"
#include "ImbueAction.h"
#include "PaladinActions.h"

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "party member dead",
            {
                CreateNextAction<CastRedemptionAction>(ACTION_CRITICAL_HEAL + 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastFlashOfLightOnPartyAction>(25.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastFlashOfLightOnPartyAction>(26.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastHolyLightOnPartyAction>(27.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastHolyLightOnPartyAction>(28.0f)
            }
        )
    );

    const uint8_t specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());

    // Holy or Protection
    if (specTab == PALADIN_TAB_HOLY || specTab == PALADIN_TAB_PROTECTION)
    {
        triggers.push_back(
            new TriggerNode(
                "often",
                {
                    CreateNextAction<ImbueWithOilAction>(1.0f)
                }
            )
        );
    }

    // Retribution
    if (specTab == PALADIN_TAB_RETRIBUTION)
    {
        triggers.push_back(
            new TriggerNode(
                "often",
                {
                    CreateNextAction<ImbueWithStoneAction>(1.0f)
                }
            )
        );
    }
}
