/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotMgr.h"

#include <cstdio>
#include <cstring>
#include <istream>
#include <string>
#include <unordered_set>
#include <openssl/sha.h>
#include <algorithm>
#include <chrono>
#include <unordered_map>

#include "ChannelMgr.h"
#include "CharacterCache.h"
#include "CharacterPackets.h"
#include "Common.h"
#include "CryptoRandom.h"
#include "Define.h"
#include "Group.h"
#include "GroupMgr.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotRepository.h"
#include "PlayerbotFactory.h"
#include "PlayerbotOperations.h"
#include "PlayerbotSecurity.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "PlayerbotGuildMgr.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "Timer.h"
#include "WorldSession.h"
#include "BroadcastHelper.h"
#include "WorldSessionMgr.h"
#include "DatabaseEnv.h"

namespace {
    // Invite code constants
    constexpr uint32 INVITE_CODE_LENGTH = 14; // Format: XXXX-XXXX-XXXX
    constexpr uint32 INVITE_CODE_EXPIRY_MINUTES = 60;
    constexpr uint32 MAX_INVITE_CODES_PER_ACCOUNT = 3;

    // Helper function to get current timestamp
    uint64 GetCurrentTimestamp()
    {
        return GetEpochTime().count();
    }

    // Generic error response to prevent information leakage
    void SendGenericError(ChatHandler& handler, const std::string& operation)
    {
        handler.PSendSysMessage("Unable to complete {} operation. Please verify your parameters and try again.", operation.c_str());
    }

    // Helper function to generate secure invite code using AzerothCore's OpenSSL-based crypto random
    std::string GenerateInviteCode()
    {
        const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"; // Excluded confusing chars (0,O,1,I)
        const size_t charCount = sizeof(chars) - 1;

        // Generate 12 cryptographically secure random bytes for character selection
        std::array<uint8, 12> randomBytes;
        Acore::Crypto::GetRandomBytes(randomBytes);

        // Generate code in format XXXX-XXXX-XXXX using OpenSSL's secure random generator
        std::string code;
        for (int i = 0; i < 12; ++i)
        {
            if (i == 4 || i == 8)
            {
                code += '-';
            }
            // Use secure random bytes from OpenSSL
            code += chars[randomBytes[i] % charCount];
        }

        return code;
    }

    // Helper function to check if invite code exists in database
    bool InviteCodeExists(const std::string& code)
    {
        QueryResult existing = PlayerbotsDatabase.Query(
            "SELECT 1 FROM playerbots_invite_codes WHERE code = '{}' AND status = 1 LIMIT 1", code);
        return existing != nullptr;
    }

    // Helper function to validate invite code format
    bool IsValidInviteCodeFormat(const std::string& code)
    {
        if (code.length() != INVITE_CODE_LENGTH)
        {
            return false;
        }

        // Check format: XXXX-XXXX-XXXX
        if (code[4] != '-' || code[9] != '-')
        {
            return false;
        }

        // Check characters (only allowed chars)
        for (size_t i = 0; i < code.length(); ++i)
        {
            if (i == 4 || i == 9) continue; // Skip dashes
            char c = code[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '9')) || c == 'O' || c == 'I')
            {
                return false;
            }
        }
        return true;
    }

}

class BotInitGuard
{
public:
    BotInitGuard(ObjectGuid guid) : guid(guid), active(false)
    {
        if (!botsBeingInitialized.contains(guid))
        {
            botsBeingInitialized.insert(guid);
            active = true;
        }
    }

    ~BotInitGuard()
    {
        if (active)
            botsBeingInitialized.erase(guid);
    }

    bool IsLocked() const { return !active; }

private:
    ObjectGuid guid;
    bool active;
    static std::unordered_set<ObjectGuid> botsBeingInitialized;
};

std::unordered_set<ObjectGuid> BotInitGuard::botsBeingInitialized;
std::unordered_set<ObjectGuid> PlayerbotHolder::botLoading;

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase(false) {}
class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
private:
    uint32 masterAccountId;
    PlayerbotHolder* playerbotHolder;
public:
    PlayerbotLoginQueryHolder(uint32 masterAccount, uint32 accountId, ObjectGuid guid)
        : LoginQueryHolder(accountId, guid), masterAccountId(masterAccount)
    {
    }

    uint32 GetMasterAccountId() const { return masterAccountId; }
};

void PlayerbotHolder::AddPlayerBot(ObjectGuid playerGuid, uint32 masterAccountId)
{
    if (botLoading.find(playerGuid) != botLoading.end())
        return;

    // has bot already been added?
    Player* bot = ObjectAccessor::FindConnectedPlayer(playerGuid);
    if (bot && bot->IsInWorld())
        return;

    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(playerGuid);
    if (!accountId)
        return;

    WorldSession* masterSession = masterAccountId ? sWorldSessionMgr->FindSession(masterAccountId) : nullptr;
    Player* masterPlayer = masterSession ? masterSession->GetPlayer() : nullptr;

    bool isRndbot = !masterAccountId;
    bool sameAccount = sPlayerbotAIConfig.allowAccountBots && accountId == masterAccountId;
    Guild* guild = masterPlayer ? sGuildMgr->GetGuildById(masterPlayer->GetGuildId()) : nullptr;
    bool sameGuild = sPlayerbotAIConfig.allowGuildBots && guild && guild->GetMember(playerGuid);
    bool addClassBot = sRandomPlayerbotMgr.IsAddclassBot(playerGuid.GetCounter());
    bool linkedAccount = sPlayerbotAIConfig.allowTrustedAccountBots && IsAccountLinked(accountId, masterAccountId);

    bool allowed = true;
    std::ostringstream out;
    std::string botName;
    sCharacterCache->GetCharacterNameByGuid(playerGuid, botName);
    if (!isRndbot && !sameAccount && !sameGuild && !addClassBot && !linkedAccount)
    {
        allowed = false;
        out << "Failure: You are not allowed to control bot " << botName.c_str();
    }
    if (masterAccountId && masterPlayer)
    {
        PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(masterPlayer);
        if (!mgr)
        {
            LOG_DEBUG("playerbots", "PlayerbotMgr not found for master player with GUID: {}", masterPlayer->GetGUID().GetRawValue());
            return;
        }
        uint32 count = mgr->GetPlayerbotsCount() + botLoading.size();
        if (count >= sPlayerbotAIConfig.maxAddedBots)
        {
            allowed = false;
            out << "Failure: You have added too many bots (more than " << sPlayerbotAIConfig.maxAddedBots << ")";
        }
    }
    if (!allowed)
    {
        if (masterSession)
        {
            ChatHandler ch(masterSession);
            ch.SendSysMessage(out.str());
        }
        return;
    }
    std::shared_ptr<PlayerbotLoginQueryHolder> holder =
        std::make_shared<PlayerbotLoginQueryHolder>(masterAccountId, accountId, playerGuid);
    if (!holder->Initialize())
    {
        return;
    }

    botLoading.insert(playerGuid);

    // Always login in with world session to avoid race condition
    sWorld->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder))
        .AfterComplete(
            [](SQLQueryHolderBase const& queryHolder)
            {
                PlayerbotLoginQueryHolder const& holder = static_cast<PlayerbotLoginQueryHolder const&>(queryHolder);
                uint32 masterAccountId = holder.GetMasterAccountId();

                if (masterAccountId)
                {
                    // verify and find current world session of master
                    WorldSession* masterSession = sWorldSessionMgr->FindSession(masterAccountId);
                    Player* masterPlayer = masterSession ? masterSession->GetPlayer() : nullptr;

                    if (masterPlayer)
                    {
                        PlayerbotHolder* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(masterPlayer);

                        if (mgr != nullptr)
                        {
                            mgr->HandlePlayerBotLoginCallback(holder);

                            return;
                        }

                        PlayerbotHolder::botLoading.erase(holder.GetGuid());

                        return;
                    }
                }

                RandomPlayerbotMgr ::instance().HandlePlayerBotLoginCallback(holder);
            });
}

