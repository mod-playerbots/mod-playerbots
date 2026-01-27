/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RacialsStrategy.h"

#include "ActionNode.h"
#include "CreateNextAction.h"
#include "GenericSpellActions.h"

class RacialsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RacialsStrategyActionNodeFactory()
    {
        creators["lifeblood"] = &lifeblood;
    }

private:
    static ActionNode* lifeblood(PlayerbotAI*)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastGiftOfTheNaaruAction>(1.0f) },
            /*C*/ {}
        );
    }
};

void RacialsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastLifeBloodAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastWarStompAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastArcaneTorrentAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "generic boost",
            {
                CreateNextAction<CastBloodFuryAction>(ACTION_NORMAL + 5.0f),
                CreateNextAction<CastBerserkingAction>(ACTION_NORMAL + 5.0f),
                CreateNextAction<UseTrinketAction>(ACTION_NORMAL + 4.0f)
            }
        )
    );

}

RacialsStrategy::RacialsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new RacialsStrategyActionNodeFactory());
}
