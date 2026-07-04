/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SWPSTRATEGY_H
#define PLAYERBOTS_SWPSTRATEGY_H

#include "Strategy.h"

class RaidSunwellStrategy : public Strategy
{
public:
    RaidSunwellStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    bool HasTargetExclusions() const override { return true; }
    std::string const getName() override { return "sunwell"; }

    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
