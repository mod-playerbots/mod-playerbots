/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootTriggers.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool LootAvailableTrigger::IsActive()
{
    // Same gate as the "has available loot" value (!can loot && a lootable entry exists), but keep
    // the nearest entry so a closer target can replace a stale one.
    if (AI_VALUE(bool, "can loot"))
        return false;

    LootObject nearest = AI_VALUE(LootObject, "nearest loot");
    if (nearest.IsEmpty())
        return false;

    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    bool distanceCheck = false;
    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
    {
        distanceCheck =
            ServerFacade::instance().IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE);
    }
    else
    {
        distanceCheck = lootTarget.IsAtInteractDistance(bot);
    }

    // Loot target in range, or no hostile targets to deal with first.
    if (distanceCheck || AI_VALUE(GuidVector, "all targets").empty())
        return true;

    if (lootTarget.IsEmpty() || !lootTarget.IsLootPossible(bot))
        return true;

    // Re-run the loot action when the nearest entry differs from the target, instead of chasing
    // the old target past fresh corpses.
    return nearest.guid != lootTarget.guid;
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return !loot.IsAtInteractDistance(bot);
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
