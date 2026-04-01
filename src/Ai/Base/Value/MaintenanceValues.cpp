/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AuctionHouseBotHelper.h"
#include "MaintenanceValues.h"

#include "Bag.h"
#include "BudgetValues.h"
#include "ItemUsageValue.h"
#include "PlayerbotAIConfig.h"
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
    uint32 ahCount = AI_VALUE(std::vector<uint32>, "ah sell list").size();
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

std::vector<uint32> AhSellListValue::Calculate()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return {};
    //Lets use a cache to avoid iterating through items.
    uint32 fingerprint = ComputeBagFingerprint();
    if (fingerprint == _lastFingerprint && !_cachedList.empty())
        return _cachedList;

    _lastFingerprint = fingerprint;
    _cachedList.clear();

    // Default backpack
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!item)
            continue;

        if (AI_VALUE2(ItemUsage, "item usage", item->GetEntry()) == ITEM_USAGE_AH)
            _cachedList.push_back(item->GetEntry());
    }

    // Extra bags
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag const* pBag = dynamic_cast<Bag const*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag));
        if (!pBag)
            continue;

        for (uint32 i = 0; i < pBag->GetBagSize(); ++i)
        {
            Item* item = bot->GetItemByPos(bag, i);
            if (!item)
                continue;

            if (AI_VALUE2(ItemUsage, "item usage", item->GetEntry()) == ITEM_USAGE_AH)
                _cachedList.push_back(item->GetEntry());
        }
    }

    return _cachedList;
}

bool ShouldAHSellValue::Calculate()
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    return !AI_VALUE(std::vector<uint32>, "ah sell list").empty();
}

std::vector<uint8> AhBuyListValue::Calculate()
{
    std::vector<uint8> weakSlots;

    uint32 botLevel = bot->GetLevel();

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
        {
            weakSlots.push_back(slot);
            continue;
        }

        if (item->GetTemplate()->ItemLevel < botLevel)
            weakSlots.push_back(slot);
    }

    return weakSlots;
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

    return !AI_VALUE(std::vector<uint8>, "ah buy list").empty();
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
