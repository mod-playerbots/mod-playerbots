/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRAVELORDERVALUE_H
#define PLAYERBOTS_TRAVELORDERVALUE_H

#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

// A master-issued long-range travel order ("travel <map> <x> <y> <z>" whisper).
// Drives the bot through the unified movement funnel every tick until arrival,
// cancel ("travel stop") or give-up. Progress/milestone fields let the driver
// whisper state changes and detect a genuinely stuck order without a timer on
// legitimate waiting states (dock wait, taxi ride).
class TravelOrder
{
public:
    WorldPosition dest;
    bool active = false;

    // Give-up accounting: closest distance reached so far and when it last
    // improved by at least progressEpsilon.
    float bestDist = 0.0f;
    uint32 lastProgressMs = 0;
    uint32 startedMs = 0;
    uint32 failedResolves = 0;

    // Milestone snapshots (whisper on change).
    uint32 lastMapId = 0;
    uint32 lastTransportEntry = 0;
    bool wasInFlight = false;
    bool routeAnnounced = false;

    // Whether the order enabled "debug move" itself (restored on completion).
    bool enabledDebugMove = false;

    void Clear() { *this = TravelOrder(); }
};

class TravelOrderValue : public ManualSetValue<TravelOrder&>
{
public:
    TravelOrderValue(PlayerbotAI* botAI) : ManualSetValue<TravelOrder&>(botAI, data, "travel order") {}

private:
    TravelOrder data = TravelOrder();
};

#endif
