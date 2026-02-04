#include "NewRpgInfo.h"

#include <cmath>

#include "Timer.h"

void NewRpgInfo::ChangeToGoGrind(WorldPosition pos)
{
    startT = getMSTime();
    data = GoGrind{pos};
}

void NewRpgInfo::ChangeToGoCamp(WorldPosition pos)
{
    startT = getMSTime();
    data = GoCamp{pos};
}

void NewRpgInfo::ChangeToWanderNpc()
{
    startT = getMSTime();
    data = WanderNpc{};
}

void NewRpgInfo::ChangeToWanderRandom()
{
    startT = getMSTime();
    data = WanderRandom{};
}

void NewRpgInfo::ChangeToDoQuest(uint32 questId, const Quest* quest)
{
    startT = getMSTime();
    DoQuest do_quest;
    do_quest.questId = questId;
    do_quest.quest = quest;
    data = do_quest;
}

void NewRpgInfo::ChangeToTravelFlight(ObjectGuid fromFlightMaster, uint32 fromNode, uint32 toNode)
{
    startT = getMSTime();
    TravelFlight flight;
    flight.fromFlightMaster = fromFlightMaster;
    flight.fromNode = fromNode;
    flight.toNode = toNode;
    flight.inFlight = false;
    data = flight;
}

void NewRpgInfo::ChangeToRest()
{
    startT = getMSTime();
    data = Rest{};
}

void NewRpgInfo::ChangeToIdle()
{
    startT = getMSTime();
    data = Idle{};
}

bool NewRpgInfo::CanChangeTo(NewRpgStatus status) { return true; }

void NewRpgInfo::Reset()
{
    data = Idle{};
    startT = getMSTime();
}

void NewRpgInfo::SetMoveFarTo(WorldPosition pos)
{
    nearestMoveFarDis = FLT_MAX;
    stuckTs = 0;
    stuckAttempts = 0;
    moveFarPos = pos;
}

NewRpgStatus NewRpgInfo::GetStatus()
{
    return std::visit([](auto&& arg) -> NewRpgStatus {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Idle>) return RPG_IDLE;
        if constexpr (std::is_same_v<T, GoGrind>) return RPG_GO_GRIND;
        if constexpr (std::is_same_v<T, GoCamp>) return RPG_GO_CAMP;
        if constexpr (std::is_same_v<T, WanderNpc>) return RPG_WANDER_NPC;
        if constexpr (std::is_same_v<T, WanderRandom>) return RPG_WANDER_RANDOM;
        if constexpr (std::is_same_v<T, Rest>) return RPG_REST;
        if constexpr (std::is_same_v<T, DoQuest>) return RPG_DO_QUEST;
        if constexpr (std::is_same_v<T, TravelFlight>) return RPG_TRAVEL_FLIGHT;
        return RPG_IDLE;
    }, data);
}

std::string NewRpgInfo::ToString()
{
    std::stringstream out;
    out << "Status: ";
    std::visit([&out, this](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, GoGrind>)
        {
            out << "GO_GRIND";
            out << "\nGrindPos: " << arg.pos.GetMapId() << " " << arg.pos.GetPositionX() << " "
                << arg.pos.GetPositionY() << " " << arg.pos.GetPositionZ();
            out << "\nlastGoGrind: " << startT;
        }
        else if constexpr (std::is_same_v<T, GoCamp>)
        {
            out << "GO_CAMP";
            out << "\nCampPos: " << arg.pos.GetMapId() << " " << arg.pos.GetPositionX() << " "
                << arg.pos.GetPositionY() << " " << arg.pos.GetPositionZ();
            out << "\nlastGoCamp: " << startT;
        }
        else if constexpr (std::is_same_v<T, WanderNpc>)
        {
            out << "WANDER_NPC";
            out << "\nnpcOrGoEntry: " << arg.npcOrGo.GetCounter();
            out << "\nlastWanderNpc: " << startT;
            out << "\nlastReachNpcOrGo: " << arg.lastReach;
        }
        else if constexpr (std::is_same_v<T, WanderRandom>)
        {
            out << "WANDER_RANDOM";
            out << "\nlastWanderRandom: " << startT;
        }
        else if constexpr (std::is_same_v<T, Idle>)
        {
            out << "IDLE";
        }
        else if constexpr (std::is_same_v<T, Rest>)
        {
            out << "REST";
            out << "\nlastRest: " << startT;
        }
        else if constexpr (std::is_same_v<T, DoQuest>)
        {
            out << "DO_QUEST";
            out << "\nquestId: " << arg.questId;
            out << "\nobjectiveIdx: " << arg.objectiveIdx;
            out << "\npoiPos: " << arg.pos.GetMapId() << " " << arg.pos.GetPositionX() << " "
                << arg.pos.GetPositionY() << " " << arg.pos.GetPositionZ();
            out << "\nlastReachPOI: " << (arg.lastReachPOI ? GetMSTimeDiffToNow(arg.lastReachPOI) : 0);
        }
        else if constexpr (std::is_same_v<T, TravelFlight>)
        {
            out << "TRAVEL_FLIGHT";
            out << "\nfromFlightMaster: " << arg.fromFlightMaster.GetEntry();
            out << "\nfromNode: " << arg.fromNode;
            out << "\ntoNode: " << arg.toNode;
            out << "\ninFlight: " << arg.inFlight;
        }
        else
            out << "UNKNOWN";
    }, data);
    return out.str();
}
