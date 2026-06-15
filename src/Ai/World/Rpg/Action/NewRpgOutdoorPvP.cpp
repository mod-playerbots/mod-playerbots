#include "NewRpgOutdoorPvP.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"

bool NewRpgOutdoorPvpAction::Execute(Event event)
{
    if (!bot->IsPvP())
    {
        botAI->rpgInfo.ChangeToIdle();
        return false;
    }
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL) || !bot->IsOutdoorPvPActive())
        return false;

    uint32 zoneId = bot->GetZoneId();
    OutdoorPvP* outdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(zoneId);
    if (!outdoorPvP || zoneId == AREA_NAGRAND)
    {
        botAI->rpgInfo.ChangeToIdle();
        return false;
    }

    OutdoorPvP::OPvPCapturePointMap const& capturePointMap = outdoorPvP->GetCapturePoints();

    NewRpgInfo& info = botAI->rpgInfo;
    auto* dataPtr = std::get_if<NewRpgInfo::OutdoorPvP>(&info.data);
    if (!dataPtr)
        return false;
    auto& data = *dataPtr;
    // Re-resolve stored spawn ID from the capture point map each tick (avoids dangling pointers)
    OPvPCapturePoint* objective = nullptr;
    if (data.capturePointSpawnId && !capturePointMap.empty())
    {
        auto it = capturePointMap.find(data.capturePointSpawnId);
        if (it != capturePointMap.end())
        {
            OPvPCapturePoint* capturePoint = it->second;
            if (capturePoint && capturePoint->_capturePoint)
            {
                float threshold = capturePoint->GetMinValue();
                float slider = capturePoint->GetSlider();
                uint8 faction = bot->GetTeamId();
                LOG_DEBUG("playerbots", "[NEW RPG] Bot {} with faction {} is evaluating existing RPG objective {} with threshold {} and slider value {}", bot->GetName(), faction, capturePoint->_capturePoint->GetName(), threshold, slider);
                if ((faction == TEAM_HORDE && slider >= -threshold) ||
                    (faction == TEAM_ALLIANCE && slider <= threshold))
                    objective = capturePoint;
            }
        }
        if (!objective)
            data.capturePointSpawnId = 0;
    }

    if (!objective)
    {
        objective = SelectNewObjective(capturePointMap);
        if (!objective)
        {
            botAI->rpgInfo.ChangeToIdle();
            return true;
        }
        data.capturePointSpawnId = objective->m_capturePointSpawnId;
        LOG_DEBUG("playerbots","[NEW RPG] Bot {} selected OutDoorPvP target capturePointSpawnId {}", bot->GetName(), data.capturePointSpawnId);
    }

    GameObject* objectiveGO = objective->_capturePoint;
    if (!objectiveGO)
        return false;

    if (objectiveGO->GetGoType() != GAMEOBJECT_TYPE_CAPTURE_POINT)
        return false;

    float radius = objectiveGO->GetGOInfo()->capturePoint.radius / 2.0f;
    if (!objectiveGO->IsWithinDistInMap(bot, radius))
        return MoveFarTo(WorldPosition(objectiveGO));

    return PatrolCapturePoint(objectiveGO, radius);
}

OPvPCapturePoint* NewRpgOutdoorPvpAction::SelectNewObjective(OutdoorPvP::OPvPCapturePointMap const& capturePointMap)
{
    OPvPCapturePoint* objective = nullptr;
    uint8 faction = bot->GetTeamId();
    std::vector<OPvPCapturePoint*> candidateObjectives;

    if (capturePointMap.empty())
    {
        botAI->rpgInfo.ChangeToIdle();
        return objective;
    }
    for (auto const& [guid, point] : capturePointMap)
    {
        GameObject* capturePointObject = point->_capturePoint;
        if (!capturePointObject)
            continue;

        float threshold = point->GetMinValue();
        float slider = point->GetSlider();
        if (faction == TEAM_HORDE && slider > -threshold)
            candidateObjectives.push_back(point);
        else if (faction == TEAM_ALLIANCE && slider < threshold)
            candidateObjectives.push_back(point);
    }
    if (candidateObjectives.empty())
    {
        LOG_DEBUG("playerbots", "[New RPG] Bot {} found no valid outdoor PVP objectives to capture", bot->GetName());
        botAI->rpgInfo.ChangeToIdle();
        return objective;
    }
    int randomIndex = urand(0, candidateObjectives.size() - 1);
    objective = candidateObjectives[randomIndex];
    return objective;
}

bool NewRpgOutdoorPvpAction::PatrolCapturePoint(GameObject* objectiveGO, float radius)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return false;

    // Randomly pause at the current spot before picking a new patrol point
    if (urand(0, 2) == 0)
        return ForceToWait(urand(3000, 6000));

    float patrolRadius = radius * 0.8f;
    if (MoveRandomNear(patrolRadius, MovementPriority::MOVEMENT_NORMAL, objectiveGO))
        return true;

    return ForceToWait(urand(3000, 6000));
}
