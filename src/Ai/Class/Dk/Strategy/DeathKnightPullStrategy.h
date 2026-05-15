/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DEATH_KNIGHT_PULL_STRATEGY_H
#define _PLAYERBOT_DEATH_KNIGHT_PULL_STRATEGY_H

#include "PullStrategy.h"

class DeathKnightPullStrategy : public PullStrategy
{
public:
    DeathKnightPullStrategy(PlayerbotAI* botAI) : PullStrategy(botAI, "icy touch") {}

    std::string GetPullActionName() const override;
};

#endif
