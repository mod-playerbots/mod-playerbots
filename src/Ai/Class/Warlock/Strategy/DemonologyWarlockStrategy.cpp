/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DemonologyWarlockStrategy.h"
#include "CreateNextAction.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "WarlockActions.h"

// ===== Action Node Factory =====
class DemonologyWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DemonologyWarlockStrategyActionNodeFactory()
    {
        creators["metamorphosis"] = &metamorphosis;
        creators["demonic empowerment"] = &demonic_empowerment;
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption_on_attacker;
        creators["immolate"] = &immolate;
        creators["immolate on attacker"] = &immolate_on_attacker;
        creators["incinerate"] = &incinerate;
        creators["soul fire"] = &soul_fire;
        creators["shadow bolt"] = &shadow_bolt;
        creators["life tap"] = &life_tap;
        creators["immolation aura"] = &immolation_aura;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption on attacker"] = &seed_of_corruption_on_attacker;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
        creators["demon charge"] = &demon_charge;
    }

private:
    static ActionNode* metamorphosis(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* demonic_empowerment(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* corruption(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* corruption_on_attacker(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* immolate(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* immolate_on_attacker(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* incinerate(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* soul_fire(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* shadow_bolt(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* life_tap(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* immolation_aura(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* shadowflame(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* seed_of_corruption(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* rain_of_fire(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* demon_charge(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

};

// ===== Single Target Strategy =====
DemonologyWarlockStrategy::DemonologyWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DemonologyWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> DemonologyWarlockStrategy::getDefaultActions()
{
    return {
       CreateNextAction<CastCorruptionAction>(5.5f),
       CreateNextAction<CastImmolateAction>(5.4f),
       CreateNextAction<CastShadowBoltAction>(5.3f),
       CreateNextAction<CastIncinerateAction>(5.2f),
       CreateNextAction<CastShootAction>(5.0f) };
}

// ===== Trigger Initialization ===
void DemonologyWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // High priority cooldowns
    triggers.push_back(
        new TriggerNode(
            "metamorphosis",
            {
                CreateNextAction<CastMetamorphosisAction>(28.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "demonic empowerment",
            {
                CreateNextAction<CastDemonicEmpowermentAction>(28.0f)
            }
        )
    );

    // Main DoT triggers for high uptime
    triggers.push_back(
        new TriggerNode(
            "corruption on attacker",
            {
                CreateNextAction<CastCorruptionOnAttackerAction>(19.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "immolate on attacker",
            {
                CreateNextAction<CastImmolateOnAttackerAction>(19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "corruption",
            {
                CreateNextAction<CastCorruptionAction>(18.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "immolate",
            {
                CreateNextAction<CastImmolateAction>(17.5f)
            }
        )
    );

    // Procs
    triggers.push_back(
        new TriggerNode(
            "decimation",
            {
                CreateNextAction<CastSoulFireAction>(17.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "molten core",
            {
                CreateNextAction<CastIncinerateAction>(16.5f)
            }
        )
    );

    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(
        new TriggerNode(
            "life tap glyph buff",
            {
                CreateNextAction<CastLifeTapAction>(29.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "life tap",
            {
                CreateNextAction<CastLifeTapAction>(5.1f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "meta melee flee check",
            {
                CreateNextAction<FleeAction>(39.0f)
            }
        )
    );
}

// Combat strategy to run to melee for Immolation Aura
// Enabled by default for the Demonology spec
// To enable, type "co +meta melee"
// To disable, type "co -meta melee"
MetaMeleeAoeStrategy::MetaMeleeAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void MetaMeleeAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "immolation aura active",
            {
                CreateNextAction<ReachMeleeAction>(25.5f),
                CreateNextAction<DemonChargeAction>(25.0f)
            }
        )
    );
}
