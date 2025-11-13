/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Playerbots.h"

#include "BattleGroundTactics.h"
#include "Channel.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "GuildTaskMgr.h"
#include "Metric.h"
#include "PlayerScript.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "cmath"
#include "cs_playerbots.h"

///**********************************************************************
/// PlayerbotsDatabaseScript
///**********************************************************************

class PlayerbotsDatabaseScript : public DatabaseScript
{
public:
    /**
     * @brief Registers the playerbots-specific database (PlayerbotsDatabase).
     *
     * Thread context:
     *  - Called during server startup from the main thread (world context),
     *    before maps, scripts, and world are fully initialized.
     *
     * Responsibility:
     *  - Set up a DatabaseLoader for the "server.playerbots" connection.
     *  - Configure which DBs to update based on config.
     *  - Attach the PlayerbotsDatabase handle and run the loader.
     */
    PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") {}

    /**
     * @brief Invoked when databases are being initialized.
     *
     * Internal calls:
     *  - DatabaseLoader playerbotLoader("server.playerbots"):
     *      Creates a loader tied to the "server.playerbots" connection string.
     *  - SetUpdateFlags(...):
     *      If "Playerbots.Updates.EnableDatabases" is true, enables
     *      DATABASE_PLAYERBOTS updates for this DB; otherwise, no schema updates.
     *  - AddDatabase(PlayerbotsDatabase, "Playerbots"):
     *      Registers the PlayerbotsDatabase handle with the loader.
     *  - playerbotLoader.Load():
     *      Actually connects and applies updates if necessary.
     */
    bool OnDatabasesLoading() override
    {
        DatabaseLoader playerbotLoader("server.playerbots");

        // Decide whether to apply DB updates for the playerbots schema
        playerbotLoader.SetUpdateFlags(sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true)
                                           ? DatabaseLoader::DATABASE_PLAYERBOTS
                                           : 0);

        // Attach the PlayerbotsDatabase handle (global DB connection)
        playerbotLoader.AddDatabase(PlayerbotsDatabase, "Playerbots");

        // Run DB initialization & updates
        return playerbotLoader.Load();
    }

    /**
     * @brief Periodic keep-alive for PlayerbotsDatabase.
     *
     * Thread context:
     *  - Runs from DB keep-alive thread / world context depending on core,
     *    but is limited to DB pinging, which is thread-safe.
     *
     * Responsibility:
     *  - Ensures the playerbots DB connection is kept alive and not dropped.
     */
    void OnDatabasesKeepAlive() override
    {
        PlayerbotsDatabase.KeepAlive();
    }

    /**
     * @brief Cleanly closes the PlayerbotsDatabase connection on shutdown.
     *
     * Thread context:
     *  - Called during shutdown in world/main thread.
     */
    void OnDatabasesClosing() override
    {
        PlayerbotsDatabase.Close();
    }

    /**
     * @brief Controls whether to warn about sync queries on PlayerbotsDatabase.
     *
     * Thread context:
     *  - Called from world/main thread when config changes or at startup.
     *
     * @param apply Whether warnings about synchronous queries should be enabled.
     */
    void OnDatabaseWarnAboutSyncQueries(bool apply) override
    {
        PlayerbotsDatabase.WarnAboutSyncQueries(apply);
    }

    /**
     * @brief Provides statement index/param when a character logs out.
     *
     * Thread context:
     *  - World thread, when a player logs out and DB updates are prepared.
     *
     * Responsibility:
     *  - Tell the DB system which prepared statement and parameter to use
     *    to mark the character offline in the playerbots DB.
     *
     * Internal:
     *  - CHAR_UPD_CHAR_OFFLINE: ID of the statement that updates offline flag.
     *  - player->GetGUID().GetCounter(): low part of the character GUID.
     */
    void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam) override
    {
        statementIndex = CHAR_UPD_CHAR_OFFLINE;
        statementParam = player->GetGUID().GetCounter();
    }

    /**
     * @brief Reads the latest playerbots DB revision string.
     *
     * Thread context:
     *  - World thread at startup when DB revisions are reported.
     *
     * Responsibility:
     *  - Query the playerbots DB for the most recent `version_db_playerbots`
     *    entry and fill the `revision` string used in logs.
     */
    void OnDatabaseGetDBRevision(std::string& revision) override
    {
        // Query the latest revision date from the playerbots DB
        if (QueryResult resultPlayerbot =
                PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
        {
            Field* fields = resultPlayerbot->Fetch();
            revision = fields[0].Get<std::string>();
        }

        // If nothing found, fall back to a default label
        if (revision.empty())
        {
            revision = "Unknown Playerbots Database Revision";
        }
    }
};

