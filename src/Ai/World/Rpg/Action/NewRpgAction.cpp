#include "NewRpgAction.h"

#include <cmath>
#include <cstdlib>

#include "BroadcastHelper.h"
#include "ChatHelper.h"
#include "G3D/Vector2.h"
#include "GossipDef.h"
#include "IVMapMgr.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "QuestDef.h"
#include "Random.h"
#include "SharedDefines.h"
#include "Timer.h"
#include "TravelMgr.h"

bool TellRpgStatusAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;
    std::string out = botAI->rpgInfo.ToString();
    bot->Whisper(out.c_str(), LANG_UNIVERSAL, owner);
    return true;
}

bool StartRpgDoQuestAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(owner);
    uint32 questId = ch.extractQuestId(text);
    const Quest* quest = sObjectMgr->GetQuestTemplate(questId);
    if (quest)
    {
        botAI->rpgInfo.ChangeToDoQuest(questId, quest);
        bot->Whisper("Start to do quest " + std::to_string(questId), LANG_UNIVERSAL, owner);
        return true;
    }
    bot->Whisper("Invalid quest " + text, LANG_UNIVERSAL, owner);
    return false;
}

bool NewRpgStatusUpdateAction::Execute(Event)
{
    NewRpgInfo& info = this->botAI->rpgInfo;
    const NewRpgStatus status = info.GetStatus();

    switch (status)
    {
        case RPG_IDLE:
            return this->RandomChangeStatus(
                {
                    RPG_GO_CAMP,
                    RPG_GO_GRIND,
                    RPG_WANDER_RANDOM,
                    RPG_WANDER_NPC,
                    RPG_DO_QUEST,
                    RPG_TRAVEL_FLIGHT,
                    RPG_REST
                }
            );

        case RPG_GO_GRIND:
        {
            const NewRpgInfo::GoGrind& data = std::get<NewRpgInfo::GoGrind>(info.data);
            const WorldPosition& originalPos = data.pos;
            assert(data.pos != WorldPosition());
            // GO_GRIND -> WANDER_RANDOM
            if (this->bot->GetExactDist(originalPos) < 10.0f)
            {
                info.ChangeToWanderRandom();

                return true;
            }
            break;
        }
        case RPG_GO_CAMP:
        {
            const NewRpgInfo::GoCamp& data = std::get<NewRpgInfo::GoCamp>(info.data);
            const WorldPosition& originalPos = data.pos;
            assert(data.pos != WorldPosition());
            // GO_CAMP -> WANDER_NPC
            if (this->bot->GetExactDist(originalPos) < 10.0f)
            {
                info.ChangeToWanderNpc();

                return true;
            }
            break;
        }
        case RPG_WANDER_RANDOM:
        {
            // WANDER_RANDOM -> IDLE
            if (info.HasStatusPersisted(statusWanderRandomDuration))
            {
                info.ChangeToIdle();

                return true;
            }
            break;
        }
        case RPG_WANDER_NPC:
        {
            if (info.HasStatusPersisted(statusWanderNpcDuration))
            {
                info.ChangeToIdle();

                return true;
            }
            break;
        }
        case RPG_DO_QUEST:
        {
            // DO_QUEST -> IDLE
            if (info.HasStatusPersisted(statusDoQuestDuration))
            {
                info.ChangeToIdle();

                return true;
            }
            break;
        }
        case RPG_TRAVEL_FLIGHT:
        {
            const NewRpgInfo::TravelFlight& data = std::get<NewRpgInfo::TravelFlight>(info.data);

            if (data.inFlight && !this->bot->IsInFlight())
            {
                // flight arrival
                info.ChangeToIdle();

                return true;
            }
            break;
        }
        case RPG_REST:
        {
            // REST -> IDLE
            if (info.HasStatusPersisted(statusRestDuration))
            {
                info.ChangeToIdle();

                return true;
            }
            break;
        }
        default:
            break;
    }

    return false;
}

bool NewRpgGoGrindAction::Execute(Event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;
    if (auto* data = std::get_if<NewRpgInfo::GoGrind>(&botAI->rpgInfo.data))
        return MoveFarTo(data->pos);

    return false;
}

bool NewRpgGoCampAction::Execute(Event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    if (auto* data = std::get_if<NewRpgInfo::GoCamp>(&botAI->rpgInfo.data))
        return MoveFarTo(data->pos);

    return false;
}

bool NewRpgWanderRandomAction::Execute(Event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    return MoveRandomNear();
}

