#include "NewRpgOutdoorPvP.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"

bool NewRpgOutdoorPvpAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;

    GetCapturePoints();
    OPvPCapturePoint* objective = nullptr;
    if (!this->outdoorPvP)
    {
        // Not in an outdoor PvP zone, go back to idle
        botAI->rpgInfo.ChangeToIdle();
        return false;
    }
    auto& data = std::get<NewRpgInfo::OutdoorPvP>(info.data);

    OPvPCapturePoint* capturePoint = data.capturePoint;
    if (capturePoint)
    {
        if (!capturePoint->_capturePoint)
            data.capturePoint = nullptr;

        else
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
    {
        objective = SelectNewObjective();
        if (!objective)
        {
            botAI->rpgInfo.ChangeToIdle();
            return true; // No valid objectives, possibly all captured
        }
        data.capturePoint = objective;
    }
    GameObject* objectiveGO = objective->_capturePoint;
    if (!objectiveGO)
        return false;

    if (objectiveGO->GetGoType() != GAMEOBJECT_TYPE_CAPTURE_POINT)
        return false;

    float radius = objectiveGO->GetGOInfo()->capturePoint.radius / 2.0f;
    if (!objectiveGO->IsWithinDistInMap(bot, radius) || !bot->IsOutdoorPvPActive())
        return MoveFarTo(WorldPosition(objectiveGO));

    // Within capture range - patrol the area while capturing
    return PatrolCapturePoint(objectiveGO, radius);
}

OPvPCapturePoint* NewRpgOutdoorPvpAction::SelectNewObjective()
{
    OPvPCapturePoint* objective = nullptr;
    uint8 faction = bot->GetTeamId();
    std::vector<OPvPCapturePoint*> candidateObjectives;
    if (!this->outdoorPvP)
        GetCapturePoints();

    if (!this->capturePointMap)
    {
        botAI->rpgInfo.ChangeToIdle();
        return objective;
    }
    for (auto const& [guid, point] : *capturePointMap)
    {
        GameObject* capturePointObject = point->_capturePoint;
        if (!capturePointObject)
            continue;

        float threshold = point->GetMinValue();
        float slider = point->GetSlider();
        if (faction == TEAM_HORDE)
        {
            if (slider > -threshold)
            candidateObjectives.push_back(point);
        }
        else
        {
            if (slider < threshold)
                candidateObjectives.push_back(point);
        }
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

void NewRpgOutdoorPvpAction::GetCapturePoints()
{
    outdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(bot->GetZoneId());
    if (!outdoorPvP)
        return;
    capturePointMap = outdoorPvP->GetCapturePoints();
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
