/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GATHERINGSESSIONVALUE_H
#define PLAYERBOTS_GATHERINGSESSIONVALUE_H

#include "Value.h"

// Finite state machine driving the "gather leveling" behaviour. Stored per
// bot in the "gathering session" value so the update action, the trainer
// action and the trigger can share and mutate it between engine ticks.
enum class GatheringLevelingState : uint8
{
    DISABLED = 0,  // not running
    TO_TRAINER,    // travelling to / using a trainer (obtain or rank-up)
    GATHERING,     // roaming the current zone, harvesting via the 'gather' strategy
    FINISHED       // 30 min elapsed or profession maxed
};

struct GatheringSession
{
    GatheringLevelingState state = GatheringLevelingState::DISABLED;
    uint32 skillId = 0;       // SKILL_MINING / SKILL_HERBALISM / SKILL_SKINNING (0 = none)
    uint32 startTime = 0;     // getMSTime() when the session began
    uint32 nextRoamTime = 0;  // throttle between roam waypoints
    bool started = false;     // session initialised for this bot
};

class GatheringSessionValue : public ManualSetValue<GatheringSession>
{
public:
    GatheringSessionValue(PlayerbotAI* botAI, std::string const name = "gathering session")
        : ManualSetValue<GatheringSession>(botAI, GatheringSession(), name)
    {
    }
};

#endif