///**********************************************************************
/// PlayerbotsPlayerScript – hooks tied to individual Player objects
///**********************************************************************

class PlayerbotsPlayerScript : public PlayerScript
{
public:
    /**
     * @brief Registers which player hooks this script responds to.
     *
     * Hooks:
     *  - ON_LOGIN: world thread, when a player logs in
     *  - ON_AFTER_UPDATE: map thread, after each player update tick
     *  - ON_CHAT / WITH_CHANNEL / WITH_GROUP: world thread, when chat is handled
     *  - ON_BEFORE_CRITERIA_PROGRESS: usually map thread (criteria update)
     *  - ON_BEFORE_ACHI_COMPLETE: world thread (achievement finalization)
     *  - CAN_PLAYER_USE_PRIVATE_CHAT: world thread (whisper permission)
     *  - ON_GIVE_EXP: map thread (xp grant logic)
     *  - ON_BEFORE_TELEPORT: context of teleport (usually map thread for bots)
     */
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_AFTER_UPDATE, PLAYERHOOK_ON_CHAT,
        PLAYERHOOK_ON_CHAT_WITH_CHANNEL,
        PLAYERHOOK_ON_CHAT_WITH_GROUP,
        PLAYERHOOK_ON_BEFORE_CRITERIA_PROGRESS,
        PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_BEFORE_TELEPORT
    }) {}

    /**
     * @brief Called when a (real) player logs in.
     *
     * Thread context:
     *  - World thread (during login flow / WorldSession handling).
     *
     * Responsibility:
     *  - Prepare playerbot-related data structures for real players.
     *  - Notify RandomPlayerbotMgr about the login (so random bots can
     *    react to this player, e.g., joining them, whispering, etc.).
     *  - Optionally send informational system messages about mod-playerbots.
     *
     * Notes:
     *  - Skips bots entirely (`IsBot()` check) to avoid redundant setup.
     */
    void OnPlayerLogin(Player* player) override
    {
        // Real players only: bots themselves do not run this initialization.
        if (!player->GetSession()->IsBot())
        {
            // Register this real player in the playerbots manager subsystem.
            // This allows the player to own and control bots (PlayerbotMgr).
            sPlayerbotsMgr->AddPlayerbotData(player, false);

            // Inform RandomPlayerbotMgr that a real player has logged in.
            // It may spawn random bots near them or adjust behavior.
            sRandomPlayerbotMgr->OnPlayerLogin(player);

            // Inform the player about mod-playerbots if enabled in config.
            if (sPlayerbotAIConfig->enabled)
            {
                ChatHandler(player->GetSession())
                    .SendSysMessage(
                        "|cff00ff00This server runs with |cff00ccffmod-playerbots|r "
                        "|cffcccccchttps://github.com/mod-playerbots/mod-playerbots|r");
            }

            // If bots are enabled or random bots autologin is on, also
            // show a rough estimate of startup bot initialization time.
            if (sPlayerbotAIConfig->enabled || sPlayerbotAIConfig->randomBotAutologin)
            {
                // Estimate: maxRandomBots * 0.11 seconds / 60 = minutes, then round
                std::string roundedTime =
                    std::to_string(std::ceil((sPlayerbotAIConfig->maxRandomBots * 0.11 / 60) * 10) / 10.0);
                roundedTime = roundedTime.substr(0, roundedTime.find('.') + 2);

                ChatHandler(player->GetSession())
                    .SendSysMessage("|cff00ff00Playerbots:|r bot initialization at server startup takes about '" +
                                    roundedTime + "' minutes.");
            }
        }
    }

    /**
     * @brief Called before a player teleports to another location/map.
     *
     * Thread context:
     *  - Called in the context invoking the teleport (often map thread).
     *    For bots, this is typically from map update or spell handling.
     *
     * Responsibility:
     *  - For bots only, proactively clear visibility references when
     *    changing maps to avoid dangling object references in other bots’
     *    visibility containers.
     *
     * Why:
     *  - Fixes a race condition where:
     *      1. Bot A teleports and its visible objects start getting removed.
     *      2. Bot B is updating visibility and accesses Bot A's old map view.
     *      3. Those objects may already be freed → crash.
     *
     * Return:
     *  - true  → allow teleport to proceed.
     *  - false → cancel teleport (we always allow here).
     */
    bool OnPlayerBeforeTeleport(Player* player, uint32 mapid, float /*x*/, float /*y*/, float /*z*/,
                                float /*orientation*/, uint32 /*options*/, Unit* /*target*/) override
    {
        // Only apply to bots; we do not want to touch real players here.
        if (!player || !player->GetSession()->IsBot())
            return true;

        // If the bot is actually changing maps and still in-world, clean
        // its visibility container before the teleport completes.
        if (player->GetMapId() != mapid && player->IsInWorld())
        {
            // This removes references to objects currently visible to this bot,
            // preventing other bots from iterating over stale objects.
            player->GetObjectVisibilityContainer().CleanVisibilityReferences();
        }

        return true;  // Always allow the teleport to continue.
    }

    /**
     * @brief Called after each player update tick.
     *
     * Thread context:
     *  - Map thread; this is called from Player::Update() inside Map::Update().
     *
     * Responsibility:
     *  - Drive per-tick logic for:
     *      - PlayerbotAI (bot brain) if the player is a bot.
     *      - PlayerbotMgr (bot manager) if the player is a real master.
     *
     * Notes:
     *  - Both AI and manager must avoid direct world-thread-only operations
     *    (group/guild/AH/etc.) and instead queue packets or commands.
     */
    void OnPlayerAfterUpdate(Player* player, uint32 diff) override
    {
        // If this player has a PlayerbotAI attached (i.e., it is a bot),
        // execute its AI update (combat, movement, strategy).
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->UpdateAI(diff);
        }

        // If this is a real player with a PlayerbotMgr, update bot management:
        // command dispatch, owned bots, formations, etc.
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->UpdateAI(diff);
        }
    }

    /**
     * @brief Determines whether a player can use private chat (whispers).
     *
     * Thread context:
     *  - World thread; whisper handling is done in WorldSession handlers.
     *
     * Responsibility:
     *  - Intercept whispers sent to bots and redirect them as commands
     *    to the target bot’s AI.
     *
     * Behavior:
     *  - If the receiver is a bot, call botAI->HandleCommand(...)
     *    and return false to prevent the whisper from going through
     *    the normal chat processing pipeline.
     */
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
    {
        // We only intercept whispers to bots.
        if (type == CHAT_MSG_WHISPER)
        {
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(receiver))
            {
                // Interpret the whisper message as a bot command.
                // 'player' is the sender (real player).
                botAI->HandleCommand(type, msg, player);

                // Returning false means "do not send the actual whisper packet";
                // we treat it purely as a control channel.
                return false;
            }
        }

        // For all non-bot targets or non-whisper types, allow normal chat.
        return true;
    }

    /**
     * @brief Called when a player sends chat to a group (party/raid).
     *
     * Thread context:
     *  - World thread; group chat processed in session/chat handlers.
     *
     * Responsibility:
     *  - Forward group chat messages as commands to any bots in the same group.
     *
     * Behavior:
     *  - Loops through group members; for each member that is a bot,
     *    calls botAI->HandleCommand(type, msg, sender).
     */
    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
    {
        // Iterate over all group members.
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* member = itr->GetSource())
            {
                // If a member is a bot, forward the message to its AI as a command.
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
                {
                    botAI->HandleCommand(type, msg, player);
                }
            }
        }
    }

    /**
     * @brief Called when a player sends generic chat (no group/channel) – e.g. guild.
     *
     * Thread context:
     *  - World thread.
     *
     * Responsibility:
     *  - For guild chat specifically, forward messages to any bots owned
     *    by this player that are in the same guild.
     *
     * Internal:
     *  - GET_PLAYERBOT_MGR(player): returns the PlayerbotMgr for this real player.
     *  - PlayerbotMgr::GetPlayerBotsBegin/End(): iterates over bots owned by this player.
     */
    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
    {
        // Only react to guild chat here.
        if (type == CHAT_MSG_GUILD)
        {
            // Only real players with bots will have a manager.
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                // Iterate over all bots owned by this player.
                for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin();
                     it != playerbotMgr->GetPlayerBotsEnd(); ++it)
                {
                    if (Player* const bot = it->second)
                    {
                        // Only forward if bot is in the same guild.
                        if (bot->GetGuildId() == player->GetGuildId())
                        {
                            // Treat guild chat as a command feed for these bots.
                            GET_PLAYERBOT_AI(bot)->HandleCommand(type, msg, player);
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Called when a player sends chat in a channel (e.g. world, custom).
     *
     * Thread context:
     *  - World thread.
     *
     * Responsibility:
     *  - For certain flagged channels (0x18), forward messages to the
     *    PlayerbotMgr of the speaker.
     *  - Independently, forward public commands to RandomPlayerbotMgr so
     *    random bots can react.
     *
     * Internal:
     *  - channel->GetFlags() & 0x18:
     *      Used by mod-playerbots to check if this channel is a "bot control"
     *      or watch channel (e.g. world, custom LFG, etc.).
     *  - playerbotMgr->HandleCommand(type, msg):
     *      Distributes this message as a command for the player's own bots.
     *  - sRandomPlayerbotMgr->HandleCommand(type, msg, player):
     *      Allows non-owned "random bots" to also react to commands/messages.
     */
    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
    {
        // First, let a player's own bots react via their manager.
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            // 0x18 flag check: channel is eligible for bot command parsing.
            if (channel->GetFlags() & 0x18)
            {
                playerbotMgr->HandleCommand(type, msg);
            }
        }

        // Also allow global random bots to parse this chat and possibly respond.
        sRandomPlayerbotMgr->HandleCommand(type, msg, player);
    }

    /**
     * @brief Called just before an achievement is marked complete.
     *
     * Thread context:
     *  - World thread; achievement completion is processed globally.
     *
     * Responsibility:
     *  - Prevent random/addclass bots from earning "realm first" achievements.
     *
     * Behavior:
     *  - If the player is a random bot or addclass bot AND the achievement
     *    has a realm-first flag, return false to veto completion.
     */
    bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement) override
    {
        // For random/addclass bots only, block realm-first achievements.
        if ((sRandomPlayerbotMgr->IsRandomBot(player) || sRandomPlayerbotMgr->IsAddclassBot(player)) &&
            (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL)))
        {
            return false;  // Prevent completion.
        }

        return true;
    }

    /**
     * @brief Adjusts XP awarded to players when XP is given.
     *
     * Thread context:
     *  - Map thread; XP is awarded as part of kill/quest logic on the map.
     *
     * Responsibility:
     *  - Apply an XP multiplier for random bots when:
     *      - randomBotXPRate != 1.0
     *      - the player is a bot and a random bot
     *      - the bot is NOT grouped with any real player.
     *
     * Internal:
     *  - sPlayerbotAIConfig->randomBotXPRate:
     *      Multiplier value from config.
     *  - sRandomPlayerbotMgr->IsRandomBot(player):
     *      Checks if this is a randomly spawned bot.
     */
    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        // If multiplier is 1.0 or player is null, there is nothing to adjust.
        if (sPlayerbotAIConfig->randomBotXPRate == 1.0 || !player)
            return;

        // Only modify XP for bots that are random bots; real players are untouched.
        if (!player->GetSession()->IsBot() || !sRandomPlayerbotMgr->IsRandomBot(player))
            return;

        // If bot is in a group with any real player, do NOT boost XP (avoid abuse).
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member)
                    continue;

                // Presence of a real player cancels XP multiplier.
                if (!member->GetSession()->IsBot())
                {
                    return;
                }
            }
        }

        // Otherwise, apply configured random bot XP multiplier.
        amount = static_cast<uint32>(std::round(static_cast<float>(amount) * sPlayerbotAIConfig->randomBotXPRate));
    }
};

