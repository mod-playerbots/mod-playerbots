/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_CHAT_SERVICE_H
#define _PLAYERBOT_BOT_CHAT_SERVICE_H

#include "Bot/Interface/IChatService.h"

class PlayerbotAI;

/**
 * @brief Implementation of IChatService
 *
 * This service provides communication functionality for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The service delegates to PlayerbotAI methods during the transition period.
 */
class BotChatService : public IChatService
{
public:
    explicit BotChatService(PlayerbotAI* ai) : botAI_(ai) {}
    ~BotChatService() override = default;

    // Master communication
    bool TellMaster(std::string const& text, PlayerbotSecurityLevel securityLevel) override;
    bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel) override;
    bool TellMasterNoFacing(std::string const& text, PlayerbotSecurityLevel securityLevel) override;
    bool TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel) override;
    bool TellError(std::string const& text, PlayerbotSecurityLevel securityLevel) override;

    // Channel communication
    bool SayToGuild(std::string const& msg) override;
    bool SayToWorld(std::string const& msg) override;
    bool SayToChannel(std::string const& msg, uint32 channelId) override;
    bool SayToParty(std::string const& msg) override;
    bool SayToRaid(std::string const& msg) override;

    // Direct communication
    bool Say(std::string const& msg) override;
    bool Yell(std::string const& msg) override;
    bool Whisper(std::string const& msg, std::string const& receiverName) override;

    // Emotes
    bool PlaySound(uint32 emote) override;
    bool PlayEmote(uint32 emote) override;

    // Visual feedback
    void Ping(float x, float y) override;

private:
    PlayerbotAI* botAI_;
};

#endif
