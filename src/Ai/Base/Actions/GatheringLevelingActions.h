/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GATHERINGLEVELINGACTIONS_H
#define PLAYERBOTS_GATHERINGLEVELINGACTIONS_H

#include "GatheringSessionValue.h"
#include "MovementActions.h"

class PlayerbotAI;
class ObjectGuid;

// Heartbeat of the "gather leveling" behaviour. Runs each non-combat tick while
// the strategy is active and drives the state machine stored in the
// "gathering session" value:
//   * obtains / rank-up the collecting profession via a trainer,
//   * keeps the 'gather' strategy active so nearby nodes are harvested,
//   * roams the current zone to find nodes,
//   * finishes after GatherLevelingDurationMinutes or when the profession maxes.
class GatheringLevelingUpdateAction : public MovementAction
{
public:
    GatheringLevelingUpdateAction(PlayerbotAI* botAI) : MovementAction(botAI, "gather leveling update") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool IsProfessionMaxed(uint32 skillId);
    bool NeedsRankUp(uint32 skillId);
    uint32 GetGatheringSkill();
    bool HandleTrainer(GatheringSession& session);
    bool FindNearestTrainer(uint32 skillId, ObjectGuid& guid);
    bool UseTrainer(GatheringSession& session, ObjectGuid guid);
    void RoamInZone(GatheringSession& session);
    void Finish(GatheringSession& session);
};

#endif
