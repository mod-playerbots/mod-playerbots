/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHAT_COMMAND_HANDLER_H
#define _PLAYERBOT_CHAT_COMMAND_HANDLER_H

#include "Bot/Interface/IChatCommandHandler.h"

class PlayerbotAI;

/**
 * @brief Implementation of IChatCommandHandler
 *
 * This handler manages chat command processing for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The handler delegates to PlayerbotAI methods during the transition period.
 */
class ChatCommandHandler : public IChatCommandHandler
{
public:
    explicit ChatCommandHandler(PlayerbotAI* ai) : _botAI(ai) {}
    ~ChatCommandHandler() override = default;

    void HandleCommand(uint32 type, std::string const& text, Player* fromPlayer) override;
    std::string HandleRemoteCommand(std::string const& command) override;
    void QueueChatResponse(ChatQueuedReply const& reply) override;
    bool IsAllowedCommand(std::string const& text) override;

private:
    PlayerbotAI* _botAI;
};

#endif
