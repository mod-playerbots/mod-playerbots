/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HunterBuffStrategies.h"
#include "CreateNextAction.h"
#include "HunterActions.h"

class BuffHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BuffHunterStrategyActionNodeFactory()
    {
        creators["aspect of the hawk"] = &aspect_of_the_hawk;
    }

private:
    static ActionNode* aspect_of_the_hawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastAspectOfTheMonkeyAction>(1.0f) },
            /*C*/ {}
        );
    }
};

HunterBuffDpsStrategy::HunterBuffDpsStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new BuffHunterStrategyActionNodeFactory());
}

void HunterBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aspect of the hawk",
            {
                CreateNextAction<CastAspectOfTheDragonhawkAction>(20.1f),
                CreateNextAction<CastAspectOfTheHawkAction>(20.0f)
            }
        )
    );
}

void HunterNatureResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aspect of the wild",
            {
                CreateNextAction<CastAspectOfTheWildAction>(20.0f)
            }
        )
    );
}

void HunterBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aspect of the pack",
            {
                CreateNextAction<CastAspectOfThePackAction>(20.0f)
            }
        )
    );
}

void HunterBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aspect of the viper",
            {
                CreateNextAction<CastAspectOfTheViperAction>(20.0f)
            }
        )
    );
}
