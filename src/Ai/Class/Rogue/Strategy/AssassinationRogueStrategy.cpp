
#include "AssassinationRogueStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "RogueComboActions.h"
#include "RogueFinishingActions.h"
#include "RogueOpeningActions.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
        creators["backstab"] = &backstab;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastBackstabAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* envenom([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRuptureAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSinisterStrikeAction>(1.0f) },
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

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

std::vector<NextAction> AssassinationRogueStrategy::getDefaultActions()
{
    return {
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
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
                CreateNextAction<CastMutilateAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hunger for blood",
            {
                CreateNextAction<CastHungerForBloodAction>(ACTION_HIGH + 6.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "slice and dice",
            {
                CreateNextAction<CastSliceAndDiceAction>(ACTION_HIGH + 5.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "combo points 3 available",
            {
                CreateNextAction<CastEnvenomAction>(ACTION_HIGH + 5.0f),
                CreateNextAction<CastEviscerateAction>(ACTION_HIGH + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                CreateNextAction<CastEnvenomAction>(ACTION_HIGH + 4.0f),
                CreateNextAction<CastEviscerateAction>(ACTION_HIGH + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "expose armor",
            {
                CreateNextAction<CastExposeArmorAction>(ACTION_HIGH + 3.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastVanishAction>(ACTION_HIGH),
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
                CreateNextAction<CastKickAction>(ACTION_INTERRUPT + 2.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                CreateNextAction<CastKickOnEnemyHealerAction>(ACTION_INTERRUPT + 1.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<FanOfKnivesAction>(ACTION_NORMAL + 5.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low tank threat",
            {
                CreateNextAction<CastTricksOfTheTradeOnMainTankAction>(ACTION_HIGH + 7.0f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastStealthAction>(ACTION_HIGH + 3.0f),
                CreateNextAction<CastSprintAction>(ACTION_HIGH + 2.0f),
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f),
            }
        )
    );
}
