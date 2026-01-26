/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotItemService.h"

#include "AiFactory.h"
#include "Bag.h"
#include "DBCStores.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "QuestDef.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "WorldPacket.h"
#include "WorldSession.h"

// Stone and oil item IDs are defined in PlayerbotAI.h

// ============================================================================
// Static method implementations (main logic)
// ============================================================================

Item* BotItemService::FindItemInInventoryStatic(Player* bot, std::function<bool(ItemTemplate const*)> checkItem)
{
    // List out items in the main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* pItemProto = pItem->GetTemplate();
            if (pItemProto && bot->CanUseItem(pItemProto) == EQUIP_ERR_OK && checkItem(pItemProto))
                return pItem;
        }
    }

    // List out items in other removable backpacks
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* pItemProto = pItem->GetTemplate();
                    if (pItemProto && bot->CanUseItem(pItemProto) == EQUIP_ERR_OK && checkItem(pItemProto))
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

Item* BotItemService::FindPoisonStatic(Player* bot)
{
    return FindItemInInventoryStatic(
        bot, [](ItemTemplate const* pItemProto) -> bool
        { return pItemProto->Class == ITEM_CLASS_CONSUMABLE && pItemProto->SubClass == 6; });
}

Item* BotItemService::FindAmmoStatic(Player* bot)
{
    // Get equipped ranged weapon
    if (Item* rangedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        uint32 weaponSubClass = rangedWeapon->GetTemplate()->SubClass;
        uint32 requiredAmmoType = 0;

        // Determine the correct ammo type based on the weapon
        switch (weaponSubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                requiredAmmoType = ITEM_SUBCLASS_BULLET;
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                requiredAmmoType = ITEM_SUBCLASS_ARROW;
                break;
            default:
                return nullptr;  // Not a ranged weapon that requires ammo
        }

        // Search inventory for the correct ammo type
        return FindItemInInventoryStatic(bot, [requiredAmmoType](ItemTemplate const* pItemProto) -> bool
                                         { return pItemProto->Class == ITEM_CLASS_PROJECTILE &&
                                                  pItemProto->SubClass == requiredAmmoType; });
    }

    return nullptr;
}

Item* BotItemService::FindBandageStatic(Player* bot)
{
    return FindItemInInventoryStatic(
        bot, [](ItemTemplate const* pItemProto) -> bool
        { return pItemProto->Class == ITEM_CLASS_CONSUMABLE && pItemProto->SubClass == ITEM_SUBCLASS_BANDAGE; });
}

Item* BotItemService::FindOpenableItemStatic(Player* bot)
{
    return FindItemInInventoryStatic(bot, [bot](ItemTemplate const* itemTemplate) -> bool
                                     {
                                         return itemTemplate->HasFlag(ITEM_FLAG_HAS_LOOT) &&
                                                (itemTemplate->LockID == 0 ||
                                                 !bot->GetItemByEntry(itemTemplate->ItemId)->IsLocked());
                                     });
}

Item* BotItemService::FindLockedItemStatic(Player* bot)
{
    return FindItemInInventoryStatic(bot, [bot](ItemTemplate const* itemTemplate) -> bool
                                     {
                                         if (!bot->HasSkill(SKILL_LOCKPICKING))
                                             return false;

                                         if (itemTemplate->LockID == 0)
                                             return false;

                                         Item* item = bot->GetItemByEntry(itemTemplate->ItemId);
                                         if (!item || !item->IsLocked())
                                             return false;

                                         LockEntry const* lockInfo = sLockStore.LookupEntry(itemTemplate->LockID);
                                         if (!lockInfo)
                                             return false;

                                         for (uint8 j = 0; j < 8; ++j)
                                         {
                                             if (lockInfo->Type[j] == LOCK_KEY_SKILL)
                                             {
                                                 uint32 skillId = SkillByLockType(LockType(lockInfo->Index[j]));
                                                 if (skillId == SKILL_LOCKPICKING)
                                                 {
                                                     uint32 requiredSkill = lockInfo->Skill[j];
                                                     uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING);
                                                     return botSkill >= requiredSkill;
                                                 }
                                             }
                                         }

                                         return false;
                                     });
}

