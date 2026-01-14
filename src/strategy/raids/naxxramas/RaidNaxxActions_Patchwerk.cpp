#include "RaidNaxxActions.h"

#include <algorithm>
#include <cmath>

bool PatchwerkRangedPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
        return false;

    constexpr float minDistance = 12.0f;
    constexpr float maxDistance = 15.0f;
    const float distance = bot->GetExactDist2d(boss);

    if (distance >= minDistance && distance <= maxDistance)
        return false;

    const float desiredDistance = std::clamp(distance, minDistance, maxDistance);
    float angle = boss->GetAngle(bot);

    if (distance < 0.1f)
        angle = boss->GetOrientation();

    const float x = boss->GetPositionX() + std::cos(angle) * desiredDistance;
    const float y = boss->GetPositionY() + std::sin(angle) * desiredDistance;
    const float z = bot->GetPositionZ();

    return MoveTo(boss->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
                  false);
}