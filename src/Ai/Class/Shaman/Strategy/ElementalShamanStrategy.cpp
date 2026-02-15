/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ElementalShamanStrategy.h"
#include "CreateNextAction.h"
#include "ShamanActions.h"

// ===== Action Node Factory =====
class ElementalShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ElementalShamanStrategyActionNodeFactory()
    {
        creators["flame shock"] = &flame_shock;
        creators["earth shock"] = &earth_shock;
        creators["lava burst"] = &lava_burst;
        creators["lightning bolt"] = &lightning_bolt;
        creators["call of the elements"] = &call_of_the_elements;
        creators["elemental mastery"] = &elemental_mastery;
        creators["stoneclaw totem"] = &stoneclaw_totem;
        creators["water shield"] = &water_shield;
        creators["thunderstorm"] = &thunderstorm;
    }

private:
    static ActionNode* flame_shock(PlayerbotAI*)
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
    static ActionNode* lava_burst(PlayerbotAI*)
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
    static ActionNode* call_of_the_elements(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* elemental_mastery(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* stoneclaw_totem(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* water_shield(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* thunderstorm(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
};

// ===== Single Target Strategy =====
ElementalShamanStrategy::ElementalShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new ElementalShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> ElementalShamanStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastLavaBurstAction>(5.2f),
        CreateNextAction<CastLightningBoltAction>(5.0f)
    };
}

// ===== Trigger Initialization ===
void ElementalShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Triggers
    triggers.push_back(
        new TriggerNode(
            "call of the elements",
            {
                CreateNextAction<CastCallOfTheElementsAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastStoneclawTotemAction>(40.0f)
            }
        )
    );

    // Cooldown Trigger
    triggers.push_back(
        new TriggerNode(
            "elemental mastery",
            {
                CreateNextAction<CastElementalMasteryAction>(29.0f)
            }
        )
    );

    // Damage Triggers
    triggers.push_back(
        new TriggerNode(
            "earth shock execute",
            {
                CreateNextAction<CastEarthShockAction>(5.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "flame shock",
            {
                CreateNextAction<CastFlameShockAction>(5.3f)
            }
        )
    );

    // Mana Triggers
    triggers.push_back(
        new TriggerNode(
            "water shield",
            {
                CreateNextAction<CastWaterShieldAction>(19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high mana",
            {
                CreateNextAction<CastThunderstormAction>(19.0f)
            }
        )
    );

    // Range Triggers
    triggers.push_back(
        new TriggerNode(
            "enemy is close",
            {
                CreateNextAction<CastThunderstormAction>(19.0f)
            }
        )
    );
}
