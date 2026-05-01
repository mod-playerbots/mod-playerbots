/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellEmblemsAction.h"

#include <array>

#include "Event.h"
#include "Playerbots.h"

bool TellEmblemsAction::Execute(Event /*event*/)
{
    static std::array<uint32, 6> const emblemIds = {
        29434, // Badge of Justice
        40752, // Emblem of Heroism
        40753, // Emblem of Valor
        45624, // Emblem of Conquest
        47241, // Emblem of Triumph
        49426  // Emblem of Frost
    };

    botAI->TellMaster("=== Emblems ===");

    for (uint32 itemId : emblemIds)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        uint32 count = bot->GetItemCount(itemId, false);
        std::ostringstream out;
        out << chat->FormatItem(proto, count);
        botAI->TellMaster(out);
    }

    return true;
}
