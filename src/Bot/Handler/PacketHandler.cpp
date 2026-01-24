/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PacketHandler.h"

#include "PlayerbotAI.h"

void PacketHandler::HandleBotOutgoingPacket(WorldPacket const& packet)
{
    if (botAI_)
    {
        botAI_->HandleBotOutgoingPacket(packet);
    }
}

void PacketHandler::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    if (botAI_)
    {
        botAI_->HandleMasterIncomingPacket(packet);
    }
}

void PacketHandler::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    if (botAI_)
    {
        botAI_->HandleMasterOutgoingPacket(packet);
    }
}

void PacketHandler::HandleTeleportAck()
{
    if (botAI_)
    {
        botAI_->HandleTeleportAck();
    }
}
