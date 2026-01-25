/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_CHAT_SERVICE_H
#define _PLAYERBOT_BOT_CHAT_SERVICE_H

#include "Bot/Interface/IChatService.h"
#include "SharedDefines.h"

#include <functional>
#include <map>

class Player;
class PlayerbotAI;
class PlayerbotSecurity;

/**
 * @brief Context for chat operations
 *
 * Provides all dependencies needed for chat operations without
 * requiring direct access to PlayerbotAI.
 */
struct ChatContext
{
    Player* bot;
    std::function<Player*()> getMaster;
    PlayerbotSecurity* security;
    std::map<std::string, time_t>* whispers;
    std::pair<ChatMsg, time_t>* currentChat;
    std::function<bool(std::string const&, int)> hasStrategy;
    std::function<bool()> hasRealPlayerMaster;
};

/**
 * @brief Implementation of IChatService
 *
 * This service provides communication functionality for bots.
 *
 * Static methods are provided for direct access without needing a service instance.
 * Instance methods implement the IChatService interface for testability/mockability.
 */
class BotChatService : public IChatService
{
public:
    BotChatService() = default;
    explicit BotChatService(PlayerbotAI* ai) : botAI_(ai) {}
    ~BotChatService() override = default;

    // ========================================================================
    // Static methods for direct access (main implementations)
    // These can be called without a service instance
    // ========================================================================

    // Master communication - static versions
    static bool IsTellAllowedStatic(Player* bot, Player* master, PlayerbotSecurity* security,
                                    PlayerbotSecurityLevel securityLevel);
    static bool TellMasterNoFacingStatic(ChatContext const& ctx, std::string const& text,
                                         PlayerbotSecurityLevel securityLevel);
    static bool TellMasterStatic(ChatContext const& ctx, std::string const& text, PlayerbotSecurityLevel securityLevel);
    static bool TellErrorStatic(Player* bot, Player* master, PlayerbotSecurity* security, std::string const& text,
                                PlayerbotSecurityLevel securityLevel);

    // Channel communication - static versions
    static bool SayToGuildStatic(Player* bot, std::string const& msg);
    static bool SayToWorldStatic(Player* bot, std::string const& msg);
    static bool SayToChannelStatic(Player* bot, std::string const& msg, uint32 channelId);
    static bool SayToPartyStatic(Player* bot, std::vector<Player*> const& recipients, std::string const& msg);
    static bool SayToRaidStatic(Player* bot, std::vector<Player*> const& recipients, std::string const& msg);

    // Direct communication - static versions
    static bool SayStatic(Player* bot, std::string const& msg);
    static bool YellStatic(Player* bot, std::string const& msg);
    static bool WhisperStatic(Player* bot, std::string const& msg, std::string const& receiverName);

    // Emotes and visual - static versions
    static bool PlaySoundStatic(Player* bot, uint32 emote);
    static bool PlayEmoteStatic(Player* bot, uint32 emote);
    static void PingStatic(Player* bot, float x, float y);

    // ========================================================================
    // Instance methods (IChatService interface implementation)
    // These call the static methods internally, but provide mockable interface
    // ========================================================================

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

    // Set the bot context for instance methods that need the bot
    void SetBotContext(PlayerbotAI* ai) { botAI_ = ai; }
    PlayerbotAI* GetBotContext() const { return botAI_; }

private:
    PlayerbotAI* botAI_ = nullptr;

    // Helper to build ChatContext from PlayerbotAI
    ChatContext BuildChatContext() const;
};

#endif
