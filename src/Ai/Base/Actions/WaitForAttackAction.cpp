/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WaitForAttackAction.h"

#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TravelMgr.h"
#include "WaitForAttackStrategy.h"

namespace
{

WorldPosition GetBestPoint(AiObjectContext* context, Player* bot, Unit* target,
                           float minDistance, float maxDistance)
{
    WorldPosition botPosition(bot);
    WorldPosition targetPosition(target);

    int8 startDir = urand(0, 1) * 2 - 1;
    float const radiansIncrement = (5.0f / 180.0f) * static_cast<float>(M_PI);
    float startAngle = targetPosition.getAngleTo(botPosition) +
                       frand(0.0f, radiansIncrement) * startDir;
    float distance = frand(minDistance, maxDistance);

    std::list<ObjectGuid> enemies = AI_VALUE(std::list<ObjectGuid>, "possible targets no los");

    for (float tryAngle = 0.0f; tryAngle < static_cast<float>(M_PI); tryAngle += radiansIncrement)
    {
        for (int8 tryDir = -1; tryAngle && tryDir < 1; tryDir += 2)
        {
            float pointAngle = startAngle + tryAngle * startDir * tryDir;

            float x = targetPosition.GetPositionX() + distance * cos(pointAngle);
            float y = targetPosition.GetPositionY() + distance * sin(pointAngle);
            float z = targetPosition.GetPositionZ() + 1.0f;

            WorldPosition point(targetPosition.GetMapId(), x, y, z);
            point.setZ(point.getHeight());

            // Check line of sight to target
            if (!target->IsWithinLOS(point.GetPositionX(), point.GetPositionY(),
                                     point.GetPositionZ() + bot->GetCollisionHeight()))
                continue;

            // Check if enemies are close to this point
            bool enemyClose = false;
            for (ObjectGuid const& enemyGUID : enemies)
            {
                Unit* enemy = ObjectAccessor::GetUnit(*bot, enemyGUID);
                if (enemy && enemy->IsWithinLOSInMap(bot) && enemy->IsHostileTo(bot))
                {
                    float enemyAttackRange = enemy->GetCombatReach() + ATTACK_DISTANCE;
                    WorldPosition enemyPos(enemy);
                    if (enemyPos.sqDistance(point) <= (enemyAttackRange * enemyAttackRange))
                    {
                        enemyClose = true;
                        break;
                    }
                }
            }

            if (enemyClose)
                continue;

            // Check if bot can path to this point
            if (!botPosition.canPathTo(point, bot))
                continue;

            return point;
        }
    }

    return botPosition;
}

}  // namespace

bool WaitForAttackKeepSafeDistanceAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");

    // If our target is moving towards a stationary unit, use that unit as anchor
    if (target && !target->IsStopped())
    {
        ObjectGuid targetGuid = target->GetTarget();
        if (targetGuid)
        {
            Unit* targetsTarget = ObjectAccessor::GetUnit(*target, targetGuid);
            if (targetsTarget && targetsTarget->IsStopped())
                target = targetsTarget;
        }
    }

    if (target && target->IsAlive())
    {
        float safeDistance = std::max(
            target->GetCombatReach() + ATTACK_DISTANCE,
            WaitForAttackStrategy::GetSafeDistance());
        float safeDistanceThreshold = WaitForAttackStrategy::GetSafeDistanceThreshold();

        WorldPosition bestPoint = GetBestPoint(context, bot, target,
            safeDistance - safeDistanceThreshold, safeDistance);

        if (bestPoint)
            return MoveTo(bestPoint.GetMapId(), bestPoint.GetPositionX(),
                          bestPoint.GetPositionY(), bestPoint.GetPositionZ());
    }

    return false;
}

bool SetWaitForAttackTimeAction::Execute(Event event)
{
    std::string newTimeStr = event.getParam();

    if (newTimeStr.empty())
    {
        botAI->TellMaster("Please provide a time to set (in seconds)");
        return false;
    }

    if (newTimeStr.find_first_not_of("0123456789") != std::string::npos)
    {
        botAI->TellMaster("Please provide valid time to set (in seconds) between 0 and 99");
        return false;
    }

    int newTime = std::stoi(newTimeStr);
    if (newTime < 0 || newTime > 99)
    {
        botAI->TellMaster("Please provide valid time to set (in seconds) between 0 and 99");
        return false;
    }

    context->GetValue<uint8>("wait for attack time")->Set(static_cast<uint8>(newTime));

    std::ostringstream out;
    out << "Wait for attack time set to " << newTime << " seconds";
    botAI->TellMaster(out.str());
    return true;
}
