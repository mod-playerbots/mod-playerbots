/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LastMovementValue.h"
#include "Timer.h"

LastMovement::LastMovement() { clear(); }

LastMovement::LastMovement(LastMovement& other)
    : taxiNodes(other.taxiNodes),
      taxiMaster(other.taxiMaster),
      lastAreaTrigger(other.lastAreaTrigger),
      lastFlee(other.lastFlee)
{
    lastMoveShort = other.lastMoveShort;
    nextTeleport = other.nextTeleport;
    lastPath = other.lastPath;
    priority = other.priority;
    holdStartMs = other.holdStartMs;
    holdDurationMs = other.holdDurationMs;
    lastTransportEntry = other.lastTransportEntry;
    lastCompletedTransportEntry = other.lastCompletedTransportEntry;
}

void LastMovement::clear()
{
    lastMoveShort = WorldPosition();
    lastPath.clear();
    lastAreaTrigger = 0;
    lastFlee = 0;
    nextTeleport = 0;
    msTime = 0;
    priority = MovementPriority::MOVEMENT_NORMAL;
    holdStartMs = 0;
    holdDurationMs = 0;
    lastTransportEntry = 0;
}

void LastMovement::Set([[maybe_unused]] Unit* follow)
{
    // Legacy signature — `follow` is ignored (lastFollow field removed).
    // The function still serves callers that want a soft-reset:
    // clears short + path + hold, resets msTime/priority via the chain below.
    Set(0, 0.0f, 0.0f, 0.0f, 0.0f);
    SetHold(0);
    setShort(WorldPosition());
    setPath(TravelPath());
}

void LastMovement::Set(uint32 mapId, float x, float y, float z, float ori, MovementPriority pri)
{
    lastMoveShort = WorldPosition(mapId, x, y, z, ori);
    msTime = getMSTime();
    priority = pri;
}

void LastMovement::SetHold(uint32 durationMs, MovementPriority pri)
{
    holdStartMs = getMSTime();
    holdDurationMs = durationMs;
    if (durationMs)
        priority = pri;
}

bool LastMovement::IsHoldActive() const
{
    return holdDurationMs && getMSTime() - holdStartMs < holdDurationMs;
}

void LastMovement::setShort(WorldPosition point)
{
    lastMoveShort = point;
}

void LastMovement::setPath(TravelPath path) { lastPath = path; }
