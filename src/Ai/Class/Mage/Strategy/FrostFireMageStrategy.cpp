/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostFireMageStrategy.h"
#include "CreateNextAction.h"
#include "MageActions.h"

// ===== Action Node Factory =====
class FrostFireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostFireMageStrategyActionNodeFactory()
    {
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["fire blast"] = &fire_blast;
        creators["pyroblast"] = &pyroblast;
        creators["combustion"] = &combustion;
        creators["icy veins"] = &icy_veins;
        creators["scorch"] = &scorch;
        creators["living bomb"] = &living_bomb;
    }

private:
    static ActionNode* frostfire_bolt(PlayerbotAI*)
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

    static ActionNode* combustion(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* icy_veins(PlayerbotAI*)
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

};

// ===== Single Target Strategy =====
FrostFireMageStrategy::FrostFireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostFireMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> FrostFireMageStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastFrostfireBoltAction>(5.2f),
        CreateNextAction<CastFireBlastAction>(5.1f),  // cast during movement
        CreateNextAction<CastShootAction>(5.0f)
    };
}

// ===== Trigger Initialization =====
void FrostFireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
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