///**********************************************************************
/// PlayerbotsMiscScript – lifecycle cleanup for Playerbots objects
///**********************************************************************

class PlayerbotsMiscScript : public MiscScript
{
public:
    /**
     * @brief Registers interest in player destruction events.
     *
     * Hook:
     *  - MISCHOOK_ON_DESTRUCT_PLAYER: called when Player object is destroyed.
     */
    PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript", {MISCHOOK_ON_DESTRUCT_PLAYER}) {}

    /**
     * @brief Clean up PlayerbotAI and PlayerbotMgr when a Player is destroyed.
     *
     * Thread context:
     *  - World or map teardown context; Player is going away permanently.
     *
     * Responsibility:
     *  - Avoid memory leaks by deleting attached AI/manager objects.
     *
     * Notes:
     *  - These pointers are stored externally (singleton maps), so we must
     *    explicitly delete them when Player is destroyed.
     */
    void OnDestructPlayer(Player* player) override
    {
        // Destroy the bot brain if attached (bot Player).
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            delete botAI;
        }

        // Destroy the manager if this Player had bots attached (real Player).
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            delete playerbotMgr;
        }
    }
};

///**********************************************************************
/// PlayerbotsServerScript – handles raw packets for masters
///**********************************************************************

class PlayerbotsServerScript : public ServerScript
{
public:
    /**
     * @brief Registers interest in packet reception events.
     *
     * Hook:
     *  - SERVERHOOK_CAN_PACKET_RECEIVE: called when a packet arrives from a client.
     */
    PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript", {SERVERHOOK_CAN_PACKET_RECEIVE}) {}

