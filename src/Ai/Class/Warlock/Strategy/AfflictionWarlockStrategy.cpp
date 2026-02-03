/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AfflictionWarlockStrategy.h"
#include "CreateNextAction.h"
#include "MovementActions.h"
#include "WarlockActions.h"

// ===== Action Node Factory =====
class AfflictionWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AfflictionWarlockStrategyActionNodeFactory()
    {
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption;
        creators["unstable affliction"] = &unstable_affliction;
        creators["unstable affliction on attacker"] = &unstable_affliction;
        creators["haunt"] = &haunt;
        creators["shadow bolt"] = &shadow_bolt;
        creators["drain soul"] = &drain_soul;
        creators["life tap"] = &life_tap;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption on attacker"] = &seed_of_corruption;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
    }

private:
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

    static ActionNode* unstable_affliction(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* unstable_affliction_on_attacker(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* haunt(PlayerbotAI*)
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

    static ActionNode* drain_soul(PlayerbotAI*)
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

};

// ===== Single Target Strategy =====
AfflictionWarlockStrategy::AfflictionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new AfflictionWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> AfflictionWarlockStrategy::getDefaultActions()
{
    return {
       CreateNextAction<CastCorruptionAction>(5.5f),
       CreateNextAction<CastUnstableAfflictionAction>(5.4f),
       CreateNextAction<CastHauntAction>(5.3f),
       CreateNextAction<CastShadowBoltAction>(5.2f),
       CreateNextAction<CastShootAction>(5.0f)
    };
}

// ===== Trigger Initialization ===
void AfflictionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

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
            "unstable affliction on attacker",
            {
                CreateNextAction<CastUnstableAfflictionOnAttackerAction>(19.0f)
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
            "unstable affliction",
            {
                CreateNextAction<CastUnstableAfflictionAction>(17.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "haunt",
            {
                CreateNextAction<CastHauntAction>(16.5f)
            }
        )
    );

    // Drain Soul as execute if target is low HP // Shadow Trance for free casts
    triggers.push_back(
        new TriggerNode(
            "shadow trance",
            {
                CreateNextAction<CastShadowBoltAction>(16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                CreateNextAction<CastDrainSoulAction>(15.5f)
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
            "enemy too close for spell",
            {
                CreateNextAction<FleeAction>(39.0f)
            }
        )
    );
}
