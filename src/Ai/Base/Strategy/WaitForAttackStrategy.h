/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAITFORATTACKSTRATEGY_H
#define PLAYERBOTS_WAITFORATTACKSTRATEGY_H

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
