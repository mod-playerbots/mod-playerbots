/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HolyPriestStrategy.h"
#include "CreateNextAction.h"
#include "GenericPriestStrategyActionNodeFactory.h"
#include "GenericSpellActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"

class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyPriestStrategyActionNodeFactory() { creators["smite"] = &smite; }

private:
    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShootAction>(1.0f) },
            /*C*/ {}
        );
    }
};

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* botAI) : HealPriestStrategy(botAI)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

std::vector<NextAction> HolyPriestStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastSmiteAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastManaBurnAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<CastStarshardsAction>(ACTION_DEFAULT)
    };
}

void HolyPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "holy fire",
            {
                CreateNextAction<CastHolyFireAction>(ACTION_NORMAL + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shadowfiend",
            {
                CreateNextAction<CastShadowfiendAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastShadowfiendAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastManaBurnAction>(ACTION_HIGH)
            }
        )
    );
}

HolyHealPriestStrategy::HolyHealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

std::vector<NextAction> HolyHealPriestStrategy::getDefaultActions()
{
    return { CreateNextAction<CastShootAction>(ACTION_DEFAULT) };
}

void HolyHealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_MEDIUM_HEAL + 9.0f),
                CreateNextAction<CastCircleOfHealingAction>(ACTION_MEDIUM_HEAL + 8.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                CreateNextAction<CastDivineHymnAction>(ACTION_CRITICAL_HEAL + 7.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastCircleOfHealingAction>(ACTION_CRITICAL_HEAL + 5.0f),
                CreateNextAction<CastPrayerOfHealingAction>(ACTION_CRITICAL_HEAL + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastGuardianSpiritOnPartyAction>(ACTION_CRITICAL_HEAL + 6.0f),
                CreateNextAction<CastPowerWordShieldOnPartyAction>(ACTION_CRITICAL_HEAL + 5.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_CRITICAL_HEAL + 3.0f),
                CreateNextAction<CastGreaterHealOnPartyAction>(ACTION_MEDIUM_HEAL + 2.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_CRITICAL_HEAL + 1.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastCircleOfHealingAction>(ACTION_MEDIUM_HEAL + 4.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_MEDIUM_HEAL + 3.0f),
                CreateNextAction<CastGreaterHealOnPartyAction>(ACTION_MEDIUM_HEAL + 2.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_MEDIUM_HEAL + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastCircleOfHealingAction>(ACTION_LIGHT_HEAL + 7.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_LIGHT_HEAL + 6.0f),
                CreateNextAction<CastGreaterHealOnPartyAction>(ACTION_MEDIUM_HEAL + 5.0f),
                CreateNextAction<CastFlashHealOnPartyAction>(ACTION_LIGHT_HEAL + 4.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastRenewOnPartyAction>(ACTION_LIGHT_HEAL + 2.0f),
                CreateNextAction<CastPrayerOfMendingAction>(ACTION_LIGHT_HEAL + 1.0f),
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
}
