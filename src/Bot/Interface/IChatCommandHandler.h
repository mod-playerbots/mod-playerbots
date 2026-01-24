/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ICHAT_COMMAND_HANDLER_H
#define _PLAYERBOT_ICHAT_COMMAND_HANDLER_H

#include "Common.h"
#include <string>

class Player;
struct ChatQueuedReply;

/**
 * @brief Interface for handling chat commands
 *
 * This interface abstracts command handling operations from PlayerbotAI,
 * enabling isolated testing of command processing logic.
 */
class IChatCommandHandler
{
public:
    virtual ~IChatCommandHandler() = default;

    /**
     * @brief Handle a chat command from a player
     * @param type Message type (whisper, say, etc.)
     * @param text Command text
     * @param fromPlayer Player sending the command
     */
    virtual void HandleCommand(uint32 type, std::string const& text, Player* fromPlayer) = 0;

    /**
     * @brief Handle a remote command (from external tools)
     * @param command Command string
     * @return Response string
     */
    virtual std::string HandleRemoteCommand(std::string const& command) = 0;

    /**
     * @brief Queue a chat response for later processing
     * @param reply The queued reply to send
     */
    virtual void QueueChatResponse(ChatQueuedReply const& reply) = 0;

    /**
     * @brief Check if a command is allowed without security check
     * @param text Command text
     * @return true if allowed
     */
    virtual bool IsAllowedCommand(std::string const& text) = 0;
};

#endif
