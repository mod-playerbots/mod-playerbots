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
#include "MapMgr.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "TravelNode.h"

using namespace Acore::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
            {"zone", HandleDebugZoneCommand, SEC_GAMEMASTER, Console::No},
        };

        static ChatCommandTable playerbotsAccountCommandTable = {
            {"setKey", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"link", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"linkedAccounts", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"unlink", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsTravelCommandTable = {
            {"generatenode", HandleGenerateTravelNodesCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"travel", playerbotsTravelCommandTable},
            {"debug", playerbotsDebugCommandTable},
            {"account", playerbotsAccountCommandTable},
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

    static bool HandlePerfMonCommand(ChatHandler* /*handler*/, char const* args)
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

    static bool HandleGenerateTravelNodesCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!sPlayerbotAIConfig.enableTravelNodes)
        {
            handler->PSendSysMessage("Travel node generation refused: AiPlayerbot.EnableTravelNodes is disabled, so the node store was never loaded and generating now would overwrite it.");
            return true;
        }

        handler->PSendSysMessage("Regenerating travel node paths (all maps)... The world thread is blocked until this finishes.");
        LOG_INFO("playerbots", "Manual travel node regeneration started via console command.");
        sTravelNodeMap.generateAll();
        handler->PSendSysMessage("Travel node regeneration complete. Paths saved to database.");
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    // Visual constants for showpath markers. Two waypoint-family
    // creatures give nodes vs path waypoints distinct visuals; both
    // render at their creature_template default scale (no override).
    //   nodes (anchors) → 15897, prominent waypoint variant
    //   path waypoints  → 15631, standard BG-showpath waypoint
    //
    // SHOWPATH_PATH_DISPLAY_ID = 0 uses the path-creature's default
    // model. To experiment with a model override, set this to a known-
    // good creature display ID for your DB (spell-visual IDs are not
    // universally registered as creature displays — using one risks
    // summoning invisible markers).
    static constexpr uint32 SHOWPATH_NODE_CREATURE = 15897;
    static constexpr uint32 SHOWPATH_PATH_CREATURE = 15631;
    static constexpr uint32 SHOWPATH_PATH_DISPLAY_ID = 0;       // 0 = default model
    static constexpr uint32 SHOWPATH_DESPAWN_MS = 60000;

    static bool HandleDebugZoneCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
        {
            handler->PSendSysMessage("Command requires an in-game player.");
            return false;
        }

        if (!args || !*args)
        {
            handler->PSendSysMessage("usage: .playerbots debug zone showpath=all|node|path");
            return false;
        }

        char* cmd = strtok(const_cast<char*>(args), " ");
        // showpath=all  → nodes + cached path waypoints (full picture)
        // showpath=node → only node anchors
        // showpath=path → only cached path waypoints (no anchors)
        bool showNodes = false;
        bool showLinks = false;
        if (cmd && strcmp(cmd, "showpath=all") == 0)
        {
            showNodes = true;
            showLinks = true;
        }
        else if (cmd && strcmp(cmd, "showpath=node") == 0)
        {
            showNodes = true;
            showLinks = false;
        }
        else if (cmd && strcmp(cmd, "showpath=path") == 0)
        {
            showNodes = false;
            showLinks = true;
        }
        else
        {
            handler->PSendSysMessage("usage: .playerbots debug zone showpath=all|node|path");
            return false;
        }

        uint32 zoneId = player->GetZoneId();
        uint32 const phaseMask = player->GetPhaseMask();
        uint32 const mapId = player->GetMapId();
        std::vector<TravelNode*> nodes;
        for (TravelNode* n : sTravelNodeMap.getNodes())
        {
            if (!n)
                continue;
            WorldPosition* pos = n->getPosition();
            if (!pos || pos->GetMapId() != mapId)
                continue;
            uint32 const nodeZone = sMapMgr->GetZoneId(phaseMask, mapId,
                                                       pos->GetPositionX(),
                                                       pos->GetPositionY(),
                                                       pos->GetPositionZ());
            if (nodeZone != zoneId)
                continue;
            nodes.push_back(n);
        }
        if (nodes.empty())
        {
            handler->PSendSysMessage("No travel nodes registered in zone {} (is the travel node system loaded?)", zoneId);
            return true;
        }

        // node markers — full-scale anchor at each travel-node position.
        uint32 nodesPlaced = 0;
        if (showNodes)
        {
            for (TravelNode* node : nodes)
            {
                if (!node)
                    continue;
                WorldPosition* pos = node->getPosition();
                if (!pos || pos->GetMapId() != player->GetMapId())
                    continue;
                Creature* wp = player->SummonCreature(SHOWPATH_NODE_CREATURE,
                                                      pos->GetPositionX(), pos->GetPositionY(),
                                                      pos->GetPositionZ(), 0,
                                                      TEMPSUMMON_TIMED_DESPAWN, SHOWPATH_DESPAWN_MS);
                if (wp)
                {
                    wp->SetOwnerGUID(player->GetGUID());
                    ++nodesPlaced;
                }
            }
        }

        if (!showLinks)
        {
            handler->PSendSysMessage("Showing {} travel nodes in zone {} (60s)", nodesPlaced, zoneId);
            return true;
        }

        // path-waypoint markers — same creature, scaled down so they
        // read as a breadcrumb trail between nodes rather than as more
        // anchor points. Walk-type links from any in-zone node are
        // drawn; the per-waypoint same-map filter keeps the trail from
        // running into other continents. Sparse zones (e.g. Teldrassil)
        // would draw nothing if we required dst-in-zone too, since their
        // only links go to nodes in neighbouring zones.
        constexpr uint32 MAX_PATH_MARKERS = 500;
        uint32 pathPlaced = 0;
        uint32 linksDrawn = 0;
        bool capped = false;
        for (TravelNode* node : nodes)
        {
            if (!node)
                continue;
            auto* links = node->getLinks();
            if (!links)
                continue;
            for (auto const& kv : *links)
            {
                TravelNode* dst = kv.first;
                TravelNodePath* path = kv.second;
                if (!dst || !path)
                    continue;
                if (path->getPathType() != TravelNodePathType::walk)
                    continue;
                ++linksDrawn;
                for (WorldPosition const& wpPos : path->GetPath())
                {
                    if (wpPos.GetMapId() != player->GetMapId())
                        continue;
                    if (pathPlaced >= MAX_PATH_MARKERS)
                    {
                        capped = true;
                        break;
                    }
                    Creature* mk = player->SummonCreature(SHOWPATH_PATH_CREATURE,
                                                          wpPos.GetPositionX(),
                                                          wpPos.GetPositionY(), wpPos.GetPositionZ(),
                                                          0, TEMPSUMMON_TIMED_DESPAWN,
                                                          SHOWPATH_DESPAWN_MS);
                    if (mk)
                    {
                        mk->SetOwnerGUID(player->GetGUID());
                        if (SHOWPATH_PATH_DISPLAY_ID)
                            mk->SetDisplayId(SHOWPATH_PATH_DISPLAY_ID);
                        ++pathPlaced;
                    }
                }
                if (capped)
                    break;
            }
            if (capped)
                break;
        }

        handler->PSendSysMessage("Showing {} nodes + {} path waypoints across {} walk links in zone {}{} (60s)",
                                 nodesPlaced, pathPlaced, linksDrawn, zoneId,
                                 capped ? " — capped at 500 path markers" : "");
        return true;
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("Usage: .playerbots account setKey <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string key = args;

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleSetSecurityKeyCommand(player, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");

        if (!accountName || !key)
        {
            handler->PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleLinkAccountCommand(player, accountName, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        if (!accountName)
        {
            handler->PSendSysMessage("Usage: .playerbots account unlink <accountName>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleUnlinkAccountCommand(player, accountName);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }
};

void AddPlayerbotsCommandscripts() { new playerbots_commandscript(); }
