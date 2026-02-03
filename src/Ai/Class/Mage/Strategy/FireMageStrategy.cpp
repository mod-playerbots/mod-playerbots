/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FireMageStrategy.h"
#include "CreateNextAction.h"
#include "MageActions.h"
#include "ReachTargetActions.h"

// ===== Action Node Factory =====
class FireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FireMageStrategyActionNodeFactory()
    {
        creators["fireball"] = &fireball;
        creators["frostbolt"] = &frostbolt;
        creators["fire blast"] = &fire_blast;
        creators["pyroblast"] = &pyroblast;
        creators["scorch"] = &scorch;
        creators["living bomb"] = &living_bomb;
        creators["combustion"] = &combustion;
    }

private:
    static ActionNode* fireball(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* frostbolt(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* fire_blast(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* pyroblast(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* scorch(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* living_bomb(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* combustion(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

};

// ===== Single Target Strategy =====
FireMageStrategy::FireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FireMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> FireMageStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastFireballAction>(5.3f),
        CreateNextAction<CastFrostboltAction>(5.2f),   // fire immune target
        CreateNextAction<CastFireBlastAction>(5.1f),  // cast during movement
        CreateNextAction<CastShootAction>(5.0f)
    };
}

// ===== Trigger Initialization =====
void FireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Debuff Triggers
    triggers.push_back(
        new TriggerNode(
            "improved scorch",
            {
                CreateNextAction<CastScorchAction>(19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "living bomb",
            {
                CreateNextAction<CastLivingBombAction>(18.5f)
            }
        )
    );

    // Proc Trigger
    triggers.push_back(
        new TriggerNode(
            "hot streak",
            {
                CreateNextAction<CastPyroblastAction>(25.0f)
            }
        )
    );
}

// Combat strategy to run to melee for Dragon's Breath and Blast Wave
// Disabled by default for the Fire/Frostfire spec
// To enable, type "co +firestarter"
// To disable, type "co -firestarter"
FirestarterStrategy::FirestarterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void FirestarterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blast wave off cd and medium aoe",
            {
                CreateNextAction<ReachMeleeAction>(25.5f)
            }
        )
    );
}
