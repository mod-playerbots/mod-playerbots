/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIBase.h"
#include "PerfMonitor.h"

PlayerbotAIBase::PlayerbotAIBase(bool isBotAI) : nextAICheckDelay(0), _isBotAI(isBotAI) {}

void PlayerbotAIBase::UpdateAI(uint32 elapsed, bool minimal)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerfMonitor.start(PERF_MON_TOTAL, "PlayerbotAIBase::FullTick");

    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    if (!CanUpdateAI())
        return;

    this->UpdateAIInternal(minimal);
    YieldThread();
}

void PlayerbotAIBase::SetNextCheckDelay(uint32 const delay)
{
    nextAICheckDelay = delay;
}

void PlayerbotAIBase::IncreaseNextCheckDelay(uint32 delay)
{
    nextAICheckDelay += delay;
}

bool PlayerbotAIBase::CanUpdateAI()
{
    return this->nextAICheckDelay == 0;
}

// @TODO: This is extremely poorly named. This is NOT yielding the thread,
// but rather setting the next check delay to the specified value if it is greater than the current value.
void PlayerbotAIBase::YieldThread(const uint32_t delay)
{
    if (this->nextAICheckDelay < delay)
    {
        this->nextAICheckDelay = delay;
    }
}

bool PlayerbotAIBase::IsActive() { return nextAICheckDelay < sPlayerbotAIConfig.maxWaitForMove; }

bool PlayerbotAIBase::IsBotAI() const { return _isBotAI; }