    /**
     * @brief Called when a packet is received from a client (before handling).
     *
     * Thread context:
     *  - World thread; packets are processed in WorldSession::Update.
     *
     * Responsibility:
     *  - Allow PlayerbotMgr to inspect and react to packets received by the
     *    real player (master), e.g. to coordinate bots with the master’s actions.
     *
     * Internal:
     *  - GET_PLAYERBOT_MGR(player):
     *      Returns manager for real players that control bots.
     *  - playerbotMgr->HandleMasterIncomingPacket(packet):
     *      Manager can use this to mirror or interpret master actions (movement,
     *      spells, interactions) and adjust bots accordingly.
     */
    void OnPacketReceived(WorldSession* session, WorldPacket const& packet) override
    {
        if (Player* player = session->GetPlayer())
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->HandleMasterIncomingPacket(packet);
    }
};

///**********************************************************************
/// PlayerbotsWorldScript – global world-level hooks
///**********************************************************************

class PlayerbotsWorldScript : public WorldScript
{
public:
    /**
     * @brief Registers this WorldScript to run before world initialization.
     *
     * Hook:
     *  - WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED: called once at startup.
     */
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED
    }) {}

    /**
     * @brief Initializes playerbots configuration and prints banner at startup.
     *
     * Thread context:
     *  - World/main thread during startup before maps are loaded.
     *
     * Responsibility:
     *  - Display the mod-playerbots banner and license notice.
     *  - Initialize PlayerbotAIConfig (reads playerbots.conf and caches options).
     */
    void OnBeforeWorldInitialized() override
    {
        // Decorative banner in server logs (AGPL notice & project URL).
        LOG_INFO("server.loading", "╔══════════════════════════════════════════════════════════╗");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "║              AzerothCore Playerbots Module               ║");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     mod-playerbots is a community-driven open-source     ║");
        LOG_INFO("server.loading", "║  project based on AzerothCore, licensed under AGPLv3.0   ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║      https://github.com/mod-playerbots/mod-playerbots    ║");
        LOG_INFO("server.loading", "╚══════════════════════════════════════════════════════════╝");

        uint32 oldMSTime = getMSTime();

        LOG_INFO("server.loading", " ");
        LOG_INFO("server.loading", "Load Playerbots Config...");

        // Read and parse playerbots config (playerbots.conf),
        // initializing sPlayerbotAIConfig with all settings.
        sPlayerbotAIConfig->Initialize();

        LOG_INFO("server.loading", ">> Loaded playerbots config in {} ms", GetMSTimeDiffToNow(oldMSTime));
        LOG_INFO("server.loading", " ");
    }
};

