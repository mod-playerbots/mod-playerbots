/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HUNTERBUFFSTRATEGIES_H
#define PLAYERBOTS_HUNTERBUFFSTRATEGIES_H

#include "Strategy.h"

class PlayerbotAI;

class HunterBuffSpeedStrategy : public Strategy
{
public:
    HunterBuffSpeedStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bspeed"; }
};

class HunterBuffDpsStrategy : public Strategy
{
public:
    HunterBuffDpsStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bdps"; }
};

class HunterNatureResistanceStrategy : public Strategy
{
public:
    HunterNatureResistanceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "rnature"; }
};

#endif
