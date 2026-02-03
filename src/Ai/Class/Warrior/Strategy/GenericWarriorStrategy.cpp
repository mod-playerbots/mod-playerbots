/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericWarriorStrategy.h"
#include "CreateNextAction.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{

}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}

class WarrirorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarrirorAoeStrategyActionNodeFactory()
    {

    }

private:

};

WarrirorAoeStrategy::WarrirorAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new WarrirorAoeStrategyActionNodeFactory());
}

void WarrirorAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastSweepingStrikesAction>(ACTION_HIGH + 7.0f),
                CreateNextAction<CastBladestormAction>(ACTION_HIGH + 6.0f),
                CreateNextAction<CastThunderClapAction>(ACTION_HIGH + 5.0f),
                CreateNextAction<CastShockwaveAction>(ACTION_HIGH + 4.0f),
                CreateNextAction<CastDemoralizingShoutWithoutLifeTimeCheckAction>(ACTION_HIGH + 1.0f),
                CreateNextAction<CastCleaveAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shockwave on snare target",
            {
                CreateNextAction<CastShockwaveSnareAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
}
