/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_ITEM_SERVICE_H
#define _PLAYERBOT_BOT_ITEM_SERVICE_H

#include "Bot/Interface/IItemService.h"

class PlayerbotAI;

/**
 * @brief Implementation of IItemService
 *
 * This service provides inventory and item management for bots,
 * extracting this functionality from PlayerbotAI for better testability.
 *
 * The service delegates to PlayerbotAI methods during the transition period.
 */
class BotItemService : public IItemService
{
public:
    explicit BotItemService(PlayerbotAI* ai) : botAI_(ai) {}
    ~BotItemService() override = default;

    // Item finding
    Item* FindPoison() const override;
    Item* FindAmmo() const override;
    Item* FindBandage() const override;
    Item* FindOpenableItem() const override;
    Item* FindLockedItem() const override;
    Item* FindConsumable(uint32 itemId) const override;

    // Weapon enhancements
    Item* FindStoneFor(Item* weapon) const override;
    Item* FindOilFor(Item* weapon) const override;

    // Item use
    void ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID) override;
    void ImbueItem(Item* item, uint8 targetInventorySlot) override;
    void ImbueItem(Item* item, Unit* target) override;
    void ImbueItem(Item* item) override;

    // Enchanting
    void EnchantItem(uint32 spellId, uint8 slot) override;

    // Inventory queries
    std::vector<Item*> GetInventoryAndEquippedItems() const override;
    std::vector<Item*> GetInventoryItems() const override;
    uint32 GetInventoryItemsCountWithId(uint32 itemId) const override;
    bool HasItemInInventory(uint32 itemId) const override;

    // Equipment
    InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap,
                                 bool notLoading = true) const override;
    uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const override;
    uint32 GetEquipGearScore(Player* player = nullptr) const override;

    // Quest items
    std::vector<std::pair<Quest const*, uint32>> GetCurrentQuestsRequiringItemId(uint32 itemId) const override;

private:
    PlayerbotAI* botAI_;
};

#endif
