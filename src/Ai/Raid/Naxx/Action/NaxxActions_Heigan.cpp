/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NaxxActions.h"
#include "LastMovementValue.h"
#include "Playerbots.h"

bool HeiganDanceAction::MoveToWaypoint(uint32 index, float distance)
{
    if (index >= HeiganBossHelper::WaypointCount)
        return false;

    if (int32(index) != lastWaypoint)
    {
        // New safe spot: forget the previous (same priority) move so we are not stuck waiting for it,
        // and stop whatever we were casting.
        lastWaypoint = int32(index);
        AI_VALUE(LastMovement&, "last movement").Set(nullptr);
        if (bot->IsNonMeleeSpellCast(true))
            botAI->InterruptSpell();
    }

    return MoveInside(bot->GetMapId(), HeiganBossHelper::WaypointX[index], HeiganBossHelper::WaypointY[index],
                      bot->GetPositionZ(), distance, MovementPriority::MOVEMENT_COMBAT);
}

bool HeiganDanceAction::MoveToPlatform(float distance)
{
    if (lastWaypoint != -1)
    {
        lastWaypoint = -1;
        AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    }

    return MoveInside(bot->GetMapId(), HeiganBossHelper::PlatformX, HeiganBossHelper::PlatformY,
                      HeiganBossHelper::PlatformZ, distance, MovementPriority::MOVEMENT_COMBAT);
}

bool HeiganDanceMeleeAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    // The boss follows the main tank, so the tank first has to hold him before it can lead the dance.
    if (!helper.ShouldDance())
        return false;

    return MoveToWaypoint(helper.GetSafeWaypoint(), botAI->IsMainTank(bot) ? 0.5f : 1.5f);
}

bool HeiganDanceRangedAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    // Slow dance: the platform is safe from eruptions. Leave it right after the last slow eruption
    // so we are out of Plague Cloud range (and at the first fast spot) before the fast dance starts.
    if (helper.ShouldRangedHoldPlatform())
        return MoveToPlatform(2.0f);

    return MoveToWaypoint(helper.GetSafeWaypoint(), 1.5f);
}
