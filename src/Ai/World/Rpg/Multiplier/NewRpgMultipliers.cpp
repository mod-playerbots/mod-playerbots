/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewRpgMultipliers.h"
#include "NewRpgBaseAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

float NewRpgLootPriorityMultiplier::GetValue(Action* action)
{
    // Filter first: "has available loot" walks the loot stack, so only pay for it for the actions
    // this multiplier can actually block.
    if (!sPlayerbotAIConfig.LootPriority || !action || !dynamic_cast<NewRpgBaseAction*>(action))
        return 1.0f;

    // Unreachable targets are deferred by the move-to-loot watchdog, so no global fail-open is
    // needed here.
    return AI_VALUE(bool, "has available loot") ? 0.0f : 1.0f;
}