///**********************************************************************
/// PlayerbotsScript – central PlayerbotScript hooks (bot-specific)
///**********************************************************************

class PlayerbotsScript : public PlayerbotScript
{
public:
    /**
     * @brief Registers PlayerbotScript hooks for various bot-related checks.
     *
     * PlayerbotScript is a custom script type used only by mod-playerbots.
     */
    PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") {}

    /**
     * @brief Checks if an LFG queue entry contains any non-bot (real) entities.
     *
     * Thread context:
     *  - LFG system context; typically world thread.
     *
     * Responsibility:
     *  - When the LFG/LFR system evaluates a queue entry, determine if it
     *    contains a real player or a group (non-bot). If yes, treat it differently.
     *
     * Behavior:
     *  - Iterates over all GUIDs in the LFG 5-player group queue structure.
     *  - If any GUID is a group or refers to a Player without PlayerbotAI
     *    (i.e. real player), sets `nonBotFound = true`.
     *  - Return true if any non-bot is present.
     */
    bool OnPlayerbotCheckLFGQueue(lfg::Lfg5Guids const& guidsList) override
    {
        bool nonBotFound = false;
        for (ObjectGuid const& guid : guidsList.guids)
        {
            Player* player = ObjectAccessor::FindPlayer(guid);
            if (guid.IsGroup() || (player && !GET_PLAYERBOT_AI(player)))
            {
                nonBotFound = true;
                break;
            }
        }

        return nonBotFound;
    }

