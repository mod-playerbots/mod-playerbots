/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShamanNonCombatStrategy.h"
#include "AiFactory.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "ShamanActions.h"

class ShamanNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShamanNonCombatStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["windfury weapon"] = &windfury_weapon;
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["wind shear"] = &wind_shear;
        creators["purge"] = &purge;
    }

private:
    static ActionNode* flametongue_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRockbiterWeaponAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* frostbrand_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFlametongueWeaponAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* windfury_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFlametongueWeaponAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* earthliving_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFlametongueWeaponAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* wind_shear(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
    static ActionNode* purge(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }
};

ShamanNonCombatStrategy::ShamanNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new ShamanNonCombatStrategyActionNodeFactory());
}

void ShamanNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // Totemic Recall
    triggers.push_back(
        new TriggerNode(
            "totemic recall",
            {
                CreateNextAction<CastTotemicRecallAction>(60.0f),
            }
        )
    );

    // Healing/Resurrect Triggers
    triggers.push_back(
        new TriggerNode(
            "party member dead",
            {
                CreateNextAction<CastAncestralSpiritAction>(ACTION_CRITICAL_HEAL + 10.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                CreateNextAction<CastRiptideOnPartyAction>(31.0f),
                CreateNextAction<CastHealingWaveOnPartyAction>(30.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                CreateNextAction<CastRiptideOnPartyAction>(29.0f),
                CreateNextAction<CastHealingWaveOnPartyAction>(28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                CreateNextAction<CastRiptideOnPartyAction>(27.0f),
                CreateNextAction<CastHealingWaveOnPartyAction>(26.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                CreateNextAction<CastRiptideOnPartyAction>(25.0f),
                CreateNextAction<CastLesserHealingWaveOnPartyAction>(24.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                CreateNextAction<CastChainHealAction>(27.0f)
            }
        )
    );

    // Cure Triggers
    triggers.push_back(
        new TriggerNode(
            "cure poison",
            {
                CreateNextAction<CastCurePoisonActionSham>(21.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member cure poison",
            {
                CreateNextAction<CastCurePoisonOnPartyActionSham>(21.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "cure disease",
            {
                CreateNextAction<CastCureDiseaseActionSham>(31.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member cure disease",
            {
                CreateNextAction<CastCureDiseaseOnPartyActionSham>(30.0f),
            }
        )
    );

    // Out of Combat Buff Triggers
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(
            new TriggerNode(
                "main hand weapon no imbue",
                {
                    CreateNextAction<CastFlametongueWeaponAction>(22.0f),
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "water shield",
                {
                    CreateNextAction<CastWaterShieldAction>(21.0f),
                }
            )
        );
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(
            new TriggerNode(
                "main hand weapon no imbue",
                {
                    CreateNextAction<CastWindfuryWeaponAction>(22.0f),
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "off hand weapon no imbue",
                {
                    CreateNextAction<CastFlametongueWeaponAction>(21.0f),
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "lightning shield",
                {
                    CreateNextAction<CastLightningShieldAction>(20.0f),
                }
            )
        );
    }
    else if (tab == 2)  // Restoration
    {
        triggers.push_back(
            new TriggerNode(
                "main hand weapon no imbue",
                {
                    CreateNextAction<CastEarthlivingWeaponAction>(22.0f),
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "water shield",
                {
                    CreateNextAction<CastWaterShieldAction>(20.0f),
                }
            )
        );
    }

    // Buff Triggers while swimming
    triggers.push_back(
        new TriggerNode(
            "water breathing",
            {
                CreateNextAction<CastWaterBreathingAction>(12.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "water walking",
            {
                CreateNextAction<CastWaterWalkingAction>(12.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "water breathing on party",
            {
                CreateNextAction<CastWaterBreathingOnPartyAction>(11.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "water walking on party",
            {
                CreateNextAction<CastWaterWalkingOnPartyAction>(11.0f),
            }
        )
    );

    // Pet Triggers
    triggers.push_back(
        new TriggerNode(
            "has pet",
            {
                CreateNextAction<TogglePetSpellAutoCastAction>(60.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "new pet",
            {
                CreateNextAction<SetPetStanceAction>(65.0f),
            }
        )
    );
}

void ShamanNonCombatStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}
