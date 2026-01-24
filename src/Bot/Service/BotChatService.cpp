/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotChatService.h"

#include "PlayerbotAI.h"

bool BotChatService::TellMaster(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (botAI_)
    {
        return botAI_->TellMaster(text, securityLevel);
    }
    return false;
}

bool BotChatService::TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    if (botAI_)
    {
        return botAI_->TellMaster(stream, securityLevel);
    }
    return false;
}

bool BotChatService::TellMasterNoFacing(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (botAI_)
    {
        return botAI_->TellMasterNoFacing(text, securityLevel);
    }
    return false;
}

bool BotChatService::TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    if (botAI_)
    {
        return botAI_->TellMasterNoFacing(stream, securityLevel);
    }
    return false;
}

bool BotChatService::TellError(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (botAI_)
    {
        return botAI_->TellError(text, securityLevel);
    }
    return false;
}

bool BotChatService::SayToGuild(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->SayToGuild(msg);
    }
    return false;
}

bool BotChatService::SayToWorld(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->SayToWorld(msg);
    }
    return false;
}

bool BotChatService::SayToChannel(std::string const& msg, uint32 channelId)
{
    if (botAI_)
    {
        return botAI_->SayToChannel(msg, static_cast<ChatChannelId>(channelId));
    }
    return false;
}

bool BotChatService::SayToParty(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->SayToParty(msg);
    }
    return false;
}

bool BotChatService::SayToRaid(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->SayToRaid(msg);
    }
    return false;
}

bool BotChatService::Say(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->Say(msg);
    }
    return false;
}

bool BotChatService::Yell(std::string const& msg)
{
    if (botAI_)
    {
        return botAI_->Yell(msg);
    }
    return false;
}

bool BotChatService::Whisper(std::string const& msg, std::string const& receiverName)
{
    if (botAI_)
    {
        return botAI_->Whisper(msg, receiverName);
    }
    return false;
}

bool BotChatService::PlaySound(uint32 emote)
{
    if (botAI_)
    {
        return botAI_->PlaySound(emote);
    }
    return false;
}

bool BotChatService::PlayEmote(uint32 emote)
{
    if (botAI_)
    {
        return botAI_->PlayEmote(emote);
    }
    return false;
}

void BotChatService::Ping(float x, float y)
{
    if (botAI_)
    {
        botAI_->Ping(x, y);
    }
}
