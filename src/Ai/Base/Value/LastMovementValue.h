/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LASTMOVEMENTVALUE_H
#define PLAYERBOTS_LASTMOVEMENTVALUE_H

#include "ObjectGuid.h"
#include "TravelNode.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

// High priority movement can override the previous low priority one
enum class MovementPriority
{
    MOVEMENT_IDLE,
    MOVEMENT_WANDER,
    MOVEMENT_NORMAL,
    MOVEMENT_COMBAT,
    MOVEMENT_FORCED
};

class LastMovement
{
public:
    LastMovement();
    LastMovement(LastMovement const& other);

    LastMovement& operator=(LastMovement const& other)
    {
        taxiNodes = other.taxiNodes;
        taxiMaster = other.taxiMaster;
        lastAreaTrigger = other.lastAreaTrigger;
        lastMoveShort = other.lastMoveShort;
        lastPath = other.lastPath;
        nextTeleport = other.nextTeleport;
        priority = other.priority;
        msTime = other.msTime;
        holdStartMs = other.holdStartMs;
        holdDurationMs = other.holdDurationMs;
        lastTransportEntry = other.lastTransportEntry;
        lastCompletedTransportEntry = other.lastCompletedTransportEntry;
        return *this;
    };

    void clear();

    void Set(Unit* follow);
    void Set(uint32 mapId, float x, float y, float z, float ori,
             MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    //Setting the hold is seperated from Set so that bots can be told to hold position without losing their last movement information.
    void SetHold(uint32 durationMs, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool IsHoldActive() const;

    void setShort(WorldPosition point);
    void setPath(TravelPath path);

    std::vector<uint32> taxiNodes;
    ObjectGuid taxiMaster;
    uint32 lastAreaTrigger;
    time_t lastFlee;
    WorldPosition lastMoveShort;
    uint32 msTime;
    MovementPriority priority;
    uint32 holdStartMs{0};
    uint32 holdDurationMs{0};
    TravelPath lastPath;
    time_t nextTeleport;
    // Entry of the transport the bot is currently aboard mid-journey,
    // used by WaitForTransport to resume a transport segment if the
    // bot is still on it next tick (e.g. boat in motion). 0 = none.
    uint32 lastTransportEntry{0};
    // Entry of the last transport whose ride ENDED (disembark, exit-scan
    // hop, or ride-gate reset). The proactive board-wait skips this entry
    // so the bot doesn't re-board the ship it just left when arrival-side
    // route points come into range. Sticky until the next ride completes
    // (a same-ship round trip within one order is the accepted blind spot).
    uint32 lastCompletedTransportEntry{0};
};

class LastMovementValue : public ManualSetValue<LastMovement&>
{
public:
    LastMovementValue(PlayerbotAI* botAI) : ManualSetValue<LastMovement&>(botAI, data) {}

private:
    LastMovement data = LastMovement();
};

class StayTimeValue : public ManualSetValue<time_t>
{
public:
    StayTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0) {}
};

#endif
