/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_LFGSTRATEGY_H
#define PLAYERBOTS_LFGSTRATEGY_H

#include "PassThroughStrategy.h"

class LfgStrategy : public PassThroughStrategy
{
public:
    LfgStrategy(PlayerbotAI* botAI);

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "lfg"; }
};

#endif