    /**
     * @brief Called when a player/bot kills a unit, to check guild kill tasks.
     *
     * Thread context:
     *  - Map or world, depending on task system hooks; logically part of kill handling.
     *
     * Responsibility:
     *  - Forward kills done by bots or players controlled by the bot system
     *    into the guild task system for progression (e.g., guild quest tasks).
     */
    void OnPlayerbotCheckKillTask(Player* player, Unit* victim) override
    {
        if (player)
            sGuildTaskMgr->CheckKillTask(player, victim);
    }

    /**
     * @brief Filters petition accounts to exclude bots.
     *
     * Thread context:
     *  - World thread where petition handling occurs.
     *
     * Responsibility:
     *  - If the system has found a petition account and the player is a bot,
     *    clear the 'found' flag so bots are not treated as petition account owners.
     */
    void OnPlayerbotCheckPetitionAccount(Player* player, bool& found) override
    {
        if (found && GET_PLAYERBOT_AI(player))
            found = false;
    }

    /**
     * @brief Controls whether to send normal update packets to a player.
     *
     * Thread context:
     *  - World thread during update-building / packet sending.
     *
     * Responsibility:
     *  - For bots that are acting as "real players" (e.g., mirrored via proxy),
     *    allow normal updates.
     *  - For pure bots, may opt to skip some update sending to save bandwidth.
     *
     * Behavior:
     *  - If the player has PlayerbotAI:
     *      - return botAI->IsRealPlayer()
     *        (true if this bot represents a real client; false otherwise).
     *  - If no AI, return true (normal player).
     */
    bool OnPlayerbotCheckUpdatesToSend(Player* player) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            return botAI->IsRealPlayer();

        return true;
    }

    /**
     * @brief Called whenever a WorldPacket is sent to a player.
     *
     * Thread context:
     *  - World thread; executed in send pipeline (before/after actual send).
     *
     * Responsibility:
     *  - Allow bot AI and manager to inspect and handle outgoing packets
     *    for bots and masters, instead of actually sending them over a socket.
     *
     * Internal:
     *  - botAI->HandleBotOutgoingPacket(*packet):
     *      Lets the bot's AI parse/update its internal state from server data.
     *  - playerbotMgr->HandleMasterOutgoingPacket(*packet):
     *      Lets the manager mirror or react to packets sent to the real player.
     */
    void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet) override
    {
        if (!player)
            return;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->HandleBotOutgoingPacket(*packet);
        }
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->HandleMasterOutgoingPacket(*packet);
        }
    }

    /**
     * @brief Global per-tick update for random bots and their sessions.
     *
     * Thread context:
     *  - World thread; called each server tick via PlayerbotScript hooks.
     *
     * Responsibility:
     *  - Run RandomPlayerbotMgr AI updates.
     *  - Run RandomPlayerbotMgr session updates (e.g., handling queued packets,
     *    login/logout cycles) for random bots.
     */
    void OnPlayerbotUpdate(uint32 diff) override
    {
        // Update random bot AI logic (spawn/despawn, roaming, etc.).
        sRandomPlayerbotMgr->UpdateAI(diff);

        // Update sessions for random bots (world-thread session responsibilities).
        sRandomPlayerbotMgr->UpdateSessions();
    }

    /**
     * @brief Per-session world-thread update for master players’ managers.
     *
     * Thread context:
     *  - World thread; called once per active player session each tick.
     *
     * Responsibility:
     *  - Allow PlayerbotMgr (for real players) to perform session-bound work
     *    that must run on the world thread (e.g. sending packets, processing
     *    deferred world-thread commands).
     */
    void OnPlayerbotUpdateSessions(Player* player) override
    {
        if (player)
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->UpdateSessions();
    }

    /**
     * @brief Called when a real player logs out; coordinates bot logout.
     *
     * Thread context:
     *  - World thread; logout handling is global.
     *
     * Responsibility:
     *  - If the player is a real master (not a bot masquerading as real),
     *    instruct their PlayerbotMgr to logout all owned bots.
     *  - Notify RandomPlayerbotMgr that this player has logged out.
     *
     * Behavior:
     *  - GET_PLAYERBOT_MGR(player): manager for this session (if any).
     *  - GET_PLAYERBOT_AI(player)->IsRealPlayer():
     *      Distinguishes between a “true” player and a proxy/forwarded bot.
     */
    void OnPlayerbotLogout(Player* player) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || botAI->IsRealPlayer())
            {
                // Logout all bots owned by this real player.
                playerbotMgr->LogoutAllBots();
            }
        }

        // Also inform the random bot system that this player is gone, so
        // it can update any behavior that depended on their presence.
        sRandomPlayerbotMgr->OnPlayerLogout(player);
    }

    /**
     * @brief Global request to logout all bots on the server.
     *
     * Thread context:
     *  - World thread; typically invoked on shutdown or admin command.
     *
     * Responsibility:
     *  - Log a message, then instruct RandomPlayerbotMgr to logout all
     *    managed bots cleanly.
     */
    void OnPlayerbotLogoutBots() override
    {
        LOG_INFO("playerbots", "Logging out all bots...");
        sRandomPlayerbotMgr->LogoutAllBots();
    }
};

