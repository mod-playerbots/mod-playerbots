/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellPvpAction.h"

#include <map>

#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "Event.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace
{
    inline char const* BracketName(uint8 slot)
    {
        switch (slot)
        {
            case 0: return "2v2"; // 2v2
            case 1: return "3v3"; // 3v3
            case 2: return "5v5"; // 5v5
            default: return "?v?";
        }
    }
}

bool TellPvpAction::Execute(Event /*event*/)
{
    Player* const master = GetMaster();
    if (!master || !bot)
        return false;

    // PVP currencies
    std::map<std::string, std::string> currencyPlaceholders;
    currencyPlaceholders["%arena_points"] = std::to_string(bot->GetArenaPoints());
    currencyPlaceholders["%honor_points"] = std::to_string(bot->GetHonorPoints());

    std::string const currencyText = sPlayerbotTextMgr->GetBotTextOrDefault(
        "pvp_currency",
        "[PVP] Arena points: %arena_points | Honor Points: %honor_points",
        currencyPlaceholders);

    botAI->TellMaster(currencyText);

    // Arena Teams by slot
    bool anyTeam = false;
    for (uint8 slot = 0; slot < MAX_ARENA_SLOT; ++slot)
    {
        uint32 const teamId = bot->GetArenaTeamId(slot);
        if (!teamId)
            continue;

        if (ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(teamId))
        {
            anyTeam = true;
            std::map<std::string, std::string> placeholders;
            placeholders["%bracket"] = BracketName(slot);
            placeholders["%team_name"] = team->GetName();
            placeholders["%team_rating"] = std::to_string(team->GetRating());

            std::string const teamText = sPlayerbotTextMgr->GetBotTextOrDefault(
                "pvp_arena_team",
                "[PVP] %bracket: <%team_name> (rating %team_rating)",
                placeholders);

            botAI->TellMaster(teamText);
        }
    }

    if (!anyTeam)
    {
        std::string const noTeamText = sPlayerbotTextMgr->GetBotTextOrDefault(
            "pvp_no_arena_team",
            "[PVP] I have no Arena Team.",
            std::map<std::string, std::string>());

        botAI->TellMaster(noTeamText);
    }

    return true;
}