Item* BotItemService::FindConsumableStatic(Player* bot, uint32 itemId)
{
    return FindItemInInventoryStatic(
        bot, [itemId](ItemTemplate const* pItemProto) -> bool
        {
            return (pItemProto->Class == ITEM_CLASS_CONSUMABLE || pItemProto->Class == ITEM_CLASS_TRADE_GOODS) &&
                   pItemProto->ItemId == itemId;
        });
}

Item* BotItemService::FindStoneForStatic(Player* bot, Item* weapon)
{
    if (!weapon)
        return nullptr;

    ItemTemplate const* item_template = weapon->GetTemplate();
    if (!item_template)
        return nullptr;

    static std::vector<uint32_t> const uPrioritizedSharpStoneIds = {
        ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE,   ELEMENTAL_SHARPENING_STONE, DENSE_SHARPENING_STONE,
        SOLID_SHARPENING_STONE,      HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE,    ROUGH_SHARPENING_STONE};

    static std::vector<uint32_t> const uPrioritizedWeightStoneIds = {
        ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE,    DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
        HEAVY_WEIGHTSTONE,      COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE};

    Item* stone = nullptr;
    ItemTemplate const* pProto = weapon->GetTemplate();
    if (pProto && (pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 ||
                   pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                   pProto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER || pProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM))
    {
        for (uint8 i = 0; i < std::size(uPrioritizedSharpStoneIds); ++i)
        {
            stone = FindConsumableStatic(bot, uPrioritizedSharpStoneIds[i]);
            if (stone)
                return stone;
        }
    }
    else if (pProto &&
             (pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
              pProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF || pProto->SubClass == ITEM_SUBCLASS_WEAPON_FIST))
    {
        for (uint8 i = 0; i < std::size(uPrioritizedWeightStoneIds); ++i)
        {
            stone = FindConsumableStatic(bot, uPrioritizedWeightStoneIds[i]);
            if (stone)
                return stone;
        }
    }

    return stone;
}

Item* BotItemService::FindOilForStatic(Player* bot, Item* weapon)
{
    if (!weapon)
        return nullptr;

    ItemTemplate const* item_template = weapon->GetTemplate();
    if (!item_template)
        return nullptr;

    static std::vector<uint32_t> const uPrioritizedWizardOilIds = {
        BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL,      LESSER_WIZARD_OIL, MINOR_WIZARD_OIL,
        BRILLIANT_MANA_OIL,   SUPERIOR_MANA_OIL,   LESSER_MANA_OIL, MINOR_MANA_OIL};

    static std::vector<uint32_t> const uPrioritizedManaOilIds = {
        BRILLIANT_MANA_OIL,  SUPERIOR_MANA_OIL, LESSER_MANA_OIL,   MINOR_MANA_OIL,  BRILLIANT_WIZARD_OIL,
        SUPERIOR_WIZARD_OIL, WIZARD_OIL,        LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};

    Item* oil = nullptr;
    int botClass = bot->getClass();
    int specTab = AiFactory::GetPlayerSpecTab(bot);

    std::vector<uint32_t> const* prioritizedOils = nullptr;
    switch (botClass)
    {
        case CLASS_PRIEST:
            prioritizedOils = (specTab == 2) ? &uPrioritizedWizardOilIds : &uPrioritizedManaOilIds;
            break;
        case CLASS_MAGE:
            prioritizedOils = &uPrioritizedWizardOilIds;
            break;
        case CLASS_DRUID:
            if (specTab == 0)  // Balance
                prioritizedOils = &uPrioritizedWizardOilIds;
            else if (specTab == 1)  // Feral
                prioritizedOils = nullptr;
            else
                prioritizedOils = &uPrioritizedManaOilIds;
            break;
        case CLASS_HUNTER:
            prioritizedOils = &uPrioritizedManaOilIds;
            break;
        case CLASS_PALADIN:
            if (specTab == 1)  // Protection
                prioritizedOils = &uPrioritizedWizardOilIds;
            else if (specTab == 2)  // Retribution
                prioritizedOils = nullptr;
            else
                prioritizedOils = &uPrioritizedManaOilIds;
            break;
        default:
            prioritizedOils = &uPrioritizedManaOilIds;
            break;
    }

    if (prioritizedOils)
    {
        for (auto const& id : *prioritizedOils)
        {
            oil = FindConsumableStatic(bot, id);
            if (oil)
                return oil;
        }
    }

    return oil;
}

