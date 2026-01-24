/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IPACKET_HANDLER_H
#define _PLAYERBOT_IPACKET_HANDLER_H

class WorldPacket;

/**
 * @brief Interface for handling network packets
 *
 * This interface abstracts packet handling operations from PlayerbotAI,
 * enabling isolated testing of packet processing logic.
 */
class IPacketHandler
{
public:
    virtual ~IPacketHandler() = default;

    /**
     * @brief Handle packets sent by the bot
     * @param packet The outgoing packet from bot
     */
    virtual void HandleBotOutgoingPacket(WorldPacket const& packet) = 0;

    /**
     * @brief Handle packets received by the master from the server
     * @param packet The incoming packet to master
     */
    virtual void HandleMasterIncomingPacket(WorldPacket const& packet) = 0;

    /**
     * @brief Handle packets sent by the master to the server
     * @param packet The outgoing packet from master
     */
    virtual void HandleMasterOutgoingPacket(WorldPacket const& packet) = 0;

    /**
     * @brief Handle teleport acknowledgment
     */
    virtual void HandleTeleportAck() = 0;
};

#endif