bool PlayerbotHolder::IsAccountLinked(uint32 accountId, uint32 linkedAccountId)
{
    QueryResult result = PlayerbotsDatabase.Query(
        "SELECT 1 FROM playerbots_account_links WHERE account_id = {} AND linked_account_id = {}", accountId, linkedAccountId);
    return result != nullptr;
}

void PlayerbotHolder::HandlePlayerBotLoginCallback(PlayerbotLoginQueryHolder const& holder)
{
    uint32 botAccountId = holder.GetAccountId();
    // At login DBC locale should be what the server is set to use by default (as spells etc are hardcoded to ENUS this
    // allows channels to work as intended)
    WorldSession* botSession =
        new WorldSession(botAccountId, "", 0x0, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0),
                         sWorld->GetDefaultDbcLocale(), 0, false, false, 0, true);

    botSession->HandlePlayerLoginFromDB(holder);  // will delete lqh

    Player* bot = botSession->GetPlayer();
    if (!bot)
    {
        // Debug log
        LOG_DEBUG("mod-playerbots", "Bot player could not be loaded for account ID: {}", botAccountId);
        botSession->LogoutPlayer(true);
        delete botSession;
        PlayerbotHolder::botLoading.erase(holder.GetGuid());

        return;
    }

    uint32 masterAccountId = holder.GetMasterAccountId();
    WorldSession* masterSession = masterAccountId ? sWorldSessionMgr->FindSession(masterAccountId) : nullptr;

    // Check if masterSession->GetPlayer() is valid
    Player* masterPlayer = masterSession ? masterSession->GetPlayer() : nullptr;
    if (masterSession && !masterPlayer)
    {
        LOG_DEBUG("mod-playerbots", "Master session found but no player is associated for master account ID: {}",
                  masterAccountId);
    }

    sRandomPlayerbotMgr.OnPlayerLogin(bot);
    auto op = std::make_unique<OnBotLoginOperation>(bot->GetGUID(), masterAccountId);
    PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(op));

    PlayerbotHolder::botLoading.erase(holder.GetGuid());
}

void PlayerbotHolder::UpdateSessions()
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI)
            {
                botAI->HandleTeleportAck();
            }
        }
        else if (bot->IsInWorld())
        {
            HandleBotPackets(bot->GetSession());
        }
    }
}

void PlayerbotHolder::HandleBotPackets(WorldSession* session)
{
    WorldPacket* packet;
    while (session->GetPacketQueue().next(packet))
    {
        OpcodeClient opcode = static_cast<OpcodeClient>(packet->GetOpcode());
        ClientOpcodeHandler const* opHandle = opcodeTable[opcode];
        if (!opHandle)
        {
            LOG_ERROR("playerbots", "Unhandled opcode {} queued for bot session {}. Packet dropped.", static_cast<uint32>(opcode), session->GetAccountId());
            delete packet;
            continue;
        }
        opHandle->Call(session, *packet);
        delete packet;
    }
}

void PlayerbotHolder::LogoutAllBots()
{
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd())
            break;

        Player* bot= itr->second;
        if (!GET_PLAYERBOT_AI(bot)->IsRealPlayer())
            LogoutPlayerBot(bot->GetGUID());
    }
    */

    PlayerBotMap bots = playerBots;
    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetGUID());
    }
}

void PlayerbotMgr::CancelLogout()
{
    Player* master = GetMaster();
    if (!master)
        return;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
            botAI->TellMaster("Logout cancelled!");
        }
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin();
         it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (botAI->GetMaster() != master)
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
        }
    }
}

void PlayerbotHolder::LogoutPlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        // Queue group cleanup operation for world thread
        auto cleanupOp = std::make_unique<BotLogoutGroupCleanupOperation>(guid);
        PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(cleanupOp));

        LOG_DEBUG("playerbots", "Bot {} logging out", bot->GetName().c_str());
        bot->SaveToDB(false, false);

        WorldSession* botWorldSessionPtr = bot->GetSession();
        WorldSession* masterWorldSessionPtr = nullptr;

        if (botWorldSessionPtr->isLogingOut())
            return;

        Player* master = botAI->GetMaster();
        if (master)
            masterWorldSessionPtr = master->GetSession();

        // check for instant logout
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

        if (masterWorldSessionPtr && masterWorldSessionPtr->ShouldLogOut(time(nullptr)))
            logout = true;

        if (masterWorldSessionPtr && !masterWorldSessionPtr->GetPlayer())
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master &&
            (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
             (masterWorldSessionPtr &&
              masterWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        TravelTarget* target = nullptr;
        if (botAI->GetAiObjectContext())  // Maybe some day re-write to delate all pointer values.
        {
            target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        }

        // Peiru: Allow bots to always instant logout to see if this resolves logout crashes
        logout = true;

        // if no instant logout, request normal logout
        if (!logout)
        {
            if (bot->GetSession()->isLogingOut())
                return;
            else if (bot)
            {
                botAI->TellMaster("I'm logging out!");
                WorldPackets::Character::LogoutRequest data = WorldPacket(CMSG_LOGOUT_REQUEST);
                botWorldSessionPtr->HandleLogoutRequestOpcode(data);
                if (!bot)
                {
                    RemoveFromPlayerbotsMap(guid);
                    delete botWorldSessionPtr;
                    if (target)
                        delete target;
                }
                return;
            }
            else
            {
                RemoveFromPlayerbotsMap(guid);     // deletes bot player ptr inside this WorldSession PlayerBotMap
                delete botWorldSessionPtr;  // finally delete the bot's WorldSession
                if (target)
                    delete target;
            }
            return;
        }  // if instant logout possible, do it
        else if (bot && (logout || !botWorldSessionPtr->isLogingOut()))
        {
            botAI->TellMaster("Goodbye!");
            RemoveFromPlayerbotsMap(guid);                  // deletes bot player ptr inside this WorldSession PlayerBotMap
            botWorldSessionPtr->LogoutPlayer(true);  // this will delete the bot Player object and PlayerbotAI object
            delete botWorldSessionPtr;               // finally delete the bot's WorldSession
        }
    }
}

void PlayerbotHolder::DisablePlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
        {
            return;
        }
        botAI->TellMaster("Goodbye!");
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleground() && !bot->InBattlegroundQueue() && botAI->HasActivePlayerMaster())
        {
            PlayerbotRepository::instance().Save(botAI);
        }

        LOG_DEBUG("playerbots", "Bot {} logged out", bot->GetName().c_str());

        bot->SaveToDB(false, false);

        if (botAI->GetAiObjectContext())  // Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        RemoveFromPlayerbotsMap(guid);  // deletes bot player ptr inside this WorldSession PlayerBotMap

        delete botAI;
    }
}

