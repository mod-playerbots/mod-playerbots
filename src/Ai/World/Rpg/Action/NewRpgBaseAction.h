/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGBASEACTION_H
#define PLAYERBOTS_NEWRPGBASEACTION_H

#include "LastMovementValue.h"
#include "MovementActions.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Object.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "QuestDef.h"
#include "TravelMgr.h"

struct POIInfo
{
    G3D::Vector2 pos;
    int32 objectiveIdx;
    uint32 poiId{0};        // quest_poi.Id, the key for the baked Z store
    bool hasBakedZ{false};  // pos comes from the baked store and z is the baked floor
    float z{0.0f};          // walkable floor, baked or terrain-estimated
};

// quest_poi.ObjectiveIndex used by scout/explore objectives. It is outside the four NPC/GO and
// six item slots, so the normal incomplete-objective loop never matches it.
constexpr int32 QUEST_OBJECTIVE_EXPLORE = 16;

// A resolved 3D objective/turn-in position from spawn data (quest guidp map).
struct QuestObjectivePosition
{
    int32 objectiveIdx;  // 0..3 NpcOrGo kill/GO objective slot, -1 turn-in
    WorldPosition pos;
    ObjectGuid::LowType spawnId{0};  // core spawn id, used to find the loaded object
    bool gameObject{false};
};

/// A base (composition) class for all new rpg actions
/// All functions that may be shared by multiple actions should be declared here
/// And we should make all actions composable instead of inheritable
class NewRpgBaseAction : public MovementAction
{
public:
    NewRpgBaseAction(PlayerbotAI* botAI, std::string name) : MovementAction(botAI, name) {}

    // Builds the spawn index once on the world thread at startup, so the first questing bot does
    // not pay a full spawn scan on a map thread.
    static void WarmUpQuestObjectiveSpawnIndex();

protected:
    /* MOVEMENT RELATED */
    bool MoveFarTo(WorldPosition dest);
    bool MoveWorldObjectTo(ObjectGuid guid, float distance = INTERACTION_DISTANCE);
    bool MoveRandomNear(float moveStep = 50.0f, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL, WorldObject* center = nullptr);
    bool ForceToWait(uint32 duration, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);

    /* QUEST RELATED CHECK */
    bool InteractWithNpcOrGameObjectForQuest(ObjectGuid guid);
    bool IsWithinInteractionDist(Object* object);
    uint32 BestRewardIndex(Quest const* quest);

    /* QUEST RELATED ACTION */
    bool SearchQuestGiverAndAcceptOrReward();
    bool AcceptQuest(Quest const* quest, ObjectGuid guid);
    bool TurnInQuest(Quest const* quest, ObjectGuid guid);
    bool OrganizeQuestLog();

    // Correct 3D positions for a quest's incomplete objectives (toComplete = turn-in) from the
    // spawn/quest-ender index: covers cave floors and tower platforms alike. Empty when the map
    // has no entry - callers fall back to POI + baked/terrain height.
    static bool GetQuestObjectivePositions(Player* bot, uint32 questId,
                                           std::vector<QuestObjectivePosition>& outPositions, bool toComplete);
    // Loaded creature/gameobject for a spawn entry, or nullptr when the spawn is not loaded.
    static WorldObject* FindLoadedSpawn(Player* bot, QuestObjectivePosition const& candidate);

protected:
    bool GetQuestPOIPosAndObjectiveIdx(uint32 questId, std::vector<POIInfo>& poiInfo, bool toComplete = false);
    static WorldPosition SelectRandomGrindPos(Player* bot);
    static WorldPosition SelectRandomCampPos(Player* bot);
    bool SelectRandomFlightTaxiNode(uint32& flightMasterEntry, WorldPosition& flightMasterPos, std::vector<uint32>& path);
    bool RandomChangeStatus(std::vector<NewRpgStatus> candidateStatus);
    bool CheckRpgStatusAvailable(NewRpgStatus status);

protected:
    /* FOR MOVE FAR */
    const float pathFinderDis = 70.0f;
    // Time without real progress toward dest before MoveFarTo
    // falls back to teleport recovery. Kept short enough that a
    // bot truly oscillating around an unreachable destination
    // (mmap returning non-progressing partial paths, or NOPATH +
    // cone fallback wandering) doesn't spin for 5 minutes before
    // the teleport fires, but long enough that a genuine long
    // walk that is slowly making progress never triggers it.
    const uint32 stuckTime = 90 * IN_MILLISECONDS;

    /* FOR QUEST GIVER */
    // Time on one giver, walking and interacting, after which it is skipped until the next IDLE.
    const uint32 questGiverGiveUpTime = 60 * IN_MILLISECONDS;
    // A longer pause (combat, eating) starts the timer over.
    const uint32 questGiverRetryGap = 5 * IN_MILLISECONDS;
};

#endif
