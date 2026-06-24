/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
