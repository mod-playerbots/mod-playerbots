/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TellItemCountAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

bool TellItemCountAction::Execute(Event event)
{
    std::string const text = event.getParam();
    std::vector<Item*> found = parseItems(text);

    botAI->TellMaster("=== Inventory ===");
    TellItems(found);

    return true;
}
