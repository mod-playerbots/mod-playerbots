/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterStrategy.h"
#include "CancelChannelAction.h"
#include "CreateNextAction.h"
#include "EquipAction.h"
#include "GenericActions.h"
#include "HunterActions.h"

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["aspect of the dragonhawk"] = &aspect_of_the_dragonhawk;
        creators["feign death"] = &feign_death;
        creators["wing clip"] = &wing_clip;
        creators["mongoose bite"] = &mongoose_bite;
        creators["raptor strike"] = &raptor_strike;
        creators["explosive trap"] = &explosive_trap;
    }

private:
    static ActionNode* rapid_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastReadinessAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastAspectOfTheCheetahAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* aspect_of_the_dragonhawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastAspectOfTheHawkAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* feign_death([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* wing_clip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            {},
            /*C*/ {}
        );
    }

    static ActionNode* mongoose_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRaptorStrikeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* raptor_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* explosive_trap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastImmolationTrapAction>(1.0f) },
            /*C*/ {}
        );
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // Mark/Ammo/Mana Triggers
    triggers.push_back(
        new TriggerNode(
            "no ammo",
            {
                CreateNextAction<EquipUpgradesPacketAction>(30.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hunter's mark",
            {
                CreateNextAction<CastHuntersMarkAction>(29.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "rapid fire",
            {
                CreateNextAction<CastRapidFireAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "aspect of the viper",
            {
                CreateNextAction<CastAspectOfTheViperAction>(28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "aspect of the hawk",
            {
                CreateNextAction<CastAspectOfTheDragonhawkAction>(27.5f)
            }
        )
    );

    // Aggro/Threat/Defensive Triggers
    triggers.push_back(
        new TriggerNode(
            "has aggro",
            {
                CreateNextAction<CastConcussiveShotAction>(20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low tank threat",
            {
                CreateNextAction<CastMisdirectionOnMainTankAction>(27.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastDeterrenceAction>(35.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "concussive shot on snare target",
            {
                CreateNextAction<CastConcussiveShotAction>(20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastFeignDeathAction>(35.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hunters pet medium health",
            {
                CreateNextAction<CastMendPetAction>(22.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hunters pet low health",
            {
                CreateNextAction<CastMendPetAction>(21.0f)
            }
        )
    );

    // Dispel Triggers
    triggers.push_back(
        new TriggerNode(
            "tranquilizing shot enrage",
            {
                CreateNextAction<CastTranquilizingShotAction>(61.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "tranquilizing shot magic",
            {
                CreateNextAction<CastTranquilizingShotAction>(61.0f)
            }
        )
    );

    // Ranged-based Triggers
    triggers.push_back(
        new TriggerNode(
            "enemy within melee",
            {
                CreateNextAction<CastExplosiveTrapAction>(37.0f),
                CreateNextAction<CastMongooseBiteAction>(22.0f),
                CreateNextAction<CastWingClipAction>(21.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy too close for auto shot",
            {
                CreateNextAction<CastDisengageAction>(35.0f),
                CreateNextAction<FleeAction>(34.0f)
            }
        )
    );
}

// ===== AoE Strategy, 2/3+ enemies =====
AoEHunterStrategy::AoEHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void AoEHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "volley channel check",
            {
                CreateNextAction<CancelChannelAction>(23.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastVolleyAction>(22.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastMultiShotAction>(21.0f)
            }
        )
    );
}

void HunterBoostStrategy::InitTriggers(std::vector<TriggerNode*>&)
{
}

void HunterCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "scare beast",
            {
                CreateNextAction<CastScareBeastCcAction>(23.0f)
            }
        )
    );
    // @TODO: This has always been broken because there is no such action.
    // triggers.push_back(
    //     new TriggerNode(
    //         "freezing trap",
    //         {
    //             CreateNextAction("freezing trap on cc", 23.0f)
    //         }
    //     )
    // );
}

void HunterTrapWeaveStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "immolation trap no cd",
            {
                CreateNextAction<MeleeAction>(23.0f)
            }
        )
    );
}