void PlayerbotHolder::RemoveFromPlayerbotsMap(ObjectGuid guid)
{
    playerBots.erase(guid);
}

Player* PlayerbotHolder::GetPlayerBot(ObjectGuid playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

Player* PlayerbotHolder::GetPlayerBot(ObjectGuid::LowType lowGuid) const
{
    ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(lowGuid);
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

void PlayerbotHolder::OnBotLogin(Player* const bot)
{
    // Prevent duplicate login
    if (playerBots.find(bot->GetGUID()) != playerBots.end())
    {
        return;
    }

    PlayerbotsMgr::instance().AddPlayerbotData(bot, true);
    playerBots[bot->GetGUID()] = bot;

    OnBotLoginInternal(bot);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        // Log a warning here to indicate that the botAI is null
        LOG_DEBUG("mod-playerbots", "PlayerbotAI is null for bot with GUID: {}", bot->GetGUID().GetRawValue());
        return;
    }

    Player* master = botAI->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        if (master->GetGroup() && !master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            if (master)
            {
                if (master->GetGUID() == member)
                {
                    groupValid = true;
                    break;
                }
            }

            // Don't disband alt groups when master goes away
            // Controlled by config
            if (sPlayerbotAIConfig.KeepAltsInGroup())
            {
                uint32 account = sCharacterCache->GetCharacterAccountIdByGuid(member);
                if (!sPlayerbotAIConfig.IsInRandomAccountList(account))
                {
                    groupValid = true;
                    break;
                }
            }
        }

        if (!groupValid)
        {
            botAI->LeaveOrDisbandGroup();
        }
    }

    group = bot->GetGroup();
    if (group)
    {
        botAI->ResetStrategies();
    }
    else
    {
        botAI->ResetStrategies(!sRandomPlayerbotMgr.IsRandomBot(bot));
    }
    PlayerbotRepository::instance().Load(botAI);

    if (master && !master->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        bot->GetMotionMaster()->MovementExpired();
        bot->CleanupAfterTaxiFlight();
    }

    // check activity
    botAI->AllowActivity(ALL_ACTIVITY, true);

    // set delay on login
    botAI->SetNextCheckDelay(urand(2000, 4000));

    botAI->TellMaster("Hello!", PLAYERBOT_SECURITY_TALK);

    // Queue group operations for world thread
    if (master && master->GetGroup() && !group)
    {
        Group* mgroup = master->GetGroup();
        if (mgroup->GetMembersCount() >= 5)
        {
            if (!mgroup->isRaidGroup() && !mgroup->isLFGGroup() && !mgroup->isBGGroup() && !mgroup->isBFGroup())
            {
                // Queue ConvertToRaid operation
                auto convertOp = std::make_unique<GroupConvertToRaidOperation>(master->GetGUID());
                PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(convertOp));
            }
            if (mgroup->isRaidGroup())
            {
                // Queue AddMember operation
                auto addOp = std::make_unique<GroupInviteOperation>(master->GetGUID(), bot->GetGUID());
                PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(addOp));
            }
        }
        else
        {
            // Queue AddMember operation
            auto addOp = std::make_unique<GroupInviteOperation>(master->GetGUID(), bot->GetGUID());
            PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(addOp));
        }
    }
    else if (master && !group)
    {
        // Queue group creation and AddMember operation
        auto inviteOp = std::make_unique<GroupInviteOperation>(master->GetGUID(), bot->GetGUID());
        PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(inviteOp));
    }
    // if (master)
    // {
    //     // bot->TeleportTo(master);
    // }
    uint32 accountId = bot->GetSession()->GetAccountId();
    bool isRandomAccount = sPlayerbotAIConfig.IsInRandomAccountList(accountId);

    if (isRandomAccount && sPlayerbotAIConfig.randomBotFixedLevel)
    {
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }
    else if (isRandomAccount && !sPlayerbotAIConfig.randomBotFixedLevel)
    {
        bot->RemovePlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }

    bot->SaveToDB(false, false);
    bool addClassBot = sRandomPlayerbotMgr.IsAccountType(accountId, 2);
    if (addClassBot && master && abs((int)master->GetLevel() - (int)bot->GetLevel()) > 3)
    {
        // PlayerbotFactory factory(bot, master->GetLevel());
        // factory.Randomize(false);
        uint32 mixedGearScore =
            PlayerbotAI::GetMixedGearScore(master, true, false, 12) * sPlayerbotAIConfig.autoInitEquipLevelLimitRatio;
        // work around: distinguish from 0 if no gear
        if (mixedGearScore == 0)
            mixedGearScore = 1;
        PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
        factory.Randomize(false);
    }

    // bots join World chat if not solo oriented
    if (bot->GetLevel() >= 10 && sRandomPlayerbotMgr.IsRandomBot(bot) && GET_PLAYERBOT_AI(bot) &&
        GET_PLAYERBOT_AI(bot)->GetGrouperType() != GrouperType::SOLO)
    {
        // TODO make action/config
        // Make the bot join the world channel for chat
        WorldPacket pkt(CMSG_JOIN_CHANNEL);
        pkt << uint32(0) << uint8(0) << uint8(0);
        pkt << std::string("World");
        pkt << "";  // Pass
        bot->GetSession()->HandleJoinChannel(pkt);
    }

    // join standard channels
    uint8 locale = BroadcastHelper::GetLocale();
    AreaTableEntry const* current_zone = GET_PLAYERBOT_AI(bot)->GetCurrentZone();
    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    std::string current_zone_name = current_zone ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_zone) : "";

    if (current_zone && cMgr)
    {
        for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
        {
            ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
            if (!channel)
                continue;

            Channel* new_channel = nullptr;
            switch (channel->ChannelID)
            {
                case ChatChannelId::GENERAL:
                case ChatChannelId::LOCAL_DEFENSE:
                {
                    char new_channel_name_buf[100];
                    snprintf(new_channel_name_buf, 100, channel->pattern[locale], current_zone_name.c_str());
                    new_channel = cMgr->GetJoinChannel(new_channel_name_buf, channel->ChannelID);
                    break;
                }
                case ChatChannelId::TRADE:
                case ChatChannelId::GUILD_RECRUITMENT:
                {
                    char new_channel_name_buf[100];
                    //3459 is ID for a zone named "City" (only exists for the sake of using its name)
                    //Currently in magons TBC, if you switch zones, then you join "Trade - <zone>" and "GuildRecruitment - <zone>"
                    //which is a core bug, should be "Trade - City" and "GuildRecruitment - City" in both 1.12 and TBC
                    //but if you (actual player) logout in a city and log back in - you join "City" versions
                    snprintf(new_channel_name_buf, 100, channel->pattern[locale], GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(GetAreaEntryByAreaID(3459)).c_str());
                    new_channel = cMgr->GetJoinChannel(new_channel_name_buf, channel->ChannelID);
                    break;
                }
                case ChatChannelId::LOOKING_FOR_GROUP:
                case ChatChannelId::WORLD_DEFENSE:
                {
                    new_channel = cMgr->GetJoinChannel(channel->pattern[locale], channel->ChannelID);
                    break;
                }
                default:
                    break;
            }

            if (new_channel)
                new_channel->JoinChannel(bot, "");
        }
    }
}

