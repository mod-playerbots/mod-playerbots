/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EnhancementShamanStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

// ===== Action Node Factory =====
class EnhancementShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    EnhancementShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["feral spirit"] = &feral_spirit;
        creators["lightning bolt"] = &lightning_bolt;
        creators["earth shock"] = &earth_shock;
        creators["flame shock"] = &flame_shock;
        creators["shamanistic rage"] = &shamanistic_rage;
        creators["call of the elements"] = &call_of_the_elements;
        creators["lightning shield"] = &lightning_shield;
    }

private:
    static ActionNode* stormstrike(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* feral_spirit(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* lightning_bolt(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* earth_shock(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* flame_shock(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* shamanistic_rage(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* call_of_the_elements(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* lightning_shield(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
};

// ===== Single Target Strategy =====
EnhancementShamanStrategy::EnhancementShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new EnhancementShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> EnhancementShamanStrategy::getDefaultActions()
{
    return {
       CreateNextAction<CastStormstrikeAction>(5.5f),
       CreateNextAction<CastFeralSpiritAction>(5.4f),
       CreateNextAction<CastEarthShockAction>(5.3f),
       CreateNextAction<CastLavaLashAction>(5.2f),
       CreateNextAction<MeleeAction>(5.0f)
    };
}

// ===== Trigger Initialization ===
void EnhancementShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Trigger
    triggers.push_back(
        new TriggerNode(
            "call of the elements and enemy within melee",
            {
                CreateNextAction<CastCallOfTheElementsAction>(60.0f)
            }
        )
    );

    // Spirit Walk Trigger
    triggers.push_back(
        new TriggerNode(
            "spirit walk ready",
            {
                CreateNextAction<CastSpiritWalkAction>(50.0f)
            }
        )
    );

    // Damage Triggers
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<ReachMeleeAction>(40.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "maelstrom weapon 5",
            {
                CreateNextAction<CastLightningBoltAction>(20.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "maelstrom weapon 4",
            {
                CreateNextAction<CastLightningBoltAction>(19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flame shock",
            {
                CreateNextAction<CastFlameShockAction>(19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lightning shield",
            {
                CreateNextAction<CastLightningShieldAction>(18.5f)
            }
        )
    );

    // Health/Mana Triggers
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastShamanisticRageAction>(23.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastShamanisticRageAction>(23.0f)
            }
        )
    );
}
