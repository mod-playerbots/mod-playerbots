/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRAVELSTRATEGY_H
#define PLAYERBOTS_TRAVELSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class TravelStrategy : public Strategy
{
public:
    TravelStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "travel"; }

    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class ExploreStrategy : public Strategy
{
public:
    ExploreStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "explore"; }
};

class MapStrategy : public Strategy
{
public:
    MapStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map"; }
};

class MapFullStrategy : public Strategy
{
public:
    MapFullStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map full"; }
};

#endif
