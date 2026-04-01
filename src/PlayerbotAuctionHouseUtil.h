/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AUCTIONHOUSEUTIL_H
#define _PLAYERBOT_AUCTIONHOUSEUTIL_H

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "AuctionHouseMgr.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Player.h"
#include "RandomPlayerbotMgr.h"

// ---------------------------------------------------------------------------
// Structs
// ---------------------------------------------------------------------------

struct PlayerbotAuctionItemPolicy
{
    bool sellable = true;
    uint8 chanceToSell = 100;
    uint16 minStackCount = 0;
    uint16 maxStackCount = 0;
    uint16 minBidPct = 100;
    uint16 buyoutMinPct = 110;
    uint16 buyoutMaxPct = 133;
    uint8 undercutChance = 15;
    uint16 marketPriceWeightPct = 75;
};

struct PlayerbotAuctionMarketSnapshot
{
    uint32 minUnitBuyout = 0;
    uint32 avgUnitBuyout = 0;
    uint32 sampleCount = 0;

    [[nodiscard]] bool HasData() const
    {
        return sampleCount > 0 && minUnitBuyout > 0;
    }
};

// ---------------------------------------------------------------------------
// Policy manager singleton (DB-backed, thread-safe)
// ---------------------------------------------------------------------------

class PlayerbotAuctionHouseUtil
{
public:
    static PlayerbotAuctionHouseUtil& instance()
    {
        static PlayerbotAuctionHouseUtil instance;
        return instance;
    }

    void Initialize()
    {
        std::lock_guard<std::mutex> guard(_lock);

        _policies.clear();
        bool const tableAvailable = TableExists();
        if (!tableAvailable)
        {
            LOG_WARN("playerbots", "playerbots_auction_item_policy table not found. Using built-in auction defaults.");
            return;
        }

        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT `item_id`, `sellable`, `chance_to_sell`, `min_stack_count`, `max_stack_count`, "
            "`min_bid_pct`, `buyout_min_pct`, `buyout_max_pct`, `undercut_chance`, `market_price_weight_pct` "
            "FROM `playerbots_auction_item_policy`");

        if (!result)
        {
            LOG_INFO("playerbots", "Loaded 0 playerbots auction item policies.");
            return;
        }

        do
        {
            Field* fields = result->Fetch();

            PlayerbotAuctionItemPolicy policy;
            policy.sellable = fields[1].Get<uint8>() != 0;
            policy.chanceToSell = std::min<uint32>(100, fields[2].Get<uint32>());
            policy.minStackCount = fields[3].Get<uint16>();
            policy.maxStackCount = fields[4].Get<uint16>();
            policy.minBidPct = std::max<uint32>(1, fields[5].Get<uint32>());
            policy.buyoutMinPct = std::max<uint32>(100, fields[6].Get<uint32>());
            policy.buyoutMaxPct = std::max<uint32>(policy.buyoutMinPct, fields[7].Get<uint32>());
            policy.undercutChance = std::min<uint32>(100, fields[8].Get<uint32>());
            policy.marketPriceWeightPct = std::min<uint32>(100, fields[9].Get<uint32>());

            _policies[fields[0].Get<uint32>()] = policy;
        } while (result->NextRow());

        LOG_INFO("playerbots", "Loaded {} playerbots auction item policies.", _policies.size());
    }

    [[nodiscard]] PlayerbotAuctionItemPolicy GetPolicy(uint32 itemId) const
    {
        std::lock_guard<std::mutex> guard(_lock);

        auto itr = _policies.find(itemId);
        if (itr != _policies.end())
            return itr->second;

        return MakeDefaultPolicy();
    }

    [[nodiscard]] bool IsSellable(uint32 itemId) const
    {
        return GetPolicy(itemId).sellable;
    }

private:
    [[nodiscard]] PlayerbotAuctionItemPolicy MakeDefaultPolicy() const
    {
        PlayerbotAuctionItemPolicy policy;
        policy.buyoutMinPct = std::max<uint32>(100, sPlayerbotAIConfig.auctionHouseBuyoutMinPct);
        policy.buyoutMaxPct = std::max<uint32>(policy.buyoutMinPct, sPlayerbotAIConfig.auctionHouseBuyoutMaxPct);
        policy.undercutChance = std::min<uint32>(100, sPlayerbotAIConfig.auctionHouseUndercutChance);
        return policy;
    }

    [[nodiscard]] bool TableExists() const
    {
        std::string const dbName = PlayerbotsDatabase.GetConnectionInfo()->database;
        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT EXISTS(SELECT 1 FROM information_schema.tables WHERE table_schema = '{}' "
            "AND table_name = 'playerbots_auction_item_policy')",
            dbName);

        if (!result)
            return false;

        return result->Fetch()[0].Get<uint32>() != 0;
    }

private:
    mutable std::mutex _lock;
    std::unordered_map<uint32, PlayerbotAuctionItemPolicy> _policies;
};

#define sPlayerbotAuctionHouseUtil PlayerbotAuctionHouseUtil::instance()

// ---------------------------------------------------------------------------
// Item classification helpers
// ---------------------------------------------------------------------------

inline constexpr uint32 AuctionHouseMaterialMinCount = 5;

inline bool IsAuctionHouseMaterial(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_GEM:
            return true;
        case ITEM_CLASS_MISC:
            return proto->SubClass != ITEM_SUBCLASS_REAGENT;
        default:
            return false;
    }
}

