/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HunterBuffStrategies.h"

#include "Playerbots.h"

class BuffHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BuffHunterStrategyActionNodeFactory()
    {
        creators["aspect of the dragonhawk"] = &aspect_of_the_dragonhawk;
        creators["aspect of the hawk"] = &aspect_of_the_hawk;
        creators["aspect of the pack"] = &aspect_of_the_pack;
    }

private:
    static ActionNode* aspect_of_the_dragonhawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the dragonhawk",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the hawk") },
                              /*C*/ {});
    }
    static ActionNode* aspect_of_the_hawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the hawk",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the monkey") },
                              /*C*/ {});
    }
    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ {},
                              /*A*/ { NextAction("aspect of the cheetah") },
                              /*C*/ {});
    }
};

HunterBuffDpsStrategy::HunterBuffDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new BuffHunterStrategyActionNodeFactory());
}

void HunterBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "aspect of the dragonhawk",
            {
                NextAction("aspect of the dragonhawk", ACTION_HIGH)
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
                NextAction("aspect of the wild", ACTION_HIGH)
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
                NextAction("aspect of the pack", ACTION_HIGH)
            }
        )
    );
}
