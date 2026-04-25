/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MaintenanceValues.h"

#include "Bag.h"
#include "BudgetValues.h"
#include "ItemUsageValue.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotAuctionHouseUtil.h"
#include "Playerbots.h"

bool CanMoveAroundValue::Calculate()
{
    if (bot->IsInCombat())
        return false;

    if (bot->GetTradeData())
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
        return false;

    if (!AI_VALUE(bool, "group ready"))
        return false;

    return true;
}

bool ShouldHomeBindValue::Calculate() { return AI_VALUE2(float, "distance", "home bind") > 1000.0f; }

bool ShouldRepairValue::Calculate() { return AI_VALUE(uint8, "durability") < 80; }

bool CanRepairValue::Calculate()
{
    return AI_VALUE(uint8, "durability") < 100 &&
           AI_VALUE(uint32, "repair cost") < AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::repair);
}

bool ShouldSellValue::Calculate() { return AI_VALUE(uint8, "bag space") > 60; }

bool CanSellValue::Calculate()
{
    uint32 ahCount = AI_VALUE(AhListMap&, "ah sell list").size();
    if (ahCount > 0)
        return true;

    return AI_VALUE2(uint32, "item count", "usage " + std::to_string(ITEM_USAGE_VENDOR)) > 1;
}

uint32 AhSellListValue::ComputeBagFingerprint()
{
    uint32 hash = 0;
    uint32 slot = 1;

    // Default backpack
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            hash ^= (item->GetEntry() * 31 + item->GetCount()) * slot;
        ++slot;
    }

    // Extra bags
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag const* pBag = dynamic_cast<Bag const*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag));
        if (!pBag)
            continue;

        for (uint32 i = 0; i < pBag->GetBagSize(); ++i)
        {
            if (Item* item = bot->GetItemByPos(bag, i))
                hash ^= (item->GetEntry() * 31 + item->GetCount()) * slot;
            ++slot;
        }
    }

    return hash;
}

bool AhSellListValue::IsItemSellableOnAh(Item* item) const
{
    if (!item)
        return false;

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto || !item->CanBeTraded())
        return false;

    // Cheap pre-filter — skip items that are clearly not AH material
    if (!BotAuctionUtils::IsAuctionableGear(proto))
        return false;

    if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM)
        return false;

    uint32 entry = item->GetEntry();
    if (sPlayerbotAIConfig.IsInAuctionHouseExcludedItemList(entry))
        return false;

    if (!sPlayerbotAuctionHouseUtil.GetPolicy(entry).sellable)
        return false;

    // Expensive check last
    if (AI_VALUE2(ItemUsage, "item usage", entry) != ITEM_USAGE_AH)
        return false;

    return true;
}

AhListMap& AhSellListValue::Get()
{
    CheckInventory();
    return _data;
}

void AhSellListValue::CheckInventory()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
    {
        _data.clear();
        return;
    }
    uint32 nowMs = getMSTime();
    if (_lastReconcileMs && nowMs - _lastReconcileMs < MINUTE * IN_MILLISECONDS)
        return;
    _lastReconcileMs = nowMs;

    uint32 fingerprint = ComputeBagFingerprint();
    if (fingerprint == _lastFingerprint && !_data.empty())
        return;
    _lastFingerprint = fingerprint;

    // Collect currently sellable entries.
    std::unordered_set<uint32> newItemEntries;
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (IsItemSellableOnAh(item))
            newItemEntries.insert(item->GetEntry());
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag const* pBag = dynamic_cast<Bag const*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag));
        if (!pBag)
            continue;

        for (uint32 i = 0; i < pBag->GetBagSize(); ++i)
        {
            Item* item = bot->GetItemByPos(bag, i);
            if (IsItemSellableOnAh(item))
                newItemEntries.insert(item->GetEntry());
        }
    }

    // Remove entries that no longer match inventory.
    for (auto it = _data.begin(); it != _data.end(); )
    {
        if (newItemEntries.count(it->first))
            ++it;
        else
            it = _data.erase(it);
    }

    for (uint32 entry : newItemEntries)
        _data.try_emplace(entry);
}

bool ShouldAHSellValue::Calculate()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    return !AI_VALUE(AhListMap&, "ah sell list").empty();
}

bool AhBuyListValue::IsSlotWeak(uint8 slot) const
{
    if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
        return false;

    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!item)
        return true;

    //TODO: The criteria for what qualifies as a bad slot is not great atm.
    return item->GetTemplate()->RequiredLevel < bot->GetLevel() - 2;
}

AhListMap& AhBuyListValue::Get()
{
    CheckEquipment();
    return _data;
}

void AhBuyListValue::CheckEquipment()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
    {
        _data.clear();
        return;
    }

    uint32 nowMs = getMSTime();
    if (_lastReconcileMs && nowMs - _lastReconcileMs < MINUTE * IN_MILLISECONDS)
        return;
    _lastReconcileMs = nowMs;

    // Drop slots that are no longer weak.
    for (auto it = _data.begin(); it != _data.end(); )
    {
        if (IsSlotWeak(static_cast<uint8>(it->first)))
            ++it;
        else
            it = _data.erase(it);
    }

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (IsSlotWeak(slot))
            _data.try_emplace(slot);
    }
}

bool ShouldAHBuyValue::Calculate()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    if (AI_VALUE(bool, "should repair"))
        return false;

    uint32 gearBudget = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::gear);
    if (!gearBudget)
        return false;

    // Any Idle or expired-Failed slot counts as actionable. PendingCheck-only
    // or cooldown-only states shouldn't trigger a city trip.
    auto& buyList = AI_VALUE(AhListMap&, "ah buy list");
    time_t now = time(nullptr);
    for (auto const& kv : buyList)
    {
        if (kv.second.status == AhStatus::Idle)
            return true;
        else if (kv.second.status == AhStatus::Failed && now >= kv.second.retryAfter)
            return true;
    }
    return false;
}

bool CanTrainValue::Calculate()
{
    return AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) > 0;
}

bool CanFightEqualValue::Calculate() { return AI_VALUE(uint8, "durability") > 20; }

bool CanFightEliteValue::Calculate()
{
    return bot->GetGroup() && AI_VALUE2(bool, "group and", "can fight equal") &&
           AI_VALUE2(bool, "group and", "following party") && !AI_VALUE2(bool, "group or", "should sell,can sell");
}

bool CanFightBossValue::Calculate()
{
    return bot->GetGroup() && bot->GetGroup()->GetMembersCount() > 3 &&
           AI_VALUE2(bool, "group and", "can fight equal") && AI_VALUE2(bool, "group and", "following party") &&
           !AI_VALUE2(bool, "group or", "should sell,can sell");
}
