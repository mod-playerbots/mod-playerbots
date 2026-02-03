/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankWarriorStrategy.h"
#include "GenericActions.h"

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["sunder armor"] = &sunder_armor;
        creators["commanding shout"] = &commanding_shout;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
        creators["heroic throw taunt"] = &heroic_throw_taunt;
        // creators["taunt"] = &taunt;
        // creators["taunt spell"] = &taunt;
        creators["vigilance"] = &vigilance;
        creators["enraged regeneration"] = &enraged_regeneration;
    }

private:
    static ActionNode* heroic_throw_taunt(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShieldSlamAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* heroic_throw_on_snare_target(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastTauntOnSnareTargetAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* last_stand(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastIntimidatingShoutAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* devastate(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSunderArmorAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* commanding_shout(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastBattleShoutAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* sunder_armor(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* charge(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<ReachMeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    // "heroic throw taunt" does not exist.
    // static ActionNode* taunt(PlayerbotAI*)
    // {
    //     return new ActionNode(
    //         /*P*/ {},
    //         /*A*/ { CreateNextAction("heroic throw taunt") },
    //         /*C*/ {}
    //     );
    // }

    static ActionNode* vigilance(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* enraged_regeneration(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

std::vector<NextAction> TankWarriorStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastDevastateAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastRevengeAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastDemoralizingShoutAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void TankWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "vigilance",
            {
                CreateNextAction<CastVigilanceAction>(ACTION_HIGH + 7.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<CastHeroicThrowAction>(ACTION_MOVE + 11.0f),
                CreateNextAction<CastChargeAction>(ACTION_MOVE + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thunder clap and rage",
            {
                CreateNextAction<CastThunderClapAction>(ACTION_MOVE + 11.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "defensive stance",
            {
                CreateNextAction<CastDefensiveStanceAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "commanding shout",
            {
                CreateNextAction<CastCommandingShoutAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "bloodrage",
            {
                CreateNextAction<CastBloodrageAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sunder armor",
            {
                CreateNextAction<CastDevastateAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium rage available",
            {
                CreateNextAction<CastShieldSlamAction>(ACTION_HIGH + 2.0f),
                CreateNextAction<CastDevastateAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shield block",
            {
                CreateNextAction<CastShieldBlockAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "revenge",
            {
                CreateNextAction<CastRevengeAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "disarm",
            {
                CreateNextAction<CastDisarmAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                CreateNextAction<CastTauntAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "taunt on snare target",
            {
                CreateNextAction<CastHeroicThrowSnareAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastShieldWallAction>(ACTION_MEDIUM_HEAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastLastStandAction>(ACTION_EMERGENCY + 3.0f),
                CreateNextAction<CastEnragedRegenerationAction>(ACTION_EMERGENCY + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high aoe",
            {
                CreateNextAction<CastChallengingShoutAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "concussion blow",
            {
                CreateNextAction<CastConcussionBlowAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shield bash",
            {
                CreateNextAction<CastShieldBashAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shield bash on enemy healer",
            {
                CreateNextAction<CastShieldBashOnEnemyHealerAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "spell reflection",
            {
                CreateNextAction<CastSpellReflectionAction>(ACTION_INTERRUPT + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "victory rush",
            {
                CreateNextAction<CastVictoryRushAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sword and board",
            {
                CreateNextAction<CastShieldSlamAction>(ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rend",
            {
                CreateNextAction<CastRendAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
        "rend on attacker",
            {
                CreateNextAction<CastRendOnAttackerAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "protect party member",
            {
                CreateNextAction<CastInterveneAction>(ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high rage available",
            {
                CreateNextAction<CastHeroicStrikeAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium rage available",
            {
                CreateNextAction<CastThunderClapAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}
