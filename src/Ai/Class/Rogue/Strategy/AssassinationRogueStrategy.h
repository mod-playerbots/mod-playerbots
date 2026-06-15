/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ASSASSINATIONROGUESTRATEGY_H
#define _PLAYERBOT_ASSASSINATIONROGUESTRATEGY_H

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
