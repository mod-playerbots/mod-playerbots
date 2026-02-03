/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPriestStrategy.h"

#include "CreateNextAction.h"
#include "GenericActions.h"
#include "GenericPriestStrategyActionNodeFactory.h"
#include "HealPriestStrategy.h"
#include "ImbueAction.h"
#include "MovementActions.h"
#include "PriestActions.h"

GenericPriestStrategy::GenericPriestStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

void GenericPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastFadeAction>(55.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastDesperatePrayerAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastPowerWordShieldAction>(ACTION_NORMAL)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastPowerWordShieldAction>(ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastShadowfiendAction>(ACTION_HIGH + 2),
                CreateNextAction<CastInnerFocusAction>(ACTION_HIGH + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastHymnOfHopeAction>(ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell",
            {
                CreateNextAction<FleeAction>(ACTION_MOVE + 9.0f)
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
            "being attacked",
            {
                CreateNextAction<CastPowerWordShieldAction>(ACTION_HIGH + 1.0f)
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

PriestCureStrategy::PriestCureStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new CurePriestStrategyActionNodeFactory());
}

void PriestCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "dispel magic",
            {
                CreateNextAction<CastDispelMagicAction>(41.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dispel magic on party",
            {
                CreateNextAction<CastDispelMagicOnPartyAction>(40.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cure disease",
            {
                CreateNextAction<CastAbolishDiseaseAction>(31.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "party member cure disease",
        {
            CreateNextAction<CastAbolishDiseaseOnPartyAction>(30.0f)
        }
    )
);
}

void PriestBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "power infusion",
            {
                CreateNextAction<CastPowerInfusionAction>(41.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "boost",
            {
                CreateNextAction<CastShadowfiendAction>(20.0f)
            }
        )
    );
}

void PriestCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "shackle undead",
            {
                CreateNextAction<CastShackleUndeadAction>(31.0f)
            }
        )
    );
}

void PriestHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "healer should attack",
            {
                CreateNextAction<CastPowerWordPainAction>(ACTION_DEFAULT + 0.5f),
                CreateNextAction<CastHolyFireAction>(ACTION_DEFAULT + 0.4f),
                CreateNextAction<CastSmiteAction>(ACTION_DEFAULT + 0.3f),
                CreateNextAction<CastMindBlastAction>(ACTION_DEFAULT + 0.2f),
                CreateNextAction<CastShootAction>(ACTION_DEFAULT)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium aoe and healer should attack",
            {
                CreateNextAction<CastMindSearAction>(ACTION_DEFAULT + 0.5f)
            }
        )
    );
}
