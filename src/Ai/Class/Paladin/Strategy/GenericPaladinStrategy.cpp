/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinStrategy.h"

#include "CreateNextAction.h"
#include "GenericPaladinStrategyActionNodeFactory.h"
#include "PaladinActions.h"

GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastDivineShieldAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hammer of justice interrupt",
            {
                CreateNextAction<CastHammerOfJusticeAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hammer of justice on enemy healer",
            {
                CreateNextAction<CastHammerOfJusticeOnEnemyHealerAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hammer of justice on snare target",
            {
                CreateNextAction<CastHammerOfJusticeSnareAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastLayOnHandsAction>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastLayOnHandsOnPartyAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "protect party member",
            {
                CreateNextAction<CastBlessingOfProtectionProtectAction>(ACTION_EMERGENCY + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high mana",
            {
                CreateNextAction<CastDivinePleaAction>(ACTION_HIGH)
            }
        )
    );
}

void PaladinCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "cleanse cure disease",
            {
                CreateNextAction<CastCleanseDiseaseAction>(ACTION_DISPEL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cleanse party member cure disease",
            {
                CreateNextAction<CastCleanseDiseaseOnPartyAction>(ACTION_DISPEL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cleanse cure poison",
            {
                CreateNextAction<CastCleansePoisonAction>(ACTION_DISPEL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cleanse party member cure poison",
            {
                CreateNextAction<CastCleansePoisonOnPartyAction>(ACTION_DISPEL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cleanse cure magic",
            {
                CreateNextAction<CastCleanseMagicAction>(ACTION_DISPEL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cleanse party member cure magic",
            {
                CreateNextAction<CastCleanseMagicOnPartyAction>(ACTION_DISPEL + 1.0f)
            }
        )
    );
}

void PaladinBoostStrategy::InitTriggers(std::vector<TriggerNode*>&)
{

}

void PaladinCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "turn undead",
            {
                CreateNextAction<CastTurnUndeadAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}

void PaladinHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "healer should attack",
            {
                CreateNextAction<CastHammerOfWrathAction>(ACTION_DEFAULT + 0.6f),
                CreateNextAction<CastHolyShockAction>(ACTION_DEFAULT + 0.5f),
                CreateNextAction<ShieldOfRighteousnessAction>(ACTION_DEFAULT + 0.4f),
                CreateNextAction<CastJudgementOfLightAction>(ACTION_DEFAULT + 0.3f),
                CreateNextAction<CastConsecrationAction>(ACTION_DEFAULT + 0.2f),
                CreateNextAction<CastExorcismAction>(ACTION_DEFAULT + 0.1f),
            }
        )
    );
}