///**********************************************************************
/// PlayerBotsBGScript – battleground strategy data for bots
///**********************************************************************

class PlayerBotsBGScript : public BGScript
{
public:
    /**
     * @brief Registers BGScript for battleground-related hooks.
     *
     * BGScript hooks are used by mod-playerbots to drive BG tactics.
     */
    PlayerBotsBGScript() : BGScript("PlayerBotsBGScript") {}

    /**
     * @brief Called when a battleground instance starts.
     *
     * Thread context:
     *  - World thread; battlegrounds are updated from a global manager.
     *
     * Responsibility:
     *  - Randomly select a strategy index for Alliance and Horde teams
     *    per BG instance, based on its type (WS, AB, AV, EY).
     *  - Store this selection in bgStrategies keyed by instance ID.
     *
     * Internal:
     *  - BGStrategyData:
     *      Holds allianceStrategy and hordeStrategy (enum indices).
     *  - urand(0, X_STRATEGY_MAX - 1):
     *      Picks a random strategy variant for that BG type.
     */
    void OnBattlegroundStart(Battleground* bg) override
    {
        BGStrategyData data;

        switch (bg->GetBgTypeID())
        {
            case BATTLEGROUND_WS:
                data.allianceStrategy = urand(0, WS_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, WS_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AB:
                data.allianceStrategy = urand(0, AB_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AB_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AV:
                data.allianceStrategy = urand(0, AV_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AV_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_EY:
                data.allianceStrategy = urand(0, EY_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, EY_STRATEGY_MAX - 1);
                break;
            default:
                break;
        }

        // Cache strategy selection for this specific instance.
        bgStrategies[bg->GetInstanceID()] = data;
    }

    /**
     * @brief Called when a battleground ends.
     *
     * Thread context:
     *  - World thread.
     *
     * Responsibility:
     *  - Remove stored strategy data for the ended BG instance.
     */
    void OnBattlegroundEnd(Battleground* bg, TeamId /*winnerTeam*/) override
    {
        bgStrategies.erase(bg->GetInstanceID());
    }
};

///**********************************************************************
/// Registration entry point
///**********************************************************************

/**
 * @brief Registers all Playerbots-related scripts with ScriptMgr.
 *
 * Thread context:
 *  - Called on startup when loading C++ scripts.
 *
 * Responsibility:
 *  - Instantiate each script so their constructors register them into
 *    the appropriate ScriptRegistry (DatabaseScript, PlayerScript, etc.).
 */
void AddPlayerbotsScripts()
{
    new PlayerbotsDatabaseScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();
    new PlayerBotsBGScript();

    AddSC_playerbots_commandscript();
}
