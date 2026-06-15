/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UnlockItemAction.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"

inline constexpr uint32_t PICK_LOCK_SPELL_ID = 1804;

bool UnlockItemAction::Execute(Event /*event*/)
{
    bool foundLockedItem = false;

    Item* item = botAI->FindLockedItem();
    if (item)
    {
        UnlockItem(item);
        foundLockedItem = true;
    }

    return foundLockedItem;
}

void UnlockItemAction::UnlockItem(Item* item)
{
    // Use CastSpell to unlock the item
    if (botAI->CastSpell(PICK_LOCK_SPELL_ID, bot, item))
    {
        std::ostringstream out;
        out << "Used Pick Lock on: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
        botAI->TellError("Failed to cast Pick Lock.");
}
