/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IITEM_SERVICE_H
#define _PLAYERBOT_IITEM_SERVICE_H

#include "Common.h"
#include <vector>
#include <utility>

class Item;
class Player;
class Unit;
class ObjectGuid;
class Quest;
struct ItemTemplate;

enum InventoryResult : uint8;

/**
 * @brief Interface for inventory and item management
 *
 * This interface abstracts item-related operations from PlayerbotAI,
 * enabling isolated testing of item logic.
 */
class IItemService
{
public:
    virtual ~IItemService() = default;

    // Item finding
    virtual Item* FindPoison() const = 0;
    virtual Item* FindAmmo() const = 0;
    virtual Item* FindBandage() const = 0;
    virtual Item* FindOpenableItem() const = 0;
    virtual Item* FindLockedItem() const = 0;
    virtual Item* FindConsumable(uint32 itemId) const = 0;

    // Weapon enhancements
    virtual Item* FindStoneFor(Item* weapon) const = 0;
    virtual Item* FindOilFor(Item* weapon) const = 0;

    // Item use
    virtual void ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID) = 0;
    virtual void ImbueItem(Item* item, uint8 targetInventorySlot) = 0;
    virtual void ImbueItem(Item* item, Unit* target) = 0;
    virtual void ImbueItem(Item* item) = 0;

    // Enchanting
    virtual void EnchantItem(uint32 spellId, uint8 slot) = 0;

    // Inventory queries
    virtual std::vector<Item*> GetInventoryAndEquippedItems() const = 0;
    virtual std::vector<Item*> GetInventoryItems() const = 0;
    virtual uint32 GetInventoryItemsCountWithId(uint32 itemId) const = 0;
    virtual bool HasItemInInventory(uint32 itemId) const = 0;

    // Equipment
    virtual InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap,
                                         bool notLoading = true) const = 0;
    virtual uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const = 0;
    virtual uint32 GetEquipGearScore(Player* player = nullptr) const = 0;

    // Quest items
    virtual std::vector<std::pair<Quest const*, uint32>> GetCurrentQuestsRequiringItemId(uint32 itemId) const = 0;
};

#endif