std::string const PlayerbotHolder::ProcessBotCommand(std::string const cmd, ObjectGuid guid, ObjectGuid masterguid,
                                                     bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig.enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sCharacterCache->GetCharacterAccountIdByGuid(guid);
    //bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(guid.GetCounter()); //not used, line marked for removal.
    //bool isRandomAccount = sPlayerbotAIConfig.IsInRandomAccountList(botAccount); //not used, shadowed, line marked for removal.
    //bool isMasterAccount = (masterAccountId == botAccount); //not used, line marked for removal.

    if (cmd == "add" || cmd == "addaccount" || cmd == "login")
    {
        if (ObjectAccessor::FindPlayer(guid))
            return "player already logged in";

        // For addaccount command, verify it's an account name
        if (cmd == "addaccount")
        {
            uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
            if (!accountId)
                return "character not found";

            if (!sPlayerbotAIConfig->allowAccountBots && accountId != masterAccountId &&
                !(sPlayerbotAIConfig->allowTrustedAccountBots && IsAccountLinked(accountId, masterAccountId)))
            {
                return "you can only add bots from your own account or linked accounts";
            }
        }

        AddPlayerBot(guid, masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!ObjectAccessor::FindPlayer(guid))
            return "player is offline";

        if (!GetPlayerBot(guid))
            return "not your bot";

        LogoutPlayerBot(guid);
        return "ok";
    }

    // if (admin)
    // {
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        bot = sRandomPlayerbotMgr.GetPlayerBot(guid);

    if (!bot)
        return "bot not found";

    bool addClassBot = sRandomPlayerbotMgr.IsAddclassBot(guid.GetCounter());

    if (!addClassBot)
        return "ERROR: You can not use this command on non-addclass bot.";

    if (!admin)
    {
        Player* master = ObjectAccessor::FindConnectedPlayer(masterguid);
        if (master && (master->IsInCombat() || bot->IsInCombat()))
        {
            return "ERROR: You can not use this command during combat.";
        }
    }

    if (GET_PLAYERBOT_AI(bot))
    {
        if (Player* master = GET_PLAYERBOT_AI(bot)->GetMaster())
        {
            if (master->GetSession()->GetSecurity() <= SEC_PLAYER && sPlayerbotAIConfig.autoInitOnly &&
                cmd != "init=auto")
            {
                return "The command is not allowed, use init=auto instead.";
            }

            //  Use boot guard
            BotInitGuard guard(bot->GetGUID());
            if (guard.IsLocked())
            {
                return "Initialization already in progress, please wait.";
            }

            int gs;
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=auto")
            {
                uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(master, true, false, 12) *
                                        sPlayerbotAIConfig.autoInitEquipLevelLimitRatio;
                // work around: distinguish from 0 if no gear
                if (mixedGearScore == 0)
                    mixedGearScore = 1;
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(mixedGearScore / PlayerbotAI::GetItemScoreMultiplier(ItemQualities(ITEM_QUALITY_EPIC))) +
                       "(for epic)";
            }
            else if (cmd.starts_with("init=") && sscanf(cmd.c_str(), "init=%d", &gs) != -1)
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(gs / PlayerbotAI::GetItemScoreMultiplier(ItemQualities(ITEM_QUALITY_EPIC))) + "(for epic)";
            }
        }

        if (cmd == "refresh=raid")
        {  // TODO: This function is not perfect yet. If you are already in a raid,
            // after the command is executed, the AI ​​needs to go back online or exit the raid and re-enter.
            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.UnbindInstance();
            return "ok";
        }
    }

    if (cmd == "levelup" || cmd == "level")
    {
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.Randomize(true);
        return "ok";
    }
    else if (cmd == "refresh")
    {
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.Refresh();
        return "ok";
    }
    else if (cmd == "random")
    {
        sRandomPlayerbotMgr.Randomize(bot);
        return "ok";
    }
    else if (cmd == "quests")
    {
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.InitInstanceQuests();
        return "Initialization quests";
    }
    // }

    return "unknown command";
}

// Added for gender choice : Returns the gender of an offline character: 0 = male, 1 = female.
static uint8 GetOfflinePlayerGender(ObjectGuid guid)
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT gender FROM characters WHERE guid = {}", guid.GetCounter());

    if (result)
        return (*result)[0].Get<uint8>();       // 0 = male, 1 = female

    return GENDER_MALE;                         // fallback value
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig.enabled)
    {
        handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
    }

    WorldSession* m_session = handler->GetSession();
    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
    if (!mgr)
    {
        handler->PSendSysMessage("You cannot control bots yet");
        return false;
    }

    std::vector<std::string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("{}", i->c_str());
    }

    return true;
}

