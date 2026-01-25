/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PacketHandler.h"

#include "PlayerbotAI.h"

void PacketHandler::HandleBotOutgoingPacket(WorldPacket const& packet)
{
    if (_botAI)
    {
        _botAI->HandleBotOutgoingPacket(packet);
    }
}

void PacketHandler::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    if (_botAI)
    {
        _botAI->HandleMasterIncomingPacket(packet);
    }
}

void PacketHandler::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    if (_botAI)
    {
        _botAI->HandleMasterOutgoingPacket(packet);
    }
}

void PacketHandler::HandleTeleportAck()
{
    if (_botAI)
    {
        _botAI->HandleTeleportAck();
    }
}
