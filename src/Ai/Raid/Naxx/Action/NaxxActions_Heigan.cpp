/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NaxxActions.h"
#include "Playerbots.h"

bool HeiganDanceMeleeAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    if (!helper.IsPlatformPhase() && PlayerbotAI::IsMainTank(bot) && !AI_VALUE2(bool, "has aggro", "boss target"))
    {
        return false;
    }

    std::pair<float, float> const& safeSpot = helper.GetSafeWaypoint();
    float safeRadius = PlayerbotAI::IsMainTank(bot) ? 0.5f : 6.0f;

    if (!helper.IsPlatformPhase() && bot->IsWithinDist2d(safeSpot.first, safeSpot.second, safeRadius))
    {
        return false;
    }

    return MoveInside(bot->GetMapId(), safeSpot.first, safeSpot.second, bot->GetPositionZ(), 0,
                      MovementPriority::MOVEMENT_COMBAT);
}

bool HeiganDanceRangedAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    if (!helper.IsPlatformPhase())
    {
        std::pair<float, float> const& platform = helper.platform;
        if (bot->IsWithinDist2d(platform.first, platform.second, 1.5f))
        {
            return false;
        }
        if (MoveTo(bot->GetMapId(), platform.first, platform.second, 276.54f, false, false, false, false,
                   MovementPriority::MOVEMENT_COMBAT))
        {
            return true;
        }
        return MoveInside(bot->GetMapId(), platform.first, platform.second, 276.54f, 2.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    botAI->InterruptSpell();
    std::pair<float, float> const& safeSpot = helper.GetSafeWaypoint();
    return MoveInside(bot->GetMapId(), safeSpot.first, safeSpot.second, bot->GetPositionZ(), 0,
                      MovementPriority::MOVEMENT_COMBAT);
}