void BotItemService::ImbueItemStatic(Player* bot, Item* item)
{
    ImbueItemStatic(bot, item, TARGET_FLAG_NONE, ObjectGuid::Empty);
}

void BotItemService::ImbueItemStatic(Player* bot, Item* item, Unit* target)
{
    if (!target || !target->IsInWorld() || target->IsDuringRemoveFromWorld())
        return;

    ImbueItemStatic(bot, item, TARGET_FLAG_UNIT, target->GetGUID());
}

void BotItemService::ImbueItemStatic(Player* bot, Item* item, uint8 targetInventorySlot)
{
    if (targetInventorySlot >= EQUIPMENT_SLOT_END)
        return;

    Item* const targetItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, targetInventorySlot);
    if (!targetItem)
        return;

    ImbueItemStatic(bot, item, TARGET_FLAG_ITEM, targetItem->GetGUID());
}

void BotItemService::ImbueItemStatic(Player* bot, Item* item, uint32 targetFlag, ObjectGuid targetGUID)
{
    if (!item)
        return;

    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 0;
    ObjectGuid item_guid = item->GetGUID();

    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    WorldPacket* packet = new WorldPacket(CMSG_USE_ITEM);
    *packet << bagIndex;
    *packet << slot;
    *packet << cast_count;
    *packet << spellId;
    *packet << item_guid;
    *packet << glyphIndex;
    *packet << castFlags;
    *packet << targetFlag;

    if (targetFlag & (TARGET_FLAG_UNIT | TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT))
        *packet << targetGUID.WriteAsPacked();

    bot->GetSession()->QueuePacket(packet);
}

void BotItemService::EnchantItemStatic(Player* bot, uint32 spellid, uint8 slot)
{
    Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!pItem || !pItem->IsInWorld() || !pItem->GetOwner() || !pItem->GetOwner()->IsInWorld() ||
        !pItem->GetOwner()->GetSession())
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return;

    uint32 enchantid = spellInfo->Effects[0].MiscValue;
    if (!enchantid)
        return;

    if (!((1 << pItem->GetTemplate()->SubClass) & spellInfo->EquippedItemSubClassMask) &&
        !((1 << pItem->GetTemplate()->InventoryType) & spellInfo->EquippedItemInventoryTypeMask))
    {
        return;
    }

    bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, false);
    pItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
    bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, true);

    LOG_INFO("playerbots", "{}: items was enchanted successfully!", bot->GetName().c_str());
}

std::vector<Item*> BotItemService::GetInventoryAndEquippedItemsStatic(Player* bot)
{
    std::vector<Item*> items;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    items.push_back(pItem);
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            items.push_back(pItem);
        }
    }

    return items;
}

std::vector<Item*> BotItemService::GetInventoryItemsStatic(Player* bot)
{
    std::vector<Item*> items;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    items.push_back(pItem);
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    return items;
}

uint32 BotItemService::GetInventoryItemsCountWithIdStatic(Player* bot, uint32 itemId)
{
    uint32 count = 0;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetTemplate()->ItemId == itemId)
                    {
                        count += pItem->GetCount();
                    }
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                count += pItem->GetCount();
            }
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                count += pItem->GetCount();
            }
        }
    }

    return count;
}

bool BotItemService::HasItemInInventoryStatic(Player* bot, uint32 itemId)
{
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetTemplate()->ItemId == itemId)
                    {
                        return true;
                    }
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                return true;
            }
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                return true;
            }
        }
    }

    return false;
}

uint32 BotItemService::GetEquipGearScoreStatic(Player* player)
{
    if (!player)
        return 0;

    uint32 gearScore = 0;
    uint32 itemCount = 0;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        // Skip shirt and tabard slots
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemTemplate const* proto = pItem->GetTemplate())
            {
                gearScore += proto->ItemLevel;
                itemCount++;
            }
        }
    }

    return itemCount > 0 ? gearScore / itemCount : 0;
}

std::vector<std::pair<Quest const*, uint32>> BotItemService::GetCurrentQuestsRequiringItemIdStatic(Player* bot,
                                                                                                    uint32 itemId)
{
    std::vector<std::pair<Quest const*, uint32>> result;

    if (!itemId)
        return result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        for (uint8 i = 0; i < std::size(quest->RequiredItemId); ++i)
        {
            if (quest->RequiredItemId[i] == itemId)
            {
                result.push_back(std::pair(quest, quest->RequiredItemId[i]));
                break;
            }
        }
    }

    return result;
}

