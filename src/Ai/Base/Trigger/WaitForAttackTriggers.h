/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAITFORATTACKTRIGGERS_H
#define PLAYERBOTS_WAITFORATTACKTRIGGERS_H

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Trigger.h"
#include "WaitForAttackStrategy.h"

class PlayerbotAI;

class WaitForAttackSafeDistanceTrigger : public Trigger
{
public:
    WaitForAttackSafeDistanceTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "wait for attack safe distance") {}

    bool IsActive() override
    {
        if (!WaitForAttackStrategy::ShouldWait(botAI))
            return false;

        // Do not move if stay strategy is set
        if (botAI->HasStrategy("stay", botAI->GetState()))
            return false;

        // Do not move if currently being targeted
        if (!bot->getAttackers().empty())
            return false;

        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target)
            return false;

        float safeDistance = WaitForAttackStrategy::GetSafeDistance();
        float safeDistanceThreshold = WaitForAttackStrategy::GetSafeDistanceThreshold();
        float distanceToTarget = ServerFacade::instance().GetDistance2d(bot, target);

        return (distanceToTarget > (safeDistance + safeDistanceThreshold)) ||
               (distanceToTarget < (safeDistance - safeDistanceThreshold));
    }
};

#endif
