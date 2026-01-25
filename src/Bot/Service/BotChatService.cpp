/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotChatService.h"

#include "Bot/Core/ManagerRegistry.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "WorldPacket.h"
#include "WorldSession.h"

// ============================================================================
// Static method implementations (main logic)
// ============================================================================

bool BotChatService::IsTellAllowedStatic(Player* bot, Player* master, PlayerbotSecurity* security,
                                         PlayerbotSecurityLevel securityLevel)
{
    if (!master || master->IsBeingTeleported())
        return false;

    if (!security->CheckLevelFor(securityLevel, true, master))
        return false;

    if (sPlayerbotAIConfig->whisperDistance && !bot->GetGroup() && sManagerRegistry.GetRandomBotManager().IsRandomBot(bot) &&
        master->GetSession()->GetSecurity() < SEC_GAMEMASTER &&
        (bot->GetMapId() != master->GetMapId() ||
         sServerFacade->GetDistance2d(bot, master) > sPlayerbotAIConfig->whisperDistance))
        return false;

    return true;
}

bool BotChatService::TellMasterNoFacingStatic(ChatContext const& ctx, std::string const& text,
                                              PlayerbotSecurityLevel securityLevel)
{
    Player* master = ctx.getMaster();
    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);

    if ((!master || (masterBotAI && !masterBotAI->IsRealPlayer())) &&
        (sPlayerbotAIConfig->randomBotSayWithoutMaster || ctx.hasStrategy("debug", BOT_STATE_NON_COMBAT)))
    {
        ctx.bot->Say(text, (ctx.bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        return true;
    }

    if (!IsTellAllowedStatic(ctx.bot, master, ctx.security, securityLevel))
        return false;

    time_t lastSaid = (*ctx.whispers)[text];

    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        (*ctx.whispers)[text] = time(nullptr);

        ChatMsg type = CHAT_MSG_WHISPER;
        if (ctx.currentChat->second - time(nullptr) >= 1)
            type = ctx.currentChat->first;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, type == CHAT_MSG_ADDON ? CHAT_MSG_PARTY : type,
                                     type == CHAT_MSG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, ctx.bot, nullptr,
                                     text.c_str());
        master->SendDirectMessage(&data);
    }

    return true;
}

bool BotChatService::TellMasterStatic(ChatContext const& ctx, std::string const& text,
                                      PlayerbotSecurityLevel securityLevel)
{
    Player* master = ctx.getMaster();
    if (!master)
    {
        if (sPlayerbotAIConfig->randomBotSayWithoutMaster)
            return TellMasterNoFacingStatic(ctx, text, securityLevel);
    }
    if (!TellMasterNoFacingStatic(ctx, text, securityLevel))
        return false;

    if (!ctx.bot->isMoving() && !ctx.bot->IsInCombat() && ctx.bot->GetMapId() == master->GetMapId() &&
        !ctx.bot->HasUnitState(UNIT_STATE_IN_FLIGHT) && !ctx.bot->IsFlying())
    {
        if (!ctx.bot->HasInArc(EMOTE_ANGLE_IN_FRONT, master, sPlayerbotAIConfig->sightDistance))
            ctx.bot->SetFacingToObject(master);

        ctx.bot->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
    }

    return true;
}

bool BotChatService::TellErrorStatic(Player* bot, Player* master, PlayerbotSecurity* security, std::string const& text,
                                     PlayerbotSecurityLevel securityLevel)
{
    if (!IsTellAllowedStatic(bot, master, security, securityLevel) || !master || GET_PLAYERBOT_AI(master))
        return false;

    if (PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master))
        mgr->TellError(bot->GetName(), text);

    return false;
}

