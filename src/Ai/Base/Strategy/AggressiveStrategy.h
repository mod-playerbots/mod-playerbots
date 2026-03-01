/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "NonCombatStrategy.h"

class PlayerbotAI;

class AggressiveStrategy : public NonCombatStrategy
{
public:
    AggressiveStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    const std::string getName() override
    {
        return "aggressive";
    }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};
