/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DEMONOLOGYWARLOCKSTRATEGY_H
#define PLAYERBOTS_DEMONOLOGYWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "CombatStrategy.h"

class PlayerbotAI;

class DemonologyWarlockStrategy : public GenericWarlockStrategy
{
public:
    DemonologyWarlockStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "demo"; }
    std::vector<NextAction> getDefaultActions() override;
};

class MetaMeleeAoeStrategy : public CombatStrategy
{
public:
    MetaMeleeAoeStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "meta melee"; }
};
#endif