bool BotChatService::SayToGuildStatic(Player* bot, std::string const& msg)
{
    if (msg.empty())
        return false;

    if (bot->GetGuildId())
    {
        if (Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId()))
        {
            if (!guild->HasRankRight(bot, GR_RIGHT_GCHATSPEAK))
                return false;
            guild->BroadcastToGuild(bot->GetSession(), false, msg.c_str(), LANG_UNIVERSAL);
            return true;
        }
    }

    return false;
}

bool BotChatService::SayToWorldStatic(Player* bot, std::string const& msg)
{
    if (msg.empty())
        return false;

    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    if (!cMgr)
        return false;

    if (Channel* worldChannel = cMgr->GetChannel("World", bot))
    {
        worldChannel->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        return true;
    }

    return false;
}

bool BotChatService::SayToChannelStatic(Player* bot, std::string const& msg, uint32 channelId)
{
    if (msg.empty())
        return false;

    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    if (!cMgr)
        return false;

    AreaTableEntry const* current_zone = sAreaTableStore.LookupEntry(bot->GetZoneId());
    if (!current_zone)
        return false;

    std::string current_str_zone = current_zone->area_name[0];

    std::mutex socialMutex;
    std::lock_guard<std::mutex> lock(socialMutex);

    for (auto const& [key, channel] : cMgr->GetChannels())
    {
        if (!channel)
            continue;

        if (channel->GetChannelId() == channelId)
        {
            if (channel->GetName().empty())
                continue;

            const auto does_contains = channel->GetName().find(current_str_zone) != std::string::npos;
            if (channelId != static_cast<uint32>(ChatChannelId::LOOKING_FOR_GROUP) &&
                channelId != static_cast<uint32>(ChatChannelId::WORLD_DEFENSE) && !does_contains)
            {
                continue;
            }

            if (channel)
            {
                channel->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
                return true;
            }
        }
    }

    return false;
}

bool BotChatService::SayToPartyStatic(Player* bot, std::vector<Player*> const& recipients, std::string const& msg)
{
    if (!bot->GetGroup())
        return false;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_PARTY, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
                                 bot->GetName());

    for (auto* receiver : recipients)
    {
        sServerFacade->SendPacket(receiver, &data);
    }

    return true;
}

bool BotChatService::SayToRaidStatic(Player* bot, std::vector<Player*> const& recipients, std::string const& msg)
{
    if (!bot->GetGroup() || bot->GetGroup()->isRaidGroup())
        return false;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
                                 bot->GetName());

    for (auto* receiver : recipients)
    {
        sServerFacade->SendPacket(receiver, &data);
    }

    return true;
}

bool BotChatService::SayStatic(Player* bot, std::string const& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Say(msg, LANG_COMMON);
    }
    else
    {
        bot->Say(msg, LANG_ORCISH);
    }

    return true;
}

bool BotChatService::YellStatic(Player* bot, std::string const& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Yell(msg, LANG_COMMON);
    }
    else
    {
        bot->Yell(msg, LANG_ORCISH);
    }

    return true;
}

bool BotChatService::WhisperStatic(Player* bot, std::string const& msg, std::string const& receiverName)
{
    const auto receiver = ObjectAccessor::FindPlayerByName(receiverName);
    if (!receiver)
        return false;

    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Whisper(msg, LANG_COMMON, receiver);
    }
    else
    {
        bot->Whisper(msg, LANG_ORCISH, receiver);
    }

    return true;
}

bool BotChatService::PlaySoundStatic(Player* bot, uint32 emote)
{
    bot->PlayDistanceSound(emote);
    return true;
}

bool BotChatService::PlayEmoteStatic(Player* bot, uint32 emote)
{
    bot->HandleEmoteCommand(emote);
    return true;
}

void BotChatService::PingStatic(Player* bot, float x, float y)
{
    WorldPacket data(MSG_MINIMAP_PING, (8 + 4 + 4));
    data << bot->GetGUID();
    data << x;
    data << y;

    if (bot->GetGroup())
    {
        bot->GetGroup()->BroadcastPacket(&data, true, -1, bot->GetGUID());
    }
    else
    {
        bot->GetSession()->SendPacket(&data);
    }
}

