/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericMageNonCombatStrategy.h"
#include "AiFactory.h"
#include "CreateNextAction.h"
#include "ImbueAction.h"
#include "MageActions.h"

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }

private:
    static ActionNode* molten_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastMageArmorAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* mage_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastIceArmorAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* ice_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFrostArmorAction>(1.0f) },
            /*C*/ {}
        );
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "arcane intellect",
            {
                CreateNextAction<CastArcaneIntellectAction>(21.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no focus magic",
            {
                CreateNextAction<CastFocusMagicOnPartyAction>(19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<ImbueWithOilAction>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no mana gem",
            {
                CreateNextAction<CastConjureManaGemAction>(20.0f)
            }
        )
    );
}

void MageBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "mage armor",
            {
                CreateNextAction<CastMageArmorAction>(19.0f)
            }
        )
    );
}

void MageBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "mage armor",
            {
                CreateNextAction<CastMoltenArmorAction>(19.0f)
            }
        )
    );
}

void MageBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "arcane intellect on party",
            {
                CreateNextAction<CastArcaneIntellectOnPartyAction>(20.0f)
            }
        )
    );
}
