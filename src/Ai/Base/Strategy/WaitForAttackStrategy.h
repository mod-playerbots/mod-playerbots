/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WAITFORATTACKSTRATEGY_H
#define _PLAYERBOT_WAITFORATTACKSTRATEGY_H

#include "Multiplier.h"
#include "Strategy.h"

class PlayerbotAI;

class WaitForAttackStrategy : public Strategy
{
public:
    WaitForAttackStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "wait for attack"; }

    static bool ShouldWait(PlayerbotAI* botAI);
    static uint8 GetWaitTime(PlayerbotAI* botAI);
    static float GetSafeDistance();
    static float GetSafeDistanceThreshold() { return 2.5f; }

private:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class WaitForAttackMultiplier : public Multiplier
{
public:
    WaitForAttackMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "wait for attack") {}

    float GetValue(Action* action) override;
};

#endif
