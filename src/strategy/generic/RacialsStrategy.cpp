/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RacialsStrategy.h"

#include "Playerbots.h"

class RacialsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RacialsStrategyActionNodeFactory() { creators["lifeblood"] = &lifeblood; }

private:
    static ActionNode* lifeblood(PlayerbotAI* botAI)
    {
        return new ActionNode("lifeblood",
                              /*P*/ {},
                              /*A*/ { new NextAction("gift of the naaru") },
                              /*C*/ {});
    }
};

void RacialsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("low health", { new NextAction("lifeblood", ACTION_NORMAL + 5) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { new NextAction("war stomp", ACTION_NORMAL + 5) }));
    triggers.push_back(new TriggerNode(
        "low mana", { new NextAction("arcane torrent", ACTION_NORMAL + 5) }));

    triggers.push_back(new TriggerNode(
        "generic boost", { new NextAction("blood fury", ACTION_NORMAL + 5),
        new NextAction("berserking", ACTION_NORMAL + 5),
        new NextAction("use trinket", ACTION_NORMAL + 4) }));

}

RacialsStrategy::RacialsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new RacialsStrategyActionNodeFactory());
}
