/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PASSTHROUGHSTRATEGY_H
#define PLAYERBOTS_PASSTHROUGHSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class PassThroughStrategy : public Strategy
{
public:
    PassThroughStrategy(PlayerbotAI* botAI, float relevance = 100.0f) : Strategy(botAI), relevance(relevance) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;

protected:
    std::vector<std::string> supported;
    float relevance;
};

#endif
