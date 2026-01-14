#include "RaidNaxxActions.h"

#include "Playerbots.h"

bool GrobbulusGoBehindAction::Execute(Event event)
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
    {
        return false;
    }
    // Position* pos = boss->GetPosition();
    float orientation = boss->GetOrientation() + M_PI + delta_angle;
    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    float rx = x + cos(orientation) * distance;
    float ry = y + sin(orientation) * distance;
    return MoveTo(bot->GetMapId(), rx, ry, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool GrobbulusMoveAwayAction::Execute(Event event)
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
    {
        return false;
    }

    const float currentDistance = bot->GetExactDist2d(boss);
    if (currentDistance >= distance)
    {
        return false;
    }

    const float angle = boss->GetAngle(bot);
    const float x = boss->GetPositionX() + cos(angle) * distance;
    const float y = boss->GetPositionY() + sin(angle) * distance;
    const float z = bot->GetPositionZ();

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

uint32 GrobbulusRotateAction::GetCurrWaypoint()
{
    uint32 current = FindNearestWaypoint();
    if (clockwise)
    {
        return (current + 1) % intervals;
    }
    return (current + intervals - 1) % intervals;
}