// ============================================================================
// Helper methods
// ============================================================================

ChatContext BotChatService::BuildChatContext() const
{
    ChatContext ctx;
    if (!_botAI)
    {
        ctx.bot = nullptr;
        ctx.getMaster = []() { return nullptr; };
        ctx.security = nullptr;
        ctx.whispers = nullptr;
        ctx.currentChat = nullptr;
        ctx.hasStrategy = [](std::string const&, int) { return false; };
        ctx.hasRealPlayerMaster = []() { return false; };
        return ctx;
    }

    ctx.bot = _botAI->GetBot();
    ctx.getMaster = [this]() { return _botAI->GetMaster(); };
    ctx.security = _botAI->GetSecurity();

    // We need access to private members, so we'll use PlayerbotAI methods directly for these
    // For now, we delegate to PlayerbotAI
    return ctx;
}

// ============================================================================
// Instance method implementations (IChatService interface)
// These delegate to static methods or PlayerbotAI during transition
// ============================================================================

bool BotChatService::TellMaster(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (_botAI)
    {
        return _botAI->TellMaster(text, securityLevel);
    }
    return false;
}

bool BotChatService::TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMaster(stream.str(), securityLevel);
}

bool BotChatService::TellMasterNoFacing(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (_botAI)
    {
        return _botAI->TellMasterNoFacing(text, securityLevel);
    }
    return false;
}

bool BotChatService::TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMasterNoFacing(stream.str(), securityLevel);
}

bool BotChatService::TellError(std::string const& text, PlayerbotSecurityLevel securityLevel)
{
    if (_botAI)
    {
        return _botAI->TellError(text, securityLevel);
    }
    return false;
}

bool BotChatService::SayToGuild(std::string const& msg)
{
    if (_botAI)
    {
        return SayToGuildStatic(_botAI->GetBot(), msg);
    }
    return false;
}

bool BotChatService::SayToWorld(std::string const& msg)
{
    if (_botAI)
    {
        return SayToWorldStatic(_botAI->GetBot(), msg);
    }
    return false;
}

bool BotChatService::SayToChannel(std::string const& msg, uint32 channelId)
{
    if (_botAI)
    {
        return SayToChannelStatic(_botAI->GetBot(), msg, channelId);
    }
    return false;
}

bool BotChatService::SayToParty(std::string const& msg)
{
    if (_botAI)
    {
        return SayToPartyStatic(_botAI->GetBot(), _botAI->GetPlayersInGroup(), msg);
    }
    return false;
}

bool BotChatService::SayToRaid(std::string const& msg)
{
    if (_botAI)
    {
        return SayToRaidStatic(_botAI->GetBot(), _botAI->GetPlayersInGroup(), msg);
    }
    return false;
}

bool BotChatService::Say(std::string const& msg)
{
    if (_botAI)
    {
        return SayStatic(_botAI->GetBot(), msg);
    }
    return false;
}

bool BotChatService::Yell(std::string const& msg)
{
    if (_botAI)
    {
        return YellStatic(_botAI->GetBot(), msg);
    }
    return false;
}

bool BotChatService::Whisper(std::string const& msg, std::string const& receiverName)
{
    if (_botAI)
    {
        return WhisperStatic(_botAI->GetBot(), msg, receiverName);
    }
    return false;
}

bool BotChatService::PlaySound(uint32 emote)
{
    if (_botAI)
    {
        return PlaySoundStatic(_botAI->GetBot(), emote);
    }
    return false;
}

bool BotChatService::PlayEmote(uint32 emote)
{
    if (_botAI)
    {
        return PlayEmoteStatic(_botAI->GetBot(), emote);
    }
    return false;
}

void BotChatService::Ping(float x, float y)
{
    if (_botAI)
    {
        PingStatic(_botAI->GetBot(), x, y);
    }
}
