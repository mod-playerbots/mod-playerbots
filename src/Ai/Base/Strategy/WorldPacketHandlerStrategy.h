/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDPACKETHANDLERSTRATEGY_H
#define PLAYERBOTS_WORLDPACKETHANDLERSTRATEGY_H

#include "PassThroughStrategy.h"

class PlayerbotAI;

class WorldPacketHandlerStrategy : public PassThroughStrategy
{
public:
    WorldPacketHandlerStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "default"; }
};

class ReadyCheckStrategy : public PassThroughStrategy
{
public:
    ReadyCheckStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) { }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "ready check"; }
};

#endif
