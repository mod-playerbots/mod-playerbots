/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShadowPriestStrategy.h"

#include "CancelChannelAction.h"
#include "CreateNextAction.h"
#include "PriestActions.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

std::vector<NextAction> ShadowPriestStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastMindBlastAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastMindFlayAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastShadowWordDeathAction>(ACTION_DEFAULT + 0.1f), // cast during movement
        CreateNextAction<CastShootAction>(ACTION_DEFAULT)
    };
}

void ShadowPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "shadowform",
            {
                CreateNextAction<CastShadowformAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastDispersionAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastDispersionAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "vampiric embrace",
            {
                CreateNextAction<CastVampiricEmbraceAction>(16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "silence",
            {
                CreateNextAction<CastSilenceAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "silence on enemy healer",
            {
                CreateNextAction<CastSilenceOnEnemyHealerAction>(ACTION_INTERRUPT)
            }
        )
    );
}

void ShadowPriestAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "shadow word: pain on attacker",
            {
                CreateNextAction<CastPowerWordPainOnAttackerAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "vampiric touch on attacker",
            {
                CreateNextAction<CastVampiricTouchOnAttackerAction>(ACTION_NORMAL + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mind sear channel check",
            {
                CreateNextAction<CancelChannelAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastMindSearAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
}

void ShadowPriestDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "vampiric touch",
            {
                CreateNextAction<CastVampiricTouchAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "devouring plague",
            {
                CreateNextAction<CastDevouringPlagueAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shadow word: pain",
            {
                CreateNextAction<CastPowerWordPainAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}
