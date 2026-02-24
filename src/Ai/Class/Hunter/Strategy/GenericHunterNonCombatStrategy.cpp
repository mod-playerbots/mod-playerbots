/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterNonCombatStrategy.h"
#include "CreateNextAction.h"
#include "EquipAction.h"
#include "GenericActions.h"
#include "HunterActions.h"
#include "ImbueAction.h"

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterNonCombatStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
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
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "trueshot aura",
            {
                CreateNextAction<CastTrueshotAuraAction>(2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<ImbueWithStoneAction>(1.0f),
                CreateNextAction<ImbueWithOilAction>(1.0f),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low ammo",
            {
                CreateNextAction<SayLowAmmoAction>(ACTION_NORMAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no track",
            {
                CreateNextAction<CastTrackHumanoidsAction>(ACTION_NORMAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no ammo",
            {
                CreateNextAction<EquipUpgradesPacketAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}

void HunterPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastCallPetAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "has pet",
            {
                CreateNextAction<TogglePetSpellAutoCastAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "new pet",
            {
                CreateNextAction<SetPetStanceAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pet not happy",
            {
                CreateNextAction<FeedPetAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hunters pet medium health",
            {
                CreateNextAction<CastMendPetAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "hunters pet dead",
            {
                CreateNextAction<CastRevivePetAction>(60.0f)
            }
        )
    );
}