bool NewRpgWanderNpcAction::Execute(Event)
{
    NewRpgInfo& info = this->botAI->rpgInfo;

    NewRpgInfo::WanderNpc* const dataPtr = std::get_if<NewRpgInfo::WanderNpc>(&info.data);

    if (dataPtr == nullptr)
    {
        return false;
    }

    NewRpgInfo::WanderNpc& data = *dataPtr;

    if (data.npcOrGo.IsEmpty())
    {
        // No npc can be found, switch to IDLE
        const ObjectGuid npcOrGo = this->ChooseNpcOrGameObjectToInteract();

        if (npcOrGo.IsEmpty())
        {
            info.ChangeToIdle();

            return true;
        }

        data.npcOrGo = npcOrGo;
        data.lastReach = 0;

        return true;
    }

    WorldObject* const object = ObjectAccessor::GetWorldObject(*bot, data.npcOrGo);

    if (object == nullptr || this->IsWithinInteractionDist(object) == false)
    {
        return this->MoveWorldObjectTo(data.npcOrGo);
    }

    if (!data.lastReach)
    {
        data.lastReach = getMSTime();

        if (this->bot->CanInteractWithQuestGiver(object))
        {
            this->InteractWithNpcOrGameObjectForQuest(data.npcOrGo);
        }

        return true;
    }

    if (data.lastReach && GetMSTimeDiffToNow(data.lastReach) < npcStayTime)
    {
        return false;
    }

    // has reached the npc for more than `npcStayTime`, select the next target
    data.npcOrGo = ObjectGuid();
    data.lastReach = 0;

    return true;
}

bool NewRpgDoQuestAction::Execute(Event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    NewRpgInfo& info = botAI->rpgInfo;
    auto* dataPtr = std::get_if<NewRpgInfo::DoQuest>(&info.data);
    if (!dataPtr)
        return false;
    auto& data = *dataPtr;
    uint32 questId = data.questId;
    uint8 questStatus = bot->GetQuestStatus(questId);
    switch (questStatus)
    {
        case QUEST_STATUS_INCOMPLETE:
            return DoIncompleteQuest(data);
        case QUEST_STATUS_COMPLETE:
            return DoCompletedQuest(data);
        default:
            break;
    }
    info.ChangeToIdle();
    return true;
}

