/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ASSASSINATIONROGUESTRATEGY_H
#define PLAYERBOTS_ASSASSINATIONROGUESTRATEGY_H

#include "MeleeCombatStrategy.h"

class AssassinationRogueStrategy : public MeleeCombatStrategy
{
public:
    AssassinationRogueStrategy(PlayerbotAI* ai);

public:
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual std::string const getName() override { return "melee"; }
    virtual std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return MeleeCombatStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

#endif
