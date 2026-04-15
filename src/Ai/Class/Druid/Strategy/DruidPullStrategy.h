/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUID_PULL_STRATEGY_H
#define _PLAYERBOT_DRUID_PULL_STRATEGY_H

#include "PullStrategy.h"

class DruidPullStrategy : public PullStrategy
{
public:
    DruidPullStrategy(PlayerbotAI* botAI) : PullStrategy(botAI, "faerie fire", "dire bear form") {}

    std::string GetPullActionName() const override;
    std::string GetPreActionName() const override;
};

#endif
