/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HealPriestStrategy.h"

#include "CreateNextAction.h"
#include "GenericPriestStrategyActionNodeFactory.h"
#include "GenericSpellActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"

HealPriestStrategy::HealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

std::vector<NextAction> HealPriestStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastShootAction>(ACTION_DEFAULT)
    };
}

void HealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_MEDIUM_HEAL + 8.0f),
                CreateNextAction<CastPowerWordShieldOnNotFullAction>(ACTION_MEDIUM_HEAL + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                CreateNextAction<CastDivineHymnAction>(ACTION_CRITICAL_HEAL + 7.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastPowerWordShieldOnNotFullAction>(ACTION_CRITICAL_HEAL + 5.0f),
                CreateNextAction<CastPrayerOfHealingAction>(ACTION_CRITICAL_HEAL + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastPowerWordShieldOnPartyAction>(ACTION_CRITICAL_HEAL + 5.0f),
                CreateNextAction<CastPenanceOnPartyAction>(ACTION_CRITICAL_HEAL + 4.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_CRITICAL_HEAL + 3.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_CRITICAL_HEAL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastPowerWordShieldOnPartyAction>(ACTION_MEDIUM_HEAL + 4.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_MEDIUM_HEAL + 3.0f),
                CreateNextAction<CastPenanceOnPartyAction>(ACTION_MEDIUM_HEAL + 2.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_MEDIUM_HEAL + 0.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastPowerWordShieldOnPartyAction>(ACTION_LIGHT_HEAL + 9.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_LIGHT_HEAL + 7.0f),
                CreateNextAction<CastPenanceOnPartyAction>(ACTION_LIGHT_HEAL + 6.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_LIGHT_HEAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_LIGHT_HEAL + 2.0f),
                CreateNextAction<CastRenewOnPartyAction>(ACTION_LIGHT_HEAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                CreateNextAction<ReachPartyMemberToHealAction>(ACTION_CRITICAL_HEAL + 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastPainSuppressionAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "protect party member",
            {
                CreateNextAction<CastPainSuppressionProtectAction>(ACTION_EMERGENCY)
            }
        )
    );
}
