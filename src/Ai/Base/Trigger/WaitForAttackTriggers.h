/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WAITFORATTACKTRIGGERS_H
#define _PLAYERBOT_WAITFORATTACKTRIGGERS_H

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
