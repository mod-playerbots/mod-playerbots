/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellPvpAction.h"

#include <sstream>

#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "Event.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace
{
    inline char const* BracketName(uint8 slot)
    {
        // Slots order in AzerothCore: 2v2, 3v3, 5v5
        switch (slot)
        {
            case 0: return "2v2";
            case 1: return "3v3";
            case 2: return "5v5";
        }
        return "?v?";
    }
}

bool TellPvpAction::Execute(Event /*event*/)
{
    Player* const master = GetMaster();
    Player* const bot = botAI->GetBot();
    if (!master || !bot)
        return false;

    // Currencies
    {
        std::ostringstream line;
        line << "[PVP] Arena points: " << bot->GetArenaPoints()
             << " | Honor Points: " << bot->GetHonorPoints();
        botAI->TellMaster(line.str());
    }

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
            std::ostringstream line;
            line << "[PVP] " << BracketName(slot) << ": <" << team->GetName() << "> (rating " << team->GetRating() << ")";
            botAI->TellMaster(line.str());
        }
    }

    if (!anyTeam)
    {
        botAI->TellMaster("[PVP] I have no Arena Team.");
    }

    return true;
}