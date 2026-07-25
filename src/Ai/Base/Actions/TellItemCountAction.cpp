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
    std::map<uint32, uint32> itemMap;
    std::map<uint32, bool> soulbound;

    for (Item* item : found)
    {
        ItemTemplate const* proto = item->GetTemplate();
        itemMap[proto->ItemId] += item->GetCount();
        soulbound[proto->ItemId] = item->IsSoulBound();
    }

    botAI->TellMaster("=== Inventory ===");
    for (std::map<uint32, uint32>::iterator i = itemMap.begin(); i != itemMap.end(); ++i)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->first);
        TellItem(proto, i->second, soulbound[i->first]);
    }

    return true;
}
