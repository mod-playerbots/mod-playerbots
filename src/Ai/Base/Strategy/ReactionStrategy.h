/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_REACTIONSTRATEGY_H
#define _PLAYERBOT_REACTIONSTRATEGY_H

#include "Strategy.h"

class ReactionStrategy : public Strategy
{
public:
    ReactionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "react"; }
    uint32 GetType() const override { return STRATEGY_TYPE_REACTION; }

protected:
    void InitReactionTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
