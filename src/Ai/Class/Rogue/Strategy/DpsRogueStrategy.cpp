/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DpsRogueStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "RogueComboActions.h"
#include "RogueFinishingActions.h"
#include "RogueOpeningActions.h"

class DpsRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["backstab"] = &backstab;
        creators["melee"] = &melee;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastMutilateAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSinisterStrikeAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* kick([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastKidneyShotAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* kidney_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastMutilateAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* rupture([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastEviscerateAction>(1.0f) },
            /*C*/ {}
        );
    }
};

DpsRogueStrategy::DpsRogueStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new DpsRogueStrategyActionNodeFactory());
}

std::vector<NextAction> DpsRogueStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastKillingSpreeAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void DpsRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                CreateNextAction<CastGarroteAction>(ACTION_HIGH + 7.0f),
                CreateNextAction<CastAmbushAction>(ACTION_HIGH + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                CreateNextAction<CastSinisterStrikeAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "slice and dice",
            {
                CreateNextAction<CastSliceAndDiceAction>(ACTION_HIGH + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "combo points available",
            {
                CreateNextAction<CastRuptureAction>(ACTION_HIGH + 1.0f),
                CreateNextAction<CastEviscerateAction>(ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                CreateNextAction<CastEviscerateAction>(ACTION_HIGH + 0.2f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastVanishAction>(ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastEvasionAction>(ACTION_HIGH + 9.0f),
                CreateNextAction<CastFeintAction>(ACTION_HIGH + 8.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastCloakOfShadowsAction>(ACTION_HIGH + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick",
            {
                CreateNextAction<CastKickAction>(ACTION_INTERRUPT + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                CreateNextAction<CastKickOnEnemyHealerAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastBladeFlurryAction>(ACTION_HIGH + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "blade flurry",
                {
                CreateNextAction<CastBladeFlurryAction>(ACTION_HIGH + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastStealthAction>(ACTION_HIGH + 3.0f),
                CreateNextAction<CastSprintAction>(ACTION_HIGH + 2.0f),
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "expose armor",
            {
                CreateNextAction<CastExposeArmorAction>(ACTION_HIGH + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low tank threat",
            {
                CreateNextAction<CastTricksOfTheTradeOnMainTankAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
}

class StealthedRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    StealthedRogueStrategyActionNodeFactory()
    {
        creators["ambush"] = &ambush;
        creators["cheap shot"] = &cheap_shot;
        creators["garrote"] = &garrote;
        creators["sap"] = &sap;
        creators["sinister strike"] = &sinister_strike;
    }

private:
    static ActionNode* ambush([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastGarroteAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* cheap_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* garrote([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* sap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastCheapShotAction>(1.0f) },
            /*C*/ {}
        );
    }
};

StealthedRogueStrategy::StealthedRogueStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new StealthedRogueStrategyActionNodeFactory());
}

std::vector<NextAction> StealthedRogueStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastAmbushAction>(ACTION_NORMAL + 4),
        CreateNextAction<CastBackstabAction>(ACTION_NORMAL + 3),
        CreateNextAction<CastCheapShotAction>(ACTION_NORMAL + 2),
        CreateNextAction<CastSinisterStrikeAction>(ACTION_NORMAL + 1),
        CreateNextAction<MeleeAction>(ACTION_NORMAL)
    };
}

void StealthedRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "combo points available",
            {
                CreateNextAction<CastEviscerateAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kick",
            {
                CreateNextAction<CastCheapShotAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                CreateNextAction<CastCheapShotAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "behind target",
            {
                CreateNextAction<CastAmbushAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not behind target",
            {
                CreateNextAction<CastCheapShotAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy flagcarrier near",
            {
                CreateNextAction<CastSprintAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unstealth",
            {
                CreateNextAction<UnstealthAction>(ACTION_NORMAL)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "no stealth",
            {
                CreateNextAction<CheckStealthAction>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sprint",
            {
                CreateNextAction<CastSprintAction>(ACTION_INTERRUPT)
            }
        )
    );
}

void StealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "stealth",
            {
                CreateNextAction<CastStealthAction>(ACTION_INTERRUPT)
            }
        )
    );
}

void RogueAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastBladeFlurryAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<FanOfKnivesAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
}

void RogueBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "adrenaline rush",
            {
                CreateNextAction<CastAdrenalineRushAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
}

void RogueCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "sap",
            {
                CreateNextAction<CastStealthAction>(ACTION_INTERRUPT),
                CreateNextAction<CastSapAction>(ACTION_INTERRUPT)
            }
        )
    );
}
