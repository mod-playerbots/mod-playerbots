/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KITESTRATEGY_H
#define PLAYERBOTS_KITESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class KiteStrategy : public Strategy
{
public:
    KiteStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "kite"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
