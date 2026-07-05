/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PullTriggers.h"

#include "PositionValue.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PullStrategy.h"

bool PullStartTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);
    return strategy && strategy->IsPullPendingToStart();
}

bool PullEndTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    if (!strategy || !strategy->HasPullStarted())
        return false;

    Unit* target = strategy->GetTarget();
    if (!target || !target->IsInWorld() || !target->IsAlive())
        return true;

    time_t const secondsSincePullStarted = time(nullptr) - strategy->GetPullStartTime();
    if (secondsSincePullStarted >= PullStrategy::GetMaxPullTime())
        return true;

    float distanceToPullTarget = bot->GetDistance(target);
    if (distanceToPullTarget > ATTACK_DISTANCE && !target->IsNonMeleeSpellCast(false, false, true) &&
        (!botAI->IsRanged(bot) || distanceToPullTarget > botAI->GetRange("spell")))
        return false;

    if (!botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return true;

    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    if (!pullPosition.isSet() || pullPosition.mapId != bot->GetMapId())
        return true;

    return bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) <= botAI->GetRange("follow");
}

bool ReturnToPullPositionTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    if (!strategy || !strategy->HasPullStarted() || !target || !target->IsInCombat() ||
        !botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return false;

    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    return pullPosition.isSet() && pullPosition.mapId == bot->GetMapId() &&
           bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) > sPlayerbotAIConfig.followDistance;
}