// ============================================================================
// Instance method implementations (IItemService interface)
// These call static methods internally
// ============================================================================

Item* BotItemService::FindPoison() const
{
    if (_botAI)
    {
        return FindPoisonStatic(_botAI->GetBot());
    }
    return nullptr;
}

Item* BotItemService::FindAmmo() const
{
    if (_botAI)
    {
        return FindAmmoStatic(_botAI->GetBot());
    }
    return nullptr;
}

Item* BotItemService::FindBandage() const
{
    if (_botAI)
    {
        return FindBandageStatic(_botAI->GetBot());
    }
    return nullptr;
}

Item* BotItemService::FindOpenableItem() const
{
    if (_botAI)
    {
        return FindOpenableItemStatic(_botAI->GetBot());
    }
    return nullptr;
}

Item* BotItemService::FindLockedItem() const
{
    if (_botAI)
    {
        return FindLockedItemStatic(_botAI->GetBot());
    }
    return nullptr;
}

Item* BotItemService::FindConsumable(uint32 itemId) const
{
    if (_botAI)
    {
        return FindConsumableStatic(_botAI->GetBot(), itemId);
    }
    return nullptr;
}

Item* BotItemService::FindStoneFor(Item* weapon) const
{
    if (_botAI)
    {
        return FindStoneForStatic(_botAI->GetBot(), weapon);
    }
    return nullptr;
}

Item* BotItemService::FindOilFor(Item* weapon) const
{
    if (_botAI)
    {
        return FindOilForStatic(_botAI->GetBot(), weapon);
    }
    return nullptr;
}

void BotItemService::ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID)
{
    if (_botAI)
    {
        ImbueItemStatic(_botAI->GetBot(), item, targetFlag, targetGUID);
    }
}

void BotItemService::ImbueItem(Item* item, uint8 targetInventorySlot)
{
    if (_botAI)
    {
        ImbueItemStatic(_botAI->GetBot(), item, targetInventorySlot);
    }
}

void BotItemService::ImbueItem(Item* item, Unit* target)
{
    if (_botAI)
    {
        ImbueItemStatic(_botAI->GetBot(), item, target);
    }
}

void BotItemService::ImbueItem(Item* item)
{
    if (_botAI)
    {
        ImbueItemStatic(_botAI->GetBot(), item);
    }
}

void BotItemService::EnchantItem(uint32 spellId, uint8 slot)
{
    if (_botAI)
    {
        EnchantItemStatic(_botAI->GetBot(), spellId, slot);
    }
}

std::vector<Item*> BotItemService::GetInventoryAndEquippedItems() const
{
    if (_botAI)
    {
        return GetInventoryAndEquippedItemsStatic(_botAI->GetBot());
    }
    return {};
}

std::vector<Item*> BotItemService::GetInventoryItems() const
{
    if (_botAI)
    {
        return GetInventoryItemsStatic(_botAI->GetBot());
    }
    return {};
}

uint32 BotItemService::GetInventoryItemsCountWithId(uint32 itemId) const
{
    if (_botAI)
    {
        return GetInventoryItemsCountWithIdStatic(_botAI->GetBot(), itemId);
    }
    return 0;
}

bool BotItemService::HasItemInInventory(uint32 itemId) const
{
    if (_botAI)
    {
        return HasItemInInventoryStatic(_botAI->GetBot(), itemId);
    }
    return false;
}

InventoryResult BotItemService::CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool notLoading) const
{
    if (_botAI)
    {
        return _botAI->CanEquipItem(slot, dest, pItem, swap, notLoading);
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 BotItemService::FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const
{
    if (_botAI)
    {
        return _botAI->FindEquipSlot(proto, slot, swap);
    }
    return 0;
}

uint32 BotItemService::GetEquipGearScore(Player* player) const
{
    if (_botAI)
    {
        return GetEquipGearScoreStatic(player ? player : _botAI->GetBot());
    }
    return 0;
}

std::vector<std::pair<Quest const*, uint32>> BotItemService::GetCurrentQuestsRequiringItemId(uint32 itemId) const
{
    if (_botAI)
    {
        return GetCurrentQuestsRequiringItemIdStatic(_botAI->GetBot(), itemId);
    }
    return {};
}
