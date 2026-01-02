/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellReputationAction.h"

#include <array>
#include <algorithm>
#include <utility>
#include <vector>

#include "Event.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "SharedDefines.h"

namespace
{
    void AppendRankText(std::ostringstream& out, ReputationRank rank)
    {
        switch (rank)
        {
            case REP_HATED:
                out << "cc2222hated";
                break;
            case REP_HOSTILE:
                out << "ff0000hostile";
                break;
            case REP_UNFRIENDLY:
                out << "ee6622unfriendly";
                break;
            case REP_NEUTRAL:
                out << "ffff00neutral";
                break;
            case REP_FRIENDLY:
                out << "00ff00friendly";
                break;
            case REP_HONORED:
                out << "00ff88honored";
                break;
            case REP_REVERED:
                out << "00ffccrevered";
                break;
            case REP_EXALTED:
                out << "00ffffexalted";
                break;
            default:
                out << "808080unknown";
                break;
        }
    }

    std::string BuildReputationLine(ReputationMgr& repMgr, FactionEntry const* entry)
    {
        ReputationRank rank = repMgr.GetRank(entry);
        int32 reputation = repMgr.GetReputation(entry->ID);

        std::ostringstream out;
        out << entry->name[0] << ": |cff";
        AppendRankText(out, rank);
        out << "|cffffffff";

        int32 base = ReputationMgr::Reputation_Cap + 1;
        for (int32 i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
            base -= ReputationMgr::PointsInRank[i];

        out << " (" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << ")";
        return out.str();
    }
}

bool TellReputationAction::Execute(Event event)
{
    std::string const param = event.getParam();
    if (param == "all")
    {
        ReputationMgr& repMgr = bot->GetReputationMgr();

        std::vector<std::pair<std::string, std::string>> lines;

        lines.reserve(64);

        static std::array<uint32, 41> const neutralFactions = {
            // Wrath of the Lich King
            1104, 1105, 1119, 1073, 1090, 1098, 1106, 1091, 1156,
            // Burning Crusade
            942, 1038, 1015, 933, 970, 1011, 935, 1077, 1031, 934, 932, 989, 967, 1012, 990,
            // Classic
            529, 609, 21, 470, 369, 577, 87, 909, 59, 910, 749, 349, 70, 576, 589, 92, 93
        };

        static std::array<uint32, 20> const hordeFactions = {
            // Wrath of the Lich King
            1052, 1085, 1064, 1124, 1067,
            // Burning Crusade
            947, 941, 922,
            // Classic
            76, 81, 530, 68, 911, 1133, 1352, 2523, 889, 510, 729, 2372
        };

        static std::array<uint32, 19> const allianceFactions = {
            // Wrath of the Lich King
            1037, 1068, 1126, 1094, 1050,
            // Burning Crusade
            946, 978,
            // Classic
            72, 47, 69, 54, 930, 1134, 1353, 2524, 890, 509, 730, 2371
        };

        for (uint32 factionId : neutralFactions)
        {
            if (FactionEntry const* entry = sFactionStore.LookupEntry(factionId))
                lines.emplace_back(entry->name[0], BuildReputationLine(repMgr, entry));
        }

        if (bot->GetTeamId() == TEAM_HORDE)
        {
            for (uint32 factionId : hordeFactions)
            {
                if (FactionEntry const* entry = sFactionStore.LookupEntry(factionId))
                    lines.emplace_back(entry->name[0], BuildReputationLine(repMgr, entry));
            }
        }
        else
        {
            for (uint32 factionId : allianceFactions)
            {
                if (FactionEntry const* entry = sFactionStore.LookupEntry(factionId))
                    lines.emplace_back(entry->name[0], BuildReputationLine(repMgr, entry));
            }
        }

        std::sort(lines.begin(), lines.end(),
            [](auto const& left, auto const& right) { return left.first < right.first; });

        botAI->TellMaster("=== Reputations ===");
        for (auto const& line : lines)
            botAI->TellMaster(line.second);

        return true;
    }

    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid selection = master->GetTarget();
    if (selection.IsEmpty())
        return false;

    Unit* unit = ObjectAccessor::GetUnit(*master, selection);
    if (!unit)
        return false;

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();
    FactionEntry const* entry = sFactionStore.LookupEntry(factionTemplate->faction);
    if (!entry)
        return false;

    ReputationMgr& repMgr = bot->GetReputationMgr();
    botAI->TellMaster(BuildReputationLine(repMgr, entry));

    return true;
}