bool NewRpgDoQuestAction::DoIncompleteQuest(NewRpgInfo::DoQuest& data)
{
    const uint32_t questId = data.questId;
    const ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return false;
    }

    if (data.pos != WorldPosition())
    {
        /// @TODO: extract to a new function
        const int32_t currentObjective = data.objectiveIdx;
        // check if the objective has completed
        const Quest* const quest = objectMgr->GetQuestTemplate(questId);
        const QuestStatusData& q_status = bot->getQuestStatusMap().at(questId);

        bool completed = true;

        if (currentObjective < QUEST_OBJECTIVES_COUNT)
        {
            if (q_status.CreatureOrGOCount[currentObjective] < quest->RequiredNpcOrGoCount[currentObjective])
            {
                completed = false;
            }
        }
        else if (currentObjective < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        {
            if (q_status.ItemCount[currentObjective - QUEST_OBJECTIVES_COUNT] <
                quest->RequiredItemCount[currentObjective - QUEST_OBJECTIVES_COUNT]
            )
            {

                completed = false;
            }
        }
        // the current objective is completed, clear and find a new objective later
        if (completed)
        {
            data.lastReachPOI = 0;
            data.pos = WorldPosition();
            data.objectiveIdx = 0;
        }
    }

    if (data.pos == WorldPosition())
    {
        std::vector<POIInfo> poiInfo;

        if (!this->GetQuestPOIPosAndObjectiveIdx(questId, poiInfo))
        {
            // can't find a poi pos to go, stop doing quest for now
            this->botAI->rpgInfo.ChangeToIdle();

            return true;
        }

        const uint32_t rndIdx = urand(0, poiInfo.size() - 1);
        const G3D::Vector2 nearestPoi = poiInfo[rndIdx].pos;
        const int32_t objectiveIdx = poiInfo[rndIdx].objectiveIdx;
        const float dx = nearestPoi.x, dy = nearestPoi.y;

        const Map* const map = this->bot->GetMap();

        if (map == nullptr)
        {
            return false;
        }

        // z = MAX_HEIGHT as we do not know accurate z
        const float dz = std::max(map->GetHeight(dx, dy, MAX_HEIGHT), map->GetWaterLevel(dx, dy));

        // double check for GetQuestPOIPosAndObjectiveIdx
        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
        {
            return false;
        }

        WorldPosition pos{bot->GetMapId(), dx, dy, dz};
        data.lastReachPOI = 0;
        data.pos = pos;
        data.objectiveIdx = objectiveIdx;
    }

    if (this->bot->GetDistance(data.pos) > 10.0f && data.lastReachPOI == 0)
    {
        return this->MoveFarTo(data.pos);
    }
    // Now we are near the quest objective
    // kill mobs and looting quest should be done automatically by grind strategy

    if (data.lastReachPOI == 0)
    {
        data.lastReachPOI = getMSTime();

        return true;
    }
    // stayed at this POI for more than 5 minutes
    if (GetMSTimeDiffToNow(data.lastReachPOI) >= poiStayTime)
    {
        bool hasProgression = false;
        const int32_t currentObjective = data.objectiveIdx;

        // check if the objective has progression
        const Quest* const quest = objectMgr->GetQuestTemplate(questId);
        const QuestStatusData& q_status = this->bot->getQuestStatusMap().at(questId);

        if (currentObjective < QUEST_OBJECTIVES_COUNT)
        {
            if (q_status.CreatureOrGOCount[currentObjective] != 0 && quest->RequiredNpcOrGoCount[currentObjective])
            {
                hasProgression = true;
            }
        }
        else if (currentObjective < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        {
            if (q_status.ItemCount[currentObjective - QUEST_OBJECTIVES_COUNT] != 0 &&
                quest->RequiredItemCount[currentObjective - QUEST_OBJECTIVES_COUNT])
                {
                    hasProgression = true;
                }
        }

        if (!hasProgression)
        {
            // we has reach the poi for more than 5 mins but no progession
            // may not be able to complete this quest, marked as abandoned
            /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
            this->botAI->lowPriorityQuest.insert(questId);
            this->botAI->rpgStatistic.questAbandoned++;
            LOG_DEBUG("playerbots", "[New RPG] {} marked as abandoned quest {}", bot->GetName(), questId);
            this->botAI->rpgInfo.ChangeToIdle();

            return true;
        }
        // clear and select another poi later
        data.lastReachPOI = 0;
        data.pos = WorldPosition();
        data.objectiveIdx = 0;

        return true;
    }

    return this->MoveRandomNear(20.0f);
}

bool NewRpgDoQuestAction::DoCompletedQuest(NewRpgInfo::DoQuest& data)
{
    const uint32_t questId = data.questId;
    const Quest* const quest = data.quest;

    if (data.objectiveIdx != -1)
    {
        // if quest is completed, back to poi with -1 idx to reward
        BroadcastHelper::BroadcastQuestUpdateComplete(this->botAI, this->bot, quest);
        this->botAI->rpgStatistic.questCompleted++;

        std::vector<POIInfo> poiInfo{};

        if (!this->GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
        {
            // can't find a poi pos to reward, stop doing quest for now
            this->botAI->rpgInfo.ChangeToIdle();

            return false;
        }

        assert(poiInfo.size() > 0);

        const Map* const map = this->bot->GetMap();

        if (map == nullptr)
        {
            return false;
        }

        // now we get the place to get rewarded
        const float dx = poiInfo[0].pos.x, dy = poiInfo[0].pos.y;
        // z = MAX_HEIGHT as we do not know accurate z
        const float dz = std::max(map->GetHeight(dx, dy, MAX_HEIGHT), map->GetWaterLevel(dx, dy));

        // double check for GetQuestPOIPosAndObjectiveIdx
        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
        {
            return false;
        }

        WorldPosition pos{this->bot->GetMapId(), dx, dy, dz};
        data.lastReachPOI = 0;
        data.pos = pos;
        data.objectiveIdx = -1;
    }

    if (data.pos == WorldPosition{})
    {
        return false;
    }

    if (this->bot->GetDistance(data.pos) > 10.0f && data.lastReachPOI == 0)
    {
        return this->MoveFarTo(data.pos);
    }

    // Now we are near the qoi of reward
    // the quest should be rewarded by SearchQuestGiverAndAcceptOrReward
    if (data.lastReachPOI == 0)
    {
        data.lastReachPOI = getMSTime();

        return true;
    }

    // stayed at this POI for more than 5 minutes
    if (GetMSTimeDiffToNow(data.lastReachPOI) >= poiStayTime)
    {
        // e.g. Can not reward quest to gameobjects
        /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
        this->botAI->lowPriorityQuest.insert(questId);
        this->botAI->rpgStatistic.questAbandoned++;
        LOG_DEBUG("playerbots", "[New RPG] {} marked as abandoned quest {}", this->bot->GetName(), questId);
        this->botAI->rpgInfo.ChangeToIdle();

        return true;
    }

    return false;
}

bool NewRpgTravelFlightAction::Execute(Event)
{
    NewRpgInfo& info = botAI->rpgInfo;

    NewRpgInfo::TravelFlight* const dataPtr = std::get_if<NewRpgInfo::TravelFlight>(&info.data);

    if (dataPtr == nullptr)
    {
        return false;
    }

    NewRpgInfo::TravelFlight& data = *dataPtr;

    if (this->bot->IsInFlight())
    {
        data.inFlight = true;

        return false;
    }

    Creature* const flightMaster = ObjectAccessor::GetCreature(*bot, data.fromFlightMaster);

    if (flightMaster == nullptr || !flightMaster->IsAlive())
    {
        botAI->rpgInfo.ChangeToIdle();

        return true;
    }

    if (bot->GetDistance(flightMaster) > INTERACTION_DISTANCE)
    {
        return this->MoveFarTo(flightMaster);
    }

    std::vector<uint32> nodes = {data.fromNode, data.toNode};

    this->botAI->RemoveShapeshift();

    if (this->bot->IsMounted())
    {
        bot->Dismount();
    }

    if (!this->bot->ActivateTaxiPathTo(nodes, flightMaster, 0))
    {
        LOG_DEBUG(
            "playerbots",
            "[New RPG] {} active taxi path {} (from {} to {}) failed",
            this->bot->GetName(),
            flightMaster->GetEntry(),
            nodes[0],
            nodes[1]
        );
        this->botAI->rpgInfo.ChangeToIdle();
    }

    return true;
}