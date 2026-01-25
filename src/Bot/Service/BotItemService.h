/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_ITEM_SERVICE_H
#define _PLAYERBOT_BOT_ITEM_SERVICE_H

#include "Bot/Interface/IItemService.h"

#include <functional>

class PlayerbotAI;

/**
 * @brief Implementation of IItemService
 *
 * This service provides inventory and item management for bots.
 *
 * Static methods are provided for direct access without needing a service instance.
 * Instance methods implement the IItemService interface for testability/mockability.
 */
class BotItemService : public IItemService
{
public:
    BotItemService() = default;
    explicit BotItemService(PlayerbotAI* ai) : _botAI(ai) {}
    ~BotItemService() override = default;

    // ========================================================================
    // Static methods for direct access (main implementations)
    // These can be called without a service instance
    // ========================================================================

    // Core item finding helper
    static Item* FindItemInInventoryStatic(Player* bot, std::function<bool(ItemTemplate const*)> checkItem);

    // Specific item type finders
    static Item* FindPoisonStatic(Player* bot);
    static Item* FindAmmoStatic(Player* bot);
    static Item* FindBandageStatic(Player* bot);
    static Item* FindOpenableItemStatic(Player* bot);
    static Item* FindLockedItemStatic(Player* bot);
    static Item* FindConsumableStatic(Player* bot, uint32 itemId);

    // Weapon enhancement finders
    static Item* FindStoneForStatic(Player* bot, Item* weapon);
    static Item* FindOilForStatic(Player* bot, Item* weapon);

    // Item use
    static void ImbueItemStatic(Player* bot, Item* item, uint32 targetFlag, ObjectGuid targetGUID);
    static void ImbueItemStatic(Player* bot, Item* item, uint8 targetInventorySlot);
    static void ImbueItemStatic(Player* bot, Item* item, Unit* target);
    static void ImbueItemStatic(Player* bot, Item* item);

    // Enchanting
    static void EnchantItemStatic(Player* bot, uint32 spellId, uint8 slot);

    // Inventory queries
    static std::vector<Item*> GetInventoryAndEquippedItemsStatic(Player* bot);
    static std::vector<Item*> GetInventoryItemsStatic(Player* bot);
    static uint32 GetInventoryItemsCountWithIdStatic(Player* bot, uint32 itemId);
    static bool HasItemInInventoryStatic(Player* bot, uint32 itemId);

    // Equipment
    static uint32 GetEquipGearScoreStatic(Player* player);

    // Quest items
    static std::vector<std::pair<Quest const*, uint32>> GetCurrentQuestsRequiringItemIdStatic(Player* bot,
                                                                                               uint32 itemId);

    // ========================================================================
    // Instance methods (IItemService interface implementation)
    // These call the static methods internally, but provide mockable interface
    // ========================================================================

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

    // Set the bot context for instance methods that need the bot
    void SetBotContext(PlayerbotAI* ai) { _botAI = ai; }
    PlayerbotAI* GetBotContext() const { return _botAI; }

private:
    PlayerbotAI* _botAI = nullptr;
};

#endif
