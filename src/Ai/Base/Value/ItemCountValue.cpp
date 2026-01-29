/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ItemCountValue.h"

uint32_t ItemCountValue::Calculate()
{
    uint32_t count = 0;
    std::vector<Item*> items = InventoryAction::parseItems(qualifier);

    for (const Item* const item : items)
    {
        count += item->GetCount();
    }

    return count;
}

std::vector<Item*> InventoryItemValue::Calculate()
{
    return InventoryAction::parseItems(qualifier);
}
