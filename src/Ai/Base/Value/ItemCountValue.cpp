/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ItemCountValue.h"

#include "PlayerbotAI.h"

std::vector<Item*> InventoryItemValueBase::Find(std::string const qualifier)
{
    std::vector<Item*> result;

    std::vector<Item*> items = InventoryAction::parseItems(qualifier);
    for (Item* item : items)
        result.push_back(item);

    return result;
}

uint32 ItemCountValue::Calculate()
{
    uint32 count = 0;
    std::vector<Item*> items = Find(qualifier);
    for (Item* item : items)
        count += item->GetCount();

    return count;
}

std::vector<Item*> InventoryItemValue::Calculate() { return Find(qualifier); }
