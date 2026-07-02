/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STAYSTRATEGY_H
#define PLAYERBOTS_STAYSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class StayStrategy : public Strategy
{
public:
    StayStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "stay"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::vector<NextAction> getDefaultActions() override;
};

class SitStrategy : public NonCombatStrategy
{
public:
    SitStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "sit"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