std::vector<std::string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    std::vector<std::string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload/tweak/self or add/addaccount/init/remove PLAYERNAME\n");
        messages.push_back("usage: addclass CLASSNAME [male|female|0|1]");
        return messages;
    }

    char* cmd = strtok((char*)args, " ");
    char* charname = strtok(nullptr, " ");
    char* genderArg = strtok(nullptr, " ");    // Added for gender choice [male|female|0|1] optionnel

    if (!cmd)
    {
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME or addclass CLASSNAME [male|female]");
        return messages;
    }

    if (!strcmp(cmd, "initself"))
    {
        if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        {
            // OnBotLogin(master);
            PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_EPIC);
            factory.Randomize(false);
            messages.push_back("initself ok");
            return messages;
        }
        else
        {
            messages.push_back("ERROR: Only GM can use this command.");
            return messages;
        }
    }

    if (!strncmp(cmd, "initself=", 9))
    {
        if (!strcmp(cmd, "initself=uncommon"))
        {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            }
            else
            {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        if (!strcmp(cmd, "initself=rare"))
        {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            }
            else
            {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        if (!strcmp(cmd, "initself=epic"))
        {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            }
            else
            {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        if (!strcmp(cmd, "initself=legendary"))
        {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            }
            else
            {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        int32 gs;
        if (sscanf(cmd, "initself=%d", &gs) != -1)
        {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
            {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->GetLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                messages.push_back("initself ok, gs = " + std::to_string(gs));
                return messages;
            }
            else
            {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
    }

    if (!strcmp(cmd, "list"))
    {
        messages.push_back(ListBots(master));
        return messages;
    }

    if (!strcmp(cmd, "reload"))
    {
        if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        {
            sPlayerbotAIConfig.Initialize();
            messages.push_back("Config reloaded.");
            return messages;
        }
        else
        {
            messages.push_back("ERROR: Only GM can use this command.");
            return messages;
        }
    }

    if (!strcmp(cmd, "tweak"))
    {
        sPlayerbotAIConfig.tweakValue = sPlayerbotAIConfig.tweakValue++;
        if (sPlayerbotAIConfig.tweakValue > 2)
            sPlayerbotAIConfig.tweakValue = 0;

        messages.push_back("Set tweakvalue to " + std::to_string(sPlayerbotAIConfig.tweakValue));
        return messages;
    }

    if (!strcmp(cmd, "self"))
    {
        if (GET_PLAYERBOT_AI(master))
        {
            messages.push_back("Disable player botAI");
            delete GET_PLAYERBOT_AI(master);
        }
        else if (sPlayerbotAIConfig.selfBotLevel == 0)
            messages.push_back("Self-bot is disabled");
        else if (sPlayerbotAIConfig.selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player botAI");
        else
        {
            messages.push_back("Enable player botAI");
            PlayerbotsMgr::instance().AddPlayerbotData(master, true);
            GET_PLAYERBOT_AI(master)->SetMaster(master);
        }

        return messages;
    }

    if (!strcmp(cmd, "lookup"))
    {
        messages.push_back(LookupBots(master));
        return messages;
    }

    if (!strcmp(cmd, "addclass"))
    {
        if (sPlayerbotAIConfig.addClassCommand == 0 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        {
            messages.push_back("You do not have permission to create bot by addclass command");
            return messages;
        }
        if (!charname)
        {
            messages.push_back(
                "addclass: invalid CLASSNAME(warrior/paladin/hunter/rogue/priest/shaman/mage/warlock/druid/dk)");
            return messages;
        }
        uint8 claz;
        if (!strcmp(charname, "warrior"))
        {
            claz = 1;
        }
        else if (!strcmp(charname, "paladin"))
        {
            claz = 2;
        }
        else if (!strcmp(charname, "hunter"))
        {
            claz = 3;
        }
        else if (!strcmp(charname, "rogue"))
        {
            claz = 4;
        }
        else if (!strcmp(charname, "priest"))
        {
            claz = 5;
        }
        else if (!strcmp(charname, "shaman"))
        {
            claz = 7;
        }
        else if (!strcmp(charname, "mage"))
        {
            claz = 8;
        }
        else if (!strcmp(charname, "warlock"))
        {
            claz = 9;
        }
        else if (!strcmp(charname, "druid"))
        {
            claz = 11;
        }
        else if (!strcmp(charname, "dk"))
        {
            claz = 6;
        }
        else
        {
            messages.push_back("Error: Invalid Class. Try again.");
            return messages;
        }
        //  Added for gender choice : Parsing gender
        int8 gender = -1; // -1 = gender will be random
        if (genderArg)
        {
            std::string g = genderArg;
            std::transform(g.begin(), g.end(), g.begin(), ::tolower);

            if (g == "male" || g == "0")
                gender = GENDER_MALE; // 0
            else if (g == "female" || g == "1")
                gender = GENDER_FEMALE; // 1
            else
            {
                messages.push_back("Unknown gender : " + g + " (male/female/0/1)");
                return messages;
            }
        } //end

        if (claz == 6 && master->GetLevel() < sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL))
        {
            messages.push_back("Your level is too low to summon Deathknight");
            return messages;
        }
        uint8 teamId = master->GetTeamId(true);
        const std::unordered_set<ObjectGuid> &guidCache = sRandomPlayerbotMgr.addclassCache[RandomPlayerbotMgr::GetTeamClassIdx(teamId == TEAM_ALLIANCE, claz)];
        for (const ObjectGuid &guid: guidCache)
        {
            // If the user requested a specific gender, skip any character that doesn't match.
            if (gender != -1 && GetOfflinePlayerGender(guid) != gender)
                continue;
            if (botLoading.find(guid) != botLoading.end())
                continue;
            if (ObjectAccessor::FindConnectedPlayer(guid))
                continue;
            uint32 guildId = sCharacterCache->GetCharacterGuildIdByGuid(guid);
            if (guildId && PlayerbotGuildMgr::instance().IsRealGuild(guildId))
                continue;
            AddPlayerBot(guid, master->GetSession()->GetAccountId());
            messages.push_back("Add class " + std::string(charname));
            return messages;
        }
        messages.push_back("Add class failed, no available characters!");
        return messages;
    }

    std::string charnameStr;

    if (!charname)
    {
        std::string name;
        bool isPlayer = sCharacterCache->GetCharacterNameByGuid(master->GetTarget(), name);
        // Player* tPlayer = ObjectAccessor::FindConnectedPlayer(master->GetTarget());
        if (isPlayer)
        {
            charnameStr = name;
        }
        else
        {
            messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME");
            return messages;
        }
    }
    else
    {
        charnameStr = charname;
    }

    std::string const cmdStr = cmd;

    std::unordered_set<std::string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
            ObjectGuid member = i->guid;

            if (member == master->GetGUID())
                continue;

            std::string bot;
            if (sCharacterCache->GetCharacterNameByGuid(member, bot))
                bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            if (Player* bot = i->second)
                if (bot->IsInWorld())
                    bots.insert(bot->GetName());
        }
    }

    std::vector<std::string> chars = split(charnameStr, ',');
    for (std::vector<std::string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        std::string const s = *i;

        if (!strcmp(cmd, "addaccount"))
        {
            // When using addaccount, first try to get account ID directly
            uint32 accountId = GetAccountId(s);
            if (!accountId)
            {
                // If not found, try to get account ID from character name
                ObjectGuid charGuid = sCharacterCache->GetCharacterGuidByName(s);
                if (!charGuid)
                {
                    messages.push_back("Neither account nor character '" + s + "' found");
                    continue;
                }
                accountId = sCharacterCache->GetCharacterAccountIdByGuid(charGuid);
                if (!accountId)
                {
                    messages.push_back("Could not find account for character '" + s + "'");
                    continue;
                }
            }

            QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId);
            if (results)
            {
                do
                {
                    Field* fields = results->Fetch();
                    std::string const charName = fields[0].Get<std::string>();
                    bots.insert(charName);
                } while (results->NextRow());
            }
        }
        else
        {
            // For regular add command, only add the specific character
            ObjectGuid charGuid = sCharacterCache->GetCharacterGuidByName(s);
            if (!charGuid)
            {
                messages.push_back("Character '" + s + "' not found");
                continue;
            }
            bots.insert(s);
        }
    }

    for (auto i = bots.begin(); i != bots.end(); ++i)
    {
        std::string const bot = *i;

        std::ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sCharacterCache->GetCharacterGuidByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member != master->GetGUID())
        {
            out << ProcessBotCommand(cmdStr, member, master->GetGUID(),
                                     master->GetSession()->GetSecurity() >= SEC_GAMEMASTER,
                                     master->GetSession()->GetAccountId(), master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid::Empty, true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return messages;
}

uint32 PlayerbotHolder::GetAccountId(std::string const name) { return AccountMgr::GetId(name); }

uint32 PlayerbotHolder::GetAccountId(ObjectGuid guid)
{
    if (!guid.IsPlayer())
        return 0;

    // prevent DB access for online player
    if (Player* player = ObjectAccessor::FindConnectedPlayer(guid))
        return player->GetSession()->GetAccountId();

    ObjectGuid::LowType lowguid = guid.GetCounter();

    if (QueryResult result = CharacterDatabase.Query("SELECT account FROM characters WHERE guid = {}", lowguid))
    {
        uint32 acc = (*result)[0].Get<uint32>();
        return acc;
    }

    return 0;
}

std::string const PlayerbotHolder::ListBots(Player* master)
{
    std::set<std::string> bots;
    std::map<uint8, std::string> classNames;

    classNames[CLASS_DEATH_KNIGHT] = "Death Knight";
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";

    std::map<std::string, std::string> online;
    std::vector<std::string> names;
    std::map<std::string, std::string> classes;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        std::string const name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+";
        classes[name] = classNames[bot->getClass()];
    }

    if (master)
    {
        QueryResult results = CharacterDatabase.Query("SELECT class, name FROM characters WHERE account = {}",
                                                      master->GetSession()->GetAccountId());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].Get<uint8>();
                std::string const name = fields[1].Get<std::string>();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-";
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
        }
    }

    std::sort(names.begin(), names.end());

    if (Group* group = master->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* member = ObjectAccessor::FindPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr.IsRandomBot(member))
            {
                std::string const name = member->GetName();

                names.push_back(name);
                online[name] = "+";
                classes[name] = classNames[member->getClass()];
            }
        }
    }

    std::ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first)
            first = false;
        else
            out << ", ";

        std::string const name = *i;
        out << online[name] << name << " " << classes[name];
    }

    return out.str();
}

