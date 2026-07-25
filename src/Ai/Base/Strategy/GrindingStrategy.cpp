/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GrindingStrategy.h"

#include "Playerbots.h"

std::vector<NextAction> GrindingStrategy::getDefaultActions()
{
    return {
        NextAction("drink", 4.2f),
        NextAction("food", 4.1f),
    };
}

void GrindingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // reduce lower than loot
    triggers.push_back(
        new TriggerNode(
            "no target",
            {
                NextAction("attack anything", 4.0f)
            }
        )
    );
}

void MoveRandomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                NextAction("move random", 1.5f)
            }
        )
    );
}
