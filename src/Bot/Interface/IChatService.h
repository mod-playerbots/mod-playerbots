/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ICHAT_SERVICE_H
#define _PLAYERBOT_ICHAT_SERVICE_H

#include "Common.h"
#include <sstream>

enum class PlayerbotSecurityLevel : uint8;

/**
 * @brief Chat channel identifiers for bot communication
 */
enum class ChatChannelType
{
    GUILD,
    WORLD,
    GENERAL,
    TRADE,
    LOOKING_FOR_GROUP,
    LOCAL_DEFENSE,
    WORLD_DEFENSE,
    GUILD_RECRUITMENT,
    SAY,
    WHISPER,
    EMOTE,
    YELL,
    PARTY,
    RAID
};

/**
 * @brief Interface for bot communication
 *
 * This interface abstracts chat and communication operations from PlayerbotAI,
 * enabling isolated testing of chat logic.
 */
class IChatService
{
public:
    virtual ~IChatService() = default;

    // Master communication
    virtual bool TellMaster(std::string const& text, PlayerbotSecurityLevel securityLevel) = 0;
    virtual bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel) = 0;
    virtual bool TellMasterNoFacing(std::string const& text, PlayerbotSecurityLevel securityLevel) = 0;
    virtual bool TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel) = 0;
    virtual bool TellError(std::string const& text, PlayerbotSecurityLevel securityLevel) = 0;

    // Channel communication
    virtual bool SayToGuild(std::string const& msg) = 0;
    virtual bool SayToWorld(std::string const& msg) = 0;
    virtual bool SayToChannel(std::string const& msg, uint32 channelId) = 0;
    virtual bool SayToParty(std::string const& msg) = 0;
    virtual bool SayToRaid(std::string const& msg) = 0;

    // Direct communication
    virtual bool Say(std::string const& msg) = 0;
    virtual bool Yell(std::string const& msg) = 0;
    virtual bool Whisper(std::string const& msg, std::string const& receiverName) = 0;

    // Emotes
    virtual bool PlaySound(uint32 emote) = 0;
    virtual bool PlayEmote(uint32 emote) = 0;

    // Visual feedback
    virtual void Ping(float x, float y) = 0;
};

#endif