std::string const PlayerbotHolder::LookupBots(Player* master)
{
    std::list<std::string> messages;
    messages.push_back("Classes Available:");
    messages.push_back("|TInterface\\icons\\INV_Sword_27.png:25:25:0:-1|t Warrior");
    messages.push_back("|TInterface\\icons\\INV_Hammer_01.png:25:25:0:-1|t Paladin");
    messages.push_back("|TInterface\\icons\\INV_Weapon_Bow_07.png:25:25:0:-1|t Hunter");
    messages.push_back("|TInterface\\icons\\INV_ThrowingKnife_04.png:25:25:0:-1|t Rogue");
    messages.push_back("|TInterface\\icons\\INV_Staff_30.png:25:25:0:-1|t Priest");
    messages.push_back("|TInterface\\icons\\inv_jewelry_talisman_04.png:25:25:0:-1|t Shaman");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Mage");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Warlock");
    messages.push_back("|TInterface\\icons\\Ability_Druid_Maul.png:25:25:0:-1|t Druid");
    messages.push_back("DK");
    messages.push_back("(Usage: .bot lookup CLASS)");
    std::string ret_msg;
    for (std::string msg : messages)
    {
        ret_msg += msg + "\n";
    }
    return ret_msg;
}

uint32 PlayerbotHolder::GetPlayerbotsCountByClass(uint32 cls)
{
    uint32 count = 0;
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot && bot->IsInWorld() && bot->getClass() == cls)
        {
            count++;
        }
    }
    return count;
}

PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(), master(master), lastErrorTell(0) {}

PlayerbotMgr::~PlayerbotMgr()
{
    if (master)
        PlayerbotsMgr::instance().RemovePlayerBotData(master->GetGUID(), false);
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, std::string const text)
{
    Player* master = GetMaster();
    if (!master)
        return;

    if (text.find(sPlayerbotAIConfig.commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, text, sPlayerbotAIConfig.commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i);
        }

        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleCommand(type, text, master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin();
         it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == master)
            botAI->HandleCommand(type, text, master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin();
         it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            break;
        }
        // if master cancelled logout, cancel too
        case CMSG_LOGOUT_CANCEL:
        {
            CancelLogout();
            break;
        }
    }
}

void PlayerbotMgr::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin();
         it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB(false, false);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin();
         it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (GET_PLAYERBOT_AI(bot) && GET_PLAYERBOT_AI(bot)->GetMaster() == GetMaster())
            bot->SaveToDB(false, false);
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player* const bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return;
    }
    botAI->SetMaster(master);
    botAI->ResetStrategies();

    LOG_INFO("playerbots", "Bot {} logged in", bot->GetName().c_str());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    if (!player)
        return;

    WorldSession* session = player->GetSession();
    if (!session)
    {
        LOG_WARN("playerbots", "Unable to register locale priority for player {} because the session is missing", player->GetName());
        return;
    }

    // DB locale (source of bot text translation)
    LocaleConstant const databaseLocale = session->GetSessionDbLocaleIndex();

    // For bot texts (DB-driven), prefer the database locale with a safe fallback.
    LocaleConstant usedLocale = databaseLocale;
    if (usedLocale >= MAX_LOCALES)
        usedLocale = LOCALE_enUS; // fallback

    // set locale priority for bot texts
    PlayerbotTextMgr::instance().AddLocalePriority(usedLocale);

    if (sPlayerbotAIConfig.selfBotLevel > 2)
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig.botAutologin)
        return;

    uint32 accountId = session->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId);
    if (results)
    {
        std::ostringstream out;
        out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();

            if (first)
                first = false;
            else
                out << ",";

            out << fields[0].Get<std::string>();
        } while (results->NextRow());

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(std::string const botName, std::string const text)
{
    std::set<std::string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }

    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(nullptr);
    if ((now - lastErrorTell) < sPlayerbotAIConfig.errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        std::string const text = i->first;
        std::set<std::string> names = i->second;

        std::ostringstream out;
        bool first = true;
        for (std::set<std::string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first)
                out << ", ";
            else
                first = false;

            out << *j;
        }

        out << "|cfff00000: " << text;

        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    errors.clear();
}

void PlayerbotsMgr::AddPlayerbotData(Player* player, bool isBotAI)
{
    if (!player)
    {
        return;
    }
    // If the guid already exists in the map, remove it

    if (!isBotAI)
    {
        std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMgrMap.find(player->GetGUID());
        if (itr != _playerbotsMgrMap.end())
        {
            _playerbotsMgrMap.erase(itr);
        }
        PlayerbotMgr* playerbotMgr = new PlayerbotMgr(player);
        ASSERT(_playerbotsMgrMap.emplace(player->GetGUID(), playerbotMgr).second);

        playerbotMgr->OnPlayerLogin(player);
    }
    else
    {
        std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsAIMap.find(player->GetGUID());
        if (itr != _playerbotsAIMap.end())
        {
            _playerbotsAIMap.erase(itr);
        }
        PlayerbotAI* botAI = new PlayerbotAI(player);
        ASSERT(_playerbotsAIMap.emplace(player->GetGUID(), botAI).second);
    }
}

void PlayerbotsMgr::RemovePlayerBotData(ObjectGuid const& guid, bool is_AI)
{
    if (is_AI)
    {
        std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsAIMap.find(guid);
        if (itr != _playerbotsAIMap.end())
        {
            _playerbotsAIMap.erase(itr);
        }
    }
    else
    {
        std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMgrMap.find(guid);
        if (itr != _playerbotsMgrMap.end())
        {
            _playerbotsMgrMap.erase(itr);
        }
    }
}

