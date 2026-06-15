/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VOASTRATEGY_H
#define _PLAYERBOT_VOASTRATEGY_H

#include "Strategy.h"

class RaidVoAStrategy : public Strategy
{
public:
    RaidVoAStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "voa"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
