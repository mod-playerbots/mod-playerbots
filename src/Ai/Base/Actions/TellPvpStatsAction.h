/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"

#include "Action.h"
#include "PlayerbotAI.h"

class TellPvpStatsAction : public Action
{
public:
    TellPvpStatsAction(PlayerbotAI* botAI) : Action(botAI, "tell pvp stats") {}

    bool Execute(Event event) override
    {
        if (this->bot == nullptr)
        {
            return false;
        }

        Player* requester = this->GetMaster();
        Unit* const eventOwner = event.getOwner();

        if (eventOwner != nullptr)
        {
            requester = eventOwner->ToPlayer();
        }

        if (requester == nullptr)
        {
            return false;
        }

        // PVP currencies
        std::map<std::string, std::string> currencyPlaceholders;

        currencyPlaceholders["%arena_points"] = std::to_string(this->bot->GetArenaPoints());
        currencyPlaceholders["%honor_points"] = std::to_string(this->bot->GetHonorPoints());

        const std::string currencyText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pvp_currency",
            "[PVP] Arena points: %arena_points | Honor Points: %honor_points",
            currencyPlaceholders);

        this->bot->Whisper(currencyText, LANG_UNIVERSAL, requester);

        // Arena Teams by slot
        bool hasArenaTeam = false;

        for (uint8_t slot = 0; slot < MAX_ARENA_SLOT; ++slot)
        {
            const uint32_t teamId = this->bot->GetArenaTeamId(slot);

            if (teamId == 0)
            {
                continue;
            }

            const ArenaTeam* const team = ArenaTeamMgr::instance()->GetArenaTeamById(teamId);

            if (team == nullptr)
            {
                continue;
            }

            hasArenaTeam = true;

            std::map<std::string, std::string> placeholders;

            placeholders["%bracket"] = this->MapBracketToString(slot);
            placeholders["%team_name"] = team->GetName();
            placeholders["%team_rating"] = std::to_string(team->GetRating());

            const std::string teamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pvp_arena_team",
                "[PVP] %bracket: <%team_name> (rating %team_rating)",
                placeholders
            );

            this->bot->Whisper(teamText, LANG_UNIVERSAL, requester);
        }

        if (!hasArenaTeam)
        {
            const std::string noTeamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pvp_no_arena_team",
                "[PVP] I have no Arena Team.",
                std::map<std::string, std::string>()
            );

            this->bot->Whisper(noTeamText, LANG_UNIVERSAL, requester);
        }

        return true;
    }

private:
    static std::string_view MapBracketToString(const uint8_t slot) noexcept
    {
        switch (slot)
        {
            case ARENA_SLOT_2v2:
                return "2v2";
            case ARENA_SLOT_3v3:
                return "3v3";
            case ARENA_SLOT_5v5:
                return "5v5";
            default:
                return "<unknown bracket>";
        }
    }
};