PlayerbotAI* PlayerbotsMgr::GetPlayerbotAI(Player* player)
{
    if (!(sPlayerbotAIConfig.enabled) || !player)
    {
        return nullptr;
    }
    // if (player->GetSession()->isLogingOut() || player->IsDuringRemoveFromWorld())
    // {
    //     return nullptr;
    // }
    auto itr = _playerbotsAIMap.find(player->GetGUID());
    if (itr != _playerbotsAIMap.end())
    {
        if (itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotAI*>(itr->second);
    }

    return nullptr;
}

PlayerbotMgr* PlayerbotsMgr::GetPlayerbotMgr(Player* player)
{
    if (!(sPlayerbotAIConfig.enabled) || !player)
    {
        return nullptr;
    }
    auto itr = _playerbotsMgrMap.find(player->GetGUID());
    if (itr != _playerbotsMgrMap.end())
    {
        if (!itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotMgr*>(itr->second);
    }

    return nullptr;
}

void PlayerbotMgr::HandleViewLinkedAccountsCommand(Player* player)
{
    ChatHandler handler(player->GetSession());
    uint32 accountId = player->GetSession()->GetAccountId();

    try {
        // Get confirmed links with shortNames
        QueryResult linkedResult = PlayerbotsDatabase.Query(
            "SELECT short_name, UNIX_TIMESTAMP(created_at) FROM playerbots_account_links WHERE account_id = {} ORDER BY created_at DESC",
            accountId);

        if (!linkedResult)
        {
            handler.PSendSysMessage("No active account links.");
            return;
        }

        // Display confirmed links using shortNames
        handler.PSendSysMessage("=== Active Account Links ===");
        do {
            Field* fields = linkedResult->Fetch();
            std::string shortName = fields[0].Get<std::string>();
            uint32 linkTimestamp = fields[1].Get<uint32>();

            // Convert timestamp to readable date
            time_t rawTime = static_cast<time_t>(linkTimestamp);
            struct tm* timeInfo = std::localtime(&rawTime);
            char dateBuffer[64];
            std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);

            handler.PSendSysMessage("{} (linked: {})", shortName.c_str(), dateBuffer);
        } while (linkedResult->NextRow());
    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "viewing linked accounts");
    }
}

void PlayerbotMgr::HandleUnlinkAccountCommand(Player* player, const std::string& shortName)
{
    ChatHandler handler(player->GetSession());
    uint32 accountId = player->GetSession()->GetAccountId();

    if (shortName.empty())
    {
        handler.PSendSysMessage("Usage: .playerbots accountlink disconnect <shortName>");
        handler.PSendSysMessage("Example: disconnect \"MyFriend\"");
        return;
    }

    // Remove quotes if present
    std::string cleanShortName = shortName;
    if (cleanShortName.size() >= 2 && cleanShortName.front() == '"' && cleanShortName.back() == '"')
    {
        cleanShortName = cleanShortName.substr(1, cleanShortName.size() - 2);
    }

    try {
        // Find the linked account by shortName
        QueryResult linkCheck = PlayerbotsDatabase.Query(
            "SELECT linked_account_id FROM playerbots_account_links WHERE account_id = {} AND short_name = '{}'",
            accountId, cleanShortName);

        if (!linkCheck)
        {
            handler.PSendSysMessage("No link exists with short name '{}'.", cleanShortName.c_str());
            handler.PSendSysMessage("Use 'activelinks' to see your current links.");
            return;
        }

        uint32 targetAccountId = linkCheck->Fetch()[0].Get<uint32>();

        // Remove bidirectional links
        PlayerbotsDatabase.Execute(
            "DELETE FROM playerbots_account_links WHERE (account_id = {} AND linked_account_id = {}) OR (account_id = {} AND linked_account_id = {})",
            accountId, targetAccountId, targetAccountId, accountId);

        handler.PSendSysMessage("Account link '{}' disconnected successfully.", cleanShortName.c_str());
    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "account unlinking");
    }
}

void PlayerbotMgr::HandleGenerateInviteCommand(Player* player)
{
    ChatHandler handler(player->GetSession());
    uint32 accountId = player->GetSession()->GetAccountId();

    try
    {
        // Check how many active invite codes this account has
        QueryResult activeCodesResult = PlayerbotsDatabase.Query(
            "SELECT COUNT(*) FROM playerbots_invite_codes WHERE account_id = {} AND expires_at > {} AND status = 1",
            accountId, GetCurrentTimestamp());

        if (activeCodesResult)
        {
            uint32 activeCount = activeCodesResult->Fetch()[0].Get<uint32>();
            if (activeCount >= MAX_INVITE_CODES_PER_ACCOUNT)
            {
                handler.PSendSysMessage("You have reached the maximum number of active invite codes ({}). Revoke some codes first.", MAX_INVITE_CODES_PER_ACCOUNT);
                return;
            }
        }

        // Generate unique invite code with collision detection
        std::string inviteCode;
        constexpr int maxAttempts = 10;
        int attempts = 0;

        do {
            inviteCode = GenerateInviteCode();
            attempts++;
        } while (InviteCodeExists(inviteCode) && attempts < maxAttempts);

        if (attempts >= maxAttempts)
        {
            LOG_ERROR("playerbots", "Failed to generate unique invite code after {} attempts", maxAttempts);
            handler.PSendSysMessage("Failed to generate unique invite code after {} attempts. Please try again.", maxAttempts);
            return;
        }

        // Store invite code in database
        uint64 expiresAt = GetCurrentTimestamp() + (INVITE_CODE_EXPIRY_MINUTES * 60);
        PlayerbotsDatabase.Execute(
            "INSERT INTO playerbots_invite_codes (account_id, code, created_at, expires_at, status) VALUES ({}, '{}', {}, {}, 1)",
            accountId, inviteCode, GetCurrentTimestamp(), expiresAt);

        handler.PSendSysMessage("|cFF32CD32[Invite Code Generated]|r");
        handler.PSendSysMessage("Code: |cFFFFFF00{}|r", inviteCode.c_str());
        handler.PSendSysMessage("Valid for: {} minutes", INVITE_CODE_EXPIRY_MINUTES);
        handler.PSendSysMessage("Share this code with trusted players to allow them to link to your account.");
        handler.PSendSysMessage("They can use: |cFFFFFFFF.playerbots accountlink connect {} \"FriendlyName\"|r", inviteCode.c_str());
    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "invite code generation");
    }
}

