/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotItemService.h"

#include "PlayerbotAI.h"

Item* BotItemService::FindPoison() const
{
    if (botAI_)
    {
        return botAI_->FindPoison();
    }
    return nullptr;
}

Item* BotItemService::FindAmmo() const
{
    if (botAI_)
    {
        return botAI_->FindAmmo();
    }
    return nullptr;
}

Item* BotItemService::FindBandage() const
{
    if (botAI_)
    {
        return botAI_->FindBandage();
    }
    return nullptr;
}

Item* BotItemService::FindOpenableItem() const
{
    if (botAI_)
    {
        return botAI_->FindOpenableItem();
    }
    return nullptr;
}

Item* BotItemService::FindLockedItem() const
{
    if (botAI_)
    {
        return botAI_->FindLockedItem();
    }
    return nullptr;
}

Item* BotItemService::FindConsumable(uint32 itemId) const
{
    if (botAI_)
    {
        return botAI_->FindConsumable(itemId);
    }
    return nullptr;
}

Item* BotItemService::FindStoneFor(Item* weapon) const
{
    if (botAI_)
    {
        return botAI_->FindStoneFor(weapon);
    }
    return nullptr;
}

Item* BotItemService::FindOilFor(Item* weapon) const
{
    if (botAI_)
    {
        return botAI_->FindOilFor(weapon);
    }
    return nullptr;
}

void BotItemService::ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID)
{
    if (botAI_)
    {
        botAI_->ImbueItem(item, targetFlag, targetGUID);
    }
}

void BotItemService::ImbueItem(Item* item, uint8 targetInventorySlot)
{
    if (botAI_)
    {
        botAI_->ImbueItem(item, targetInventorySlot);
    }
}

void BotItemService::ImbueItem(Item* item, Unit* target)
{
    if (botAI_)
    {
        botAI_->ImbueItem(item, target);
    }
}

void BotItemService::ImbueItem(Item* item)
{
    if (botAI_)
    {
        botAI_->ImbueItem(item);
    }
}

void BotItemService::EnchantItem(uint32 spellId, uint8 slot)
{
    if (botAI_)
    {
        botAI_->EnchantItemT(spellId, slot);
    }
}

std::vector<Item*> BotItemService::GetInventoryAndEquippedItems() const
{
    if (botAI_)
    {
        return botAI_->GetInventoryAndEquippedItems();
    }
    return {};
}

std::vector<Item*> BotItemService::GetInventoryItems() const
{
    if (botAI_)
    {
        return botAI_->GetInventoryItems();
    }
    return {};
}

uint32 BotItemService::GetInventoryItemsCountWithId(uint32 itemId) const
{
    if (botAI_)
    {
        return botAI_->GetInventoryItemsCountWithId(itemId);
    }
    return 0;
}

bool BotItemService::HasItemInInventory(uint32 itemId) const
{
    if (botAI_)
    {
        return botAI_->HasItemInInventory(itemId);
    }
    return false;
}

InventoryResult BotItemService::CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool notLoading) const
{
    if (botAI_)
    {
        return botAI_->CanEquipItem(slot, dest, pItem, swap, notLoading);
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 BotItemService::FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const
{
    if (botAI_)
    {
        return botAI_->FindEquipSlot(proto, slot, swap);
    }
    return 0;
}

uint32 BotItemService::GetEquipGearScore() const
{
    if (botAI_)
    {
        return botAI_->GetEquipGearScore(botAI_->GetBot());
    }
    return 0;
}

std::vector<std::pair<Quest const*, uint32>> BotItemService::GetCurrentQuestsRequiringItemId(uint32 itemId) const
{
    if (botAI_)
    {
        return botAI_->GetCurrentQuestsRequiringItemId(itemId);
    }
    return {};
}
