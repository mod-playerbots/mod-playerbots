/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DEADSTRATEGY_H
#define PLAYERBOTS_DEADSTRATEGY_H

#include "PassThroughStrategy.h"

class PlayerbotAI;

class DeadStrategy : public PassThroughStrategy
{
public:
    DeadStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "dead"; }
};

#endif