void PlayerbotMgr::HandleLinkWithInviteCommand(Player* player, const std::string& args)
{
    ChatHandler handler(player->GetSession());
    uint32 requestingAccountId = player->GetSession()->GetAccountId();

    // Parse arguments: <inviteCode> <shortName>
    std::istringstream iss(args);
    std::string inviteCode, shortName;
    iss >> inviteCode;

    // Get remaining args as shortName (handles quoted names)
    std::getline(iss, shortName);
    if (!shortName.empty() && shortName[0] == ' ')
    {
        shortName = shortName.substr(1); // Remove leading space
    }

    // Remove quotes if present
    if (shortName.size() >= 2 && shortName.front() == '"' && shortName.back() == '"')
    {
        shortName = shortName.substr(1, shortName.size() - 2);
    }

    if (inviteCode.empty() || shortName.empty())
    {
        handler.PSendSysMessage("Usage: connect <inviteCode> <shortName>");
        handler.PSendSysMessage("Example: connect AB3X-9K2L-PQ8M \"MyFriend\"");
        return;
    }

    // Validate shortName
    if (shortName.length() > 32)
    {
        handler.PSendSysMessage("Short name must be 32 characters or less.");
        return;
    }

    // Validate invite code format
    if (!IsValidInviteCodeFormat(inviteCode))
    {
        handler.PSendSysMessage("Invalid invite code format.");
        return;
    }

    try {
        // Check if invite code exists and is valid
        QueryResult inviteResult = PlayerbotsDatabase.Query(
            "SELECT account_id, created_at, expires_at FROM playerbots_invite_codes WHERE code = '{}' AND status = 1 AND expires_at > {}",
            inviteCode, GetCurrentTimestamp());

        if (!inviteResult)
        {
            handler.PSendSysMessage("Invalid or expired invite code.");
            return;
        }

        Field* inviteFields = inviteResult->Fetch();
        uint32 targetAccountId = inviteFields[0].Get<uint32>();

        // Prevent self-linking
        if (requestingAccountId == targetAccountId)
        {
            handler.PSendSysMessage("You cannot link to your own account.");
            return;
        }

        // Check if accounts are already linked
        QueryResult existingLink = PlayerbotsDatabase.Query(
            "SELECT 1 FROM playerbots_account_links WHERE account_id = {} AND linked_account_id = {}",
            requestingAccountId, targetAccountId);
        if (existingLink)
        {
            handler.PSendSysMessage("Accounts are already linked.");
            return;
        }

        // Create immediate bidirectional account link with same shortName for both sides
        // Insert both directions of the link using the same shortName
        PlayerbotsDatabase.Execute(
            "INSERT INTO playerbots_account_links (account_id, linked_account_id, short_name) VALUES ({}, {}, '{}')",
            requestingAccountId, targetAccountId, shortName);

        PlayerbotsDatabase.Execute(
            "INSERT INTO playerbots_account_links (account_id, linked_account_id, short_name) VALUES ({}, {}, '{}')",
            targetAccountId, requestingAccountId, shortName);
            handler.PSendSysMessage("Accounts linked successfully as '{}'! You can now manage each other's player bots.", shortName.c_str());
    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "invite code linking");
    }
}

void PlayerbotMgr::HandleViewInviteCodesCommand(Player* player)
{
    ChatHandler handler(player->GetSession());
    uint32 accountId = player->GetSession()->GetAccountId();

    try {
        QueryResult activeCodesResult = PlayerbotsDatabase.Query(
            "SELECT code, created_at, expires_at FROM playerbots_invite_codes "
            "WHERE account_id = {} AND status = 1 AND expires_at > {} "
            "ORDER BY created_at DESC",
            accountId, GetCurrentTimestamp());

        if (!activeCodesResult)
        {
            handler.PSendSysMessage("No active invite codes found. Use '.playerbots accountlink generate' to create one.");
            return;
        }

        handler.PSendSysMessage("=== Active Invite Codes ===");
        do {
            Field* fields = activeCodesResult->Fetch();
            std::string code = fields[0].Get<std::string>();
            uint64 expiry = fields[2].Get<uint64>();

            uint64 minutesLeft = (expiry - GetCurrentTimestamp()) / 60;
            handler.PSendSysMessage("- |cFFFFFF00{}|r (expires in {} minutes)", code.c_str(), minutesLeft);
        } while (activeCodesResult->NextRow());

    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "viewing invite codes");
    }
}

void PlayerbotMgr::HandleRevokeInviteCommand(Player* player, const std::string& inviteCode)
{
    ChatHandler handler(player->GetSession());
    uint32 accountId = player->GetSession()->GetAccountId();

    if (inviteCode.empty())
    {
        handler.PSendSysMessage("Usage: .playerbots accountlink remove <inviteCode>");
        return;
    }

    try {
        // Check if the code belongs to this account and is active
        QueryResult codeResult = PlayerbotsDatabase.Query(
            "SELECT 1 FROM playerbots_invite_codes WHERE account_id = {} AND code = '{}' AND status = 1",
            accountId, inviteCode);

        if (!codeResult)
        {
            handler.PSendSysMessage("Invite code not found or already used/revoked.");
            return;
        }

        // Revoke the code
        PlayerbotsDatabase.Execute(
            "UPDATE playerbots_invite_codes SET status = 0 WHERE account_id = {} AND code = '{}'",
            accountId, inviteCode);

        handler.PSendSysMessage("Invite code |cFFFFFF00{}|r has been revoked.", inviteCode.c_str());
    }
    catch (const std::exception&)
    {
        SendGenericError(handler, "invite code revocation");
    }
}

bool PlayerbotMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Account link commands:");
        handler->PSendSysMessage("  Invite System: generate, connect, codes, remove");
        handler->PSendSysMessage("  Account Management: activelinks, disconnect");
        return false;
    }

    std::string command;
    std::istringstream iss(args);
    iss >> command;

    std::string remainingArgs;
    std::getline(iss, remainingArgs);
    if (!remainingArgs.empty() && remainingArgs[0] == ' ')
    {
        remainingArgs = remainingArgs.substr(1); // Remove leading space
    }

    WorldSession* session = handler->GetSession();
    if (!session)
    {
        handler->PSendSysMessage("Console commands not supported for this operation.");
        return false;
    }

    Player* player = session->GetPlayer();
    if (!player)
    {
        handler->PSendSysMessage("Player not found.");
        return false;
    }

    PlayerbotMgr* mgr = sPlayerbotsMgr->GetPlayerbotMgr(player);
    if (!mgr)
    {
        handler->PSendSysMessage("PlayerbotMgr instance not found.");
        return false;
    }

    try {
        // Invite code system commands
        if (command == "generate")
        {
            mgr->HandleGenerateInviteCommand(player);
            return true;
        }
        else if (command == "connect")
        {
            if (remainingArgs.empty())
            {
                handler->PSendSysMessage("Usage: connect <inviteCode> <shortName>");
                handler->PSendSysMessage("Example: connect AB3X-9K2L-PQ8M \"MyFriend\"");
                return false;
            }
            mgr->HandleLinkWithInviteCommand(player, remainingArgs);
            return true;
        }
        else if (command == "codes")
        {
            mgr->HandleViewInviteCodesCommand(player);
            return true;
        }
        else if (command == "remove")
        {
            if (remainingArgs.empty())
            {
                handler->PSendSysMessage("Usage: remove <inviteCode>");
                return false;
            }
            mgr->HandleRevokeInviteCommand(player, remainingArgs);
            return true;
        }
        // Account management commands
        else if (command == "activelinks")
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }
        else if (command == "disconnect")
        {
            if (remainingArgs.empty())
            {
                handler->PSendSysMessage("Usage: disconnect <shortName>");
                handler->PSendSysMessage("Example: disconnect \"MyFriend\"");
                return false;
            }
            mgr->HandleUnlinkAccountCommand(player, remainingArgs);
            return true;
        }
        else
        {
            handler->PSendSysMessage("Unknown command '{}'. Available commands:", command.c_str());
            handler->PSendSysMessage("  Invite System: generate, connect, codes, remove");
            handler->PSendSysMessage("  Account Management: activelinks, disconnect");
            return false;
        }
    }
    catch (const std::exception&)
    {
        handler->PSendSysMessage("An error occurred processing the command.");
        return false;
    }
}

void PlayerbotMgr::CleanupExpiredInviteCodes()
{
    PlayerbotsDatabase.Execute("DELETE FROM playerbots_invite_codes WHERE expires_at < {}", GetCurrentTimestamp());
    LOG_DEBUG("mod-playerbots", "Cleaned up expired invite codes on server startup");
}