inline bool IsPreferredAuctionHouseItem(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    if (proto->Quality >= ITEM_QUALITY_UNCOMMON)
        return true;

    if (IsAuctionHouseMaterial(proto))
        return true;

    if (proto->Bonding != NO_BIND || proto->Quality < ITEM_QUALITY_NORMAL)
        return false;

    switch (proto->Class)
    {
        case ITEM_CLASS_CONTAINER:
        case ITEM_CLASS_CONSUMABLE:
        case ITEM_CLASS_ARMOR:
        case ITEM_CLASS_WEAPON:
            return true;
        default:
            return false;
    }
}

// ---------------------------------------------------------------------------
// Market snapshot / pricing
// ---------------------------------------------------------------------------

inline PlayerbotAuctionMarketSnapshot GetPlayerbotAuctionMarketSnapshot(
    AuctionHouseObject* auctionHouse, uint32 itemId, ObjectGuid owner = ObjectGuid(), uint32 maxSamples = 64)
{
    PlayerbotAuctionMarketSnapshot snapshot;
    if (!auctionHouse || !itemId)
        return snapshot;

    uint64 totalUnitBuyout = 0;
    for (auto itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
    {
        AuctionEntry const* auction = itr->second;
        if (!auction || auction->item_template != itemId || !auction->buyout || !auction->itemCount)
            continue;

        if (!owner.IsEmpty() && auction->owner == owner)
            continue;

        uint32 unitBuyout = std::max<uint32>(1, auction->buyout / auction->itemCount);
        if (!snapshot.minUnitBuyout || unitBuyout < snapshot.minUnitBuyout)
            snapshot.minUnitBuyout = unitBuyout;

        totalUnitBuyout += unitBuyout;
        ++snapshot.sampleCount;

        if (maxSamples && snapshot.sampleCount >= maxSamples)
            break;
    }

    if (snapshot.sampleCount)
        snapshot.avgUnitBuyout = std::max<uint32>(1, totalUnitBuyout / snapshot.sampleCount);

    return snapshot;
}

inline uint32 GetPlayerbotAuctionReferenceUnitPrice(PlayerbotAuctionMarketSnapshot const& snapshot)
{
    if (!snapshot.HasData())
        return 0;

    if (snapshot.sampleCount == 1)
        return snapshot.minUnitBuyout;

    return std::max<uint32>(1, (snapshot.minUnitBuyout + snapshot.avgUnitBuyout) / 2);
}

// ---------------------------------------------------------------------------
// Pricing / sell helpers
// ---------------------------------------------------------------------------

inline uint32 RoundAuctionPrice(double price)
{
    if (price <= 1.0)
        return 1;

    if (price < 100.0)
        return uint32(price);

    if (price < 10000.0)
        return uint32(price / 100.0) * 100;

    if (price < 100000.0)
        return uint32(price / 1000.0) * 1000;

    return uint32(price / 10000.0) * 10000;
}

inline uint32 GetAuctionUnitPrice(Player* bot, ItemTemplate const* proto,
    AuctionHouseObject* auctionHouse, PlayerbotAuctionItemPolicy const& policy,
    PlayerbotAuctionMarketSnapshot* marketSnapshot = nullptr)
{
    if (!bot || !proto)
        return 0;

    uint32 marketWeight = std::min<uint32>(100, policy.marketPriceWeightPct);

    uint32 unitPrice = 0;

    if (proto->BuyPrice)
        unitPrice = RoundAuctionPrice(proto->BuyPrice * sRandomPlayerbotMgr.GetBuyMultiplier(bot));
    else if (proto->SellPrice)
        unitPrice = RoundAuctionPrice(proto->SellPrice * std::max(1.0, sRandomPlayerbotMgr.GetSellMultiplier(bot)));
    else
        unitPrice = 1;

    if (!marketWeight)
        return std::max<uint32>(1, unitPrice);

    PlayerbotAuctionMarketSnapshot snapshot =
        GetPlayerbotAuctionMarketSnapshot(auctionHouse, proto->ItemId, bot->GetGUID());
    if (marketSnapshot)
        *marketSnapshot = snapshot;

    uint32 marketUnitPrice = GetPlayerbotAuctionReferenceUnitPrice(snapshot);
    if (!marketUnitPrice)
        return std::max<uint32>(1, unitPrice);

    return std::max<uint32>(1,
        RoundAuctionPrice((double(unitPrice) * (100 - marketWeight) + double(marketUnitPrice) * marketWeight) / 100.0));
}

inline bool HasNearbyAuctioneer(Player* bot, GuidVector const& npcs, ObjectGuid& auctioneerGuid)
{
    for (ObjectGuid const& guid : npcs)
    {
        if (!bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER))
            continue;

        auctioneerGuid = guid;
        return true;
    }

    return false;
}

inline uint32 GetAuctionStackCount(Item* item, PlayerbotAuctionItemPolicy const& policy)
{
    if (!item)
        return 0;

    uint32 itemCount = item->GetCount();
    if (!itemCount)
        return 0;

    uint32 maxStackCount = std::min<uint32>(itemCount, item->GetMaxStackCount());
    if (policy.maxStackCount)
        maxStackCount = std::min<uint32>(maxStackCount, std::max<uint32>(1, policy.maxStackCount));

    if (!sPlayerbotAIConfig.auctionHouseRandomStackSize)
        return maxStackCount ? maxStackCount : itemCount;

    if (maxStackCount <= 1)
        return 1;

    uint32 minStackCount = 1;
    if (IsAuctionHouseMaterial(item->GetTemplate()) && itemCount >= AuctionHouseMaterialMinCount)
        minStackCount = std::min<uint32>(AuctionHouseMaterialMinCount, maxStackCount);

    if (policy.minStackCount)
        minStackCount = std::min<uint32>(std::max<uint32>(1, policy.minStackCount), maxStackCount);

    if (maxStackCount <= minStackCount)
        return maxStackCount;

    return urand(minStackCount, maxStackCount);
}

#endif
