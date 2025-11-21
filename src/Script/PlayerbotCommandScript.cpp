/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BattleGroundTactics.h"
#include "Chat.h"
#include "GuildTaskMgr.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsAccountLinkCommandTable = {
            // Invite code system
            {"generate", HandleGenerateCommand, SEC_PLAYER, Console::No},
            {"connect", HandleConnectCommand, SEC_PLAYER, Console::No},
            {"codes", HandleCodesCommand, SEC_PLAYER, Console::No},
            {"remove", HandleRemoveCommand, SEC_PLAYER, Console::No},
            // Account management
            {"activelinks", HandleActiveLinksCommand, SEC_PLAYER, Console::No},
            {"disconnect", HandleDisconnectCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"debug", playerbotsDebugCommandTable},
            {"accountlink", playerbotsAccountLinkCommandTable},
        };

        static ChatCommandTable commandTable = {
            {"playerbots", playerbotsCommandTable},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* handler, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerfMonitor.Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerfMonitor.PrintStats(true, false);
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerfMonitor.PrintStats(false, true);
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
            if (sPlayerbotAIConfig.perfMonEnabled)
                LOG_INFO("playerbots", "Performance monitor enabled");
            else
                LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerfMonitor.PrintStats();
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    // Generic helper for account link commands
    // Forwards subcommands to PlayerbotMgr::HandleConsoleCommand with the command name prepended
    // Example: HandleGenerateCommand("mycode") -> PlayerbotMgr::HandleConsoleCommand("generate mycode")
    static bool HandleAccountLinkCommand(ChatHandler* handler, char const* args, const char* command)
    {
        std::string commandArgs = command;
        if (args && *args)
        {
            commandArgs += " ";
            commandArgs += args;
        }
        return PlayerbotMgr::HandleConsoleCommand(handler, commandArgs.c_str());
    }

    // Wrapper functions for account linking subcommands
    // Each function forwards to HandleAccountLinkCommand which delegates to PlayerbotMgr::HandleConsoleCommand
    // This pattern allows the command table to use simple function pointers while keeping implementation centralized

    static bool HandleGenerateCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "generate"); }           // Generate new invite code
    static bool HandleConnectCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "connect"); }             // Connect using invite code
    static bool HandleCodesCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "codes"); }                 // List available codes
    static bool HandleRemoveCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "remove"); }               // Remove invite code
    static bool HandleActiveLinksCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "activelinks"); }     // List active connections
    static bool HandleDisconnectCommand(ChatHandler* handler, char const* args) { return HandleAccountLinkCommand(handler, args, "disconnect"); }       // Disconnect linked account
};

void AddPlayerbotsCommandscripts() { new playerbots_commandscript(); }
