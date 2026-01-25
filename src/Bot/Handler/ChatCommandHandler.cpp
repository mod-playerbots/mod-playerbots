/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandler.h"

#include "PlayerbotAI.h"

void ChatCommandHandler::HandleCommand(uint32 type, std::string const& text, Player* fromPlayer)
{
    if (_botAI)
    {
        _botAI->HandleCommand(type, text, fromPlayer);
    }
}

std::string ChatCommandHandler::HandleRemoteCommand(std::string const& command)
{
    if (_botAI)
    {
        return _botAI->HandleRemoteCommand(command);
    }
    return "";
}

void ChatCommandHandler::QueueChatResponse(ChatQueuedReply const& reply)
{
    if (_botAI)
    {
        _botAI->QueueChatResponse(reply);
    }
}

bool ChatCommandHandler::IsAllowedCommand(std::string const& text)
{
    if (_botAI)
    {
        return _botAI->IsAllowedCommand(text);
    }
    return false;
}
