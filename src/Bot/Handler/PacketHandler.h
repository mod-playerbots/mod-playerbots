/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PACKET_HANDLER_H
#define _PLAYERBOT_PACKET_HANDLER_H

#include "Bot/Interface/IPacketHandler.h"

class PlayerbotAI;

/**
 * @brief Implementation of IPacketHandler
 *
 * This handler manages network packet processing for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The handler delegates to PlayerbotAI methods during the transition period.
 */
class PacketHandler : public IPacketHandler
{
public:
    explicit PacketHandler(PlayerbotAI* ai) : botAI_(ai) {}
    ~PacketHandler() override = default;

    void HandleBotOutgoingPacket(WorldPacket const& packet) override;
    void HandleMasterIncomingPacket(WorldPacket const& packet) override;
    void HandleMasterOutgoingPacket(WorldPacket const& packet) override;
    void HandleTeleportAck() override;

private:
    PlayerbotAI* botAI_;
};

#endif
