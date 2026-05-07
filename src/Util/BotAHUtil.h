/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _BOT_AH_UTIL_H
#define _BOT_AH_UTIL_H

#include <algorithm>
#include <array>
#include <cstring>
#include <ctime>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "AuctionHouseMgr.h"
#include "AuctionHouseSearcher.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Player.h"
#include "RandomPlayerbotMgr.h"

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

struct PlayerbotAuctionPriceKey
{
    uint32 itemEntry;
    uint8 ahFaction;

    bool operator==(PlayerbotAuctionPriceKey const& other) const
    {
        return itemEntry == other.itemEntry && ahFaction == other.ahFaction;
    }
};

struct PlayerbotAuctionPriceKeyHash
{
    std::size_t operator()(PlayerbotAuctionPriceKey const& k) const noexcept
    {
        return (std::size_t(k.itemEntry) << 8) ^ std::size_t(k.ahFaction);
    }
};

// One observation of the AH market for a given (item, faction), captured per
// CMSG_AUCTION_LIST_ITEMS response. Aggregates across all listings in that
// single query — not a single listing.
struct MarketSample
{
    uint32 minUnit = 0;
    uint32 avgUnit = 0;
    uint16 sampleCount = 0;      // number of listings aggregated in this sample
    uint16 _pad = 0;             // keeps sizeof() at 16 for packed serialization
    uint32 observedAt = 0;       // epoch seconds; 0 = tombstoned by InvalidateMarketSnapshot
};
static_assert(sizeof(MarketSample) == 16,
    "MarketSample must be 16 bytes — persisted as packed little-endian on disk.");
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
    "MarketSample blob is packed in host byte order; only little-endian targets are supported. "
    "Switch to explicit LE byte-by-byte packing in PackHistory/UnpackHistory if porting.");

// Rolling buffer of up to 100 MarketSamples per (item, faction). Newest sample
// sits at `(head - 1 + kCapacity) % kCapacity`. `dirty` means there are
// unpersisted in-memory changes — cleared by FlushDirty().
struct CachedPriceHistory
{
    static constexpr size_t kCapacity = 100;
    std::array<MarketSample, kCapacity> samples{};
    uint8 head = 0;
    uint8 filled = 0;
    bool dirty = false;

    void Push(MarketSample const& s)
    {
        samples[head] = s;
        head = uint8((head + 1) % kCapacity);
        if (filled < kCapacity)
            ++filled;
        dirty = true;
    }

    [[nodiscard]] MarketSample const* Latest() const
    {
        if (!filled)
            return nullptr;
        return &samples[(head + kCapacity - 1) % kCapacity];
    }

    [[nodiscard]] MarketSample* Latest()
    {
        if (!filled)
            return nullptr;
        return &samples[(head + kCapacity - 1) % kCapacity];
    }
};

// ---------------------------------------------------------------------------
// BotAHUtil — AH policy + price-history singleton.
//
// Owns two pieces of state (shared_mutex, reads dominate):
//   - _policies: (DB: playerbots_auction_item_policy) per-item posting knobs
//   - _prices:   (DB: playerbots_auction_price_cache) ring buffer of up to
//                kCapacity (=100) MarketSamples per (item, faction_bucket).
//
// Write cadence:
//   StoreMarketSnapshot pushes into memory + marks dirty; no DB I/O.
//   FlushDirty() is called periodically from PlayerbotsWorldScript::OnUpdate
//   (every ~15 min) and once on shutdown. Writes are a single batched
//   multi-row REPLACE INTO (chunked at 200 rows to stay under packet limits).
//
// ---------------------------------------------------------------------------
// TODO — evolving the pricing brain beyond MVP:
//
// 1. Use the history buffer for aggregates (currently only the latest sample
//    is consulted in GetMarketSnapshot / GetAuctionPrice). Candidate derivations:
//      - short-window "current market": avg of last 3-5 snapshots
//      - long-window "reference price": EWMA with α ≈ 0.01 for manipulation
//        resistance — shifts ~half over ~70 snapshots (≈12 h at 10-min TTL)
//
// 2. Manipulation-resistant pricing logic:
//      - Sell:  unitPrice = blend(template, referencePrice, weight)
//               (anchor-based, not spot-based)
//      - Buy:   reject candidates with  buyout > 2.0 × referencePrice
//               flag bargains where    buyout < 0.3 × referencePrice
//      - Within-snapshot: use MEDIAN of listings not MEAN (outlier resistance)
//
// 3. Shift detection for legitimate market moves:
//      - compare "last N" slice avg vs "older N" slice avg
//      - sustained divergence promotes candidate to new normal
//      - single-snapshot divergence treated as noise / manipulation
//
// 4. Ops:
//      - admin command to flush referencePrice for real-world shifts
//      - metric: |current - reference| / reference logged as volatility signal
// ---------------------------------------------------------------------------

namespace BotAuctionUtils
{
    // Three-valued result for the cache-driven pricing path.
    enum class AuctionPriceStatus : uint8
    {
        NoData,        // cache miss or stale — caller must query + defer
        TemplateOnly,  // queried but no listings — use template-only price
        Blended,       // queried with samples — blended template/market price
    };

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

    inline bool IsAuctionableReagent(ItemTemplate const* proto)
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

    inline bool IsAuctionableGear(ItemTemplate const* proto)
    {
        if (!proto)
            return false;

        if (proto->Quality >= ITEM_QUALITY_UNCOMMON)
            return true;

        if (IsAuctionableReagent(proto))
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

    inline uint32 GetAuctionStackCount(Item* item, PlayerbotAuctionItemPolicy const& policy)
    {
        if (!item)
            return 0;

        uint32 itemCount = item->GetCount();
        if (!itemCount)
            return 0;

        // Gear always lists one at a time — stack size doesn't apply.
        if (!IsAuctionableReagent(item->GetTemplate()))
            return 1;

        uint32 const naturalUnit = std::max<uint32>(1, sPlayerbotAIConfig.auctionHouseMaterialStackSize);
        uint32 const inventoryCap = std::min<uint32>(itemCount, item->GetMaxStackCount());

        uint32 maxStackCount = policy.maxStackCount ? policy.maxStackCount : naturalUnit;
        maxStackCount = std::min<uint32>(maxStackCount, inventoryCap);

        uint32 minStackCount = policy.minStackCount ? policy.minStackCount : naturalUnit;
        minStackCount = std::min<uint32>(minStackCount, maxStackCount);

        if (maxStackCount <= minStackCount)
            return maxStackCount;

        return urand(minStackCount, maxStackCount);
    }

    inline void SendAhSearchForSlot(Player* bot, Creature* auctioneer, uint8 equipSlot)
    {
        if (!bot || !auctioneer)
            return;

        // Map equipment slot to AH inventory type filter
        uint32 inventoryType = 0;
        switch (equipSlot)
        {
            case EQUIPMENT_SLOT_HEAD:      inventoryType = INVTYPE_HEAD; break;
            case EQUIPMENT_SLOT_NECK:      inventoryType = INVTYPE_NECK; break;
            case EQUIPMENT_SLOT_SHOULDERS: inventoryType = INVTYPE_SHOULDERS; break;
            case EQUIPMENT_SLOT_CHEST:     inventoryType = INVTYPE_CHEST; break;
            case EQUIPMENT_SLOT_WAIST:     inventoryType = INVTYPE_WAIST; break;
            case EQUIPMENT_SLOT_LEGS:      inventoryType = INVTYPE_LEGS; break;
            case EQUIPMENT_SLOT_FEET:      inventoryType = INVTYPE_FEET; break;
            case EQUIPMENT_SLOT_WRISTS:    inventoryType = INVTYPE_WRISTS; break;
            case EQUIPMENT_SLOT_HANDS:     inventoryType = INVTYPE_HANDS; break;
            case EQUIPMENT_SLOT_FINGER1:
            case EQUIPMENT_SLOT_FINGER2:   inventoryType = INVTYPE_FINGER; break;
            case EQUIPMENT_SLOT_TRINKET1:
            case EQUIPMENT_SLOT_TRINKET2:  inventoryType = INVTYPE_TRINKET; break;
            case EQUIPMENT_SLOT_BACK:      inventoryType = INVTYPE_CLOAK; break;
            case EQUIPMENT_SLOT_MAINHAND:
            case EQUIPMENT_SLOT_OFFHAND:
            case EQUIPMENT_SLOT_RANGED:    inventoryType = 0; break;
            default: return;
        }

        // Armor class for armor slots, any for weapons/jewelry
        uint32 itemClass = 0xFFFFFFFF;
        uint32 itemSubClass = 0xFFFFFFFF;
        if (inventoryType != 0 &&
            equipSlot != EQUIPMENT_SLOT_FINGER1 &&
            equipSlot != EQUIPMENT_SLOT_FINGER2 &&
            equipSlot != EQUIPMENT_SLOT_TRINKET1 &&
            equipSlot != EQUIPMENT_SLOT_TRINKET2 &&
            equipSlot != EQUIPMENT_SLOT_NECK &&
            equipSlot != EQUIPMENT_SLOT_BACK)
        {
            itemClass = ITEM_CLASS_ARMOR;
            //TODO, filter items based on class/spec
        }

        uint8 levelMin = bot->GetLevel() > 5 ? bot->GetLevel() - 5 : 1;
        uint8 levelMax = bot->GetLevel();

        WorldPacket packet(CMSG_AUCTION_LIST_ITEMS);
        packet << auctioneer->GetGUID();
        packet << uint32(0);                       // listfrom (page 0)
        packet << std::string("");                 // no name filter
        packet << levelMin;
        packet << levelMax;
        packet << inventoryType;
        packet << itemClass;
        packet << itemSubClass;
        packet << uint32(ITEM_QUALITY_UNCOMMON);   // green minimum
        packet << uint8(1);                        // usable only

        packet << uint8(0);                        // getAll = false

        // Sort: rarity descending (best quality first), then level descending (highest ilvl first)
        packet << uint8(2);
        packet << uint8(AUCTION_SORT_RARITY) << uint8(1);
        packet << uint8(AUCTION_SORT_MINLEVEL) << uint8(1);

        auto opPacket = std::make_unique<AuctionPacketOperation>(
            bot->GetGUID(), auctioneer->GetGUID(), std::move(packet));
        PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(opPacket));

        LOG_DEBUG("playerbots", "[AH] Bot {} sent AH search for slot {} (invType={}, level={}-{})",
                  bot->GetName(), equipSlot, inventoryType, levelMin, levelMax);
    }
}

class BotAHUtil
{
public:
    static BotAHUtil& instance()
    {
        static BotAHUtil instance;
        return instance;
    }

    void Initialize()
    {
        LoadPolicies();
        LoadPriceCache();
    }

    [[nodiscard]] PlayerbotAuctionItemPolicy GetPolicy(uint32 itemId) const
    {
        std::shared_lock<std::shared_mutex> guard(_mtx);
        auto itr = _policies.find(itemId);
        if (itr != _policies.end())
            return itr->second;
        return MakeDefaultPolicy();
    }

    // Returns a snapshot iff the cache's latest sample is fresh. nullopt covers
    // miss, stale, and explicitly-invalidated. A present value with HasData()
    // false is a tombstone (queried, no listings — caller uses template price).
    //
    // NOTE (MVP): derived from the most recent sample only. The full history
    // buffer is persisted but not yet consulted here — see TODO for reference
    // price / shift detection.
    [[nodiscard]] std::optional<PlayerbotAuctionMarketSnapshot> GetMarketSnapshot(
        uint32 itemId, uint32 auctioneerFactionTemplate) const
    {
        if (!itemId)
            return std::nullopt;

        PlayerbotAuctionPriceKey key{itemId, ResolveAhFactionBucket(auctioneerFactionTemplate)};
        std::shared_lock<std::shared_mutex> guard(_mtx);
        auto itr = _prices.find(key);
        if (itr == _prices.end())
            return std::nullopt;
        MarketSample const* latest = itr->second.Latest();
        if (!latest || !IsFresh(latest->observedAt))
            return std::nullopt;

        PlayerbotAuctionMarketSnapshot snapshot;
        snapshot.minUnitBuyout = latest->minUnit;
        snapshot.avgUnitBuyout = latest->avgUnit;
        snapshot.sampleCount = latest->sampleCount;
        return snapshot;
    }

    // Pushes one market observation into the in-memory ring buffer and marks
    // the entry dirty for the next FlushDirty(). NO DB write — flushing is
    // batched via the periodic hook / shutdown handler.
    // `snapshot.sampleCount == 0` pushes a tombstone (queried, no listings).
    void StoreMarketSnapshot(uint32 itemId, uint32 auctioneerFactionTemplate,
        PlayerbotAuctionMarketSnapshot const& snapshot)
    {
        uint8 bucket = ResolveAhFactionBucket(auctioneerFactionTemplate);
        PlayerbotAuctionPriceKey key{itemId, bucket};

        MarketSample sample;
        sample.minUnit = snapshot.minUnitBuyout;
        sample.avgUnit = snapshot.avgUnitBuyout;
        sample.sampleCount = uint16(std::min<uint32>(snapshot.sampleCount, 0xFFFF));
        sample.observedAt = uint32(time(nullptr));

        std::unique_lock<std::shared_mutex> guard(_mtx);
        _prices[key].Push(sample);  // Push also sets dirty=true
    }

    // Marks the most-recent sample as stale so the next pricing pass re-queries.
    // Called after a bot posts a new auction so its own listing is visible next
    // lookup. Doesn't touch the buffer's history — just the freshness of the
    // latest observation.
    void InvalidateMarketSnapshot(uint32 itemId, uint32 auctioneerFactionTemplate)
    {
        PlayerbotAuctionPriceKey key{itemId, ResolveAhFactionBucket(auctioneerFactionTemplate)};
        std::unique_lock<std::shared_mutex> guard(_mtx);
        auto itr = _prices.find(key);
        if (itr == _prices.end())
            return;
        if (MarketSample* latest = itr->second.Latest())
        {
            latest->observedAt = 0;
            itr->second.dirty = true;
        }
    }

    // Writes all dirty entries to DB in one batched REPLACE INTO, then clears
    // the dirty flags. Called periodically (~every 15 min) from the world
    // thread and once on shutdown.
    void FlushDirty()
    {
        struct PendingFlush
        {
            uint32 itemEntry;
            uint8 bucket;
            uint32 updatedAt;
            std::string samplesHex;
        };
        std::vector<PendingFlush> pending;
        {
            std::unique_lock<std::shared_mutex> guard(_mtx);
            pending.reserve(_prices.size());
            for (auto& [key, history] : _prices)
            {
                if (!history.dirty)
                    continue;
                MarketSample const* latest = history.Latest();
                pending.push_back({
                    key.itemEntry, key.ahFaction,
                    latest ? latest->observedAt : uint32(time(nullptr)),
                    PackHistory(history)});
                history.dirty = false;
            }
        }

        if (pending.empty())
            return;

        // Single multi-row REPLACE INTO. Size check: each row is at most
        // ~3270 chars (3200 hex + scaffolding). 200 rows ≈ 650 KB — comfortably
        // under default max_allowed_packet (4+ MB), but chunk to stay safe.
        static constexpr size_t kBatchRows = 200;
        for (size_t start = 0; start < pending.size(); start += kBatchRows)
        {
            size_t const end = std::min(start + kBatchRows, pending.size());
            std::string sql = "REPLACE INTO playerbots_auction_price_cache "
                "(item_entry, ah_faction, updated_at, samples) VALUES ";
            for (size_t i = start; i < end; ++i)
            {
                if (i > start)
                    sql += ", ";
                PendingFlush const& p = pending[i];
                sql += "(";
                sql += std::to_string(p.itemEntry);
                sql += ",";
                sql += std::to_string(uint32(p.bucket));
                sql += ",";
                sql += std::to_string(p.updatedAt);
                sql += ",'";
                sql += p.samplesHex;
                sql += "')";
            }
            PlayerbotsDatabase.Execute(sql.c_str());
        }

        LOG_DEBUG("playerbots", "Flushed {} auction price history rows", pending.size());
    }

    [[nodiscard]] BotAuctionUtils::AuctionPriceStatus GetAuctionPrice(Player* bot, ItemTemplate const* proto, uint32 auctioneerFactionTemplate,
        uint32& outUnitPrice, PlayerbotAuctionMarketSnapshot* outSnapshot = nullptr) const
    {
        outUnitPrice = 0;
        if (outSnapshot)
            *outSnapshot = PlayerbotAuctionMarketSnapshot{};
        if (!bot || !proto)
            return BotAuctionUtils::AuctionPriceStatus::NoData;

        uint32 templatePrice = 1;
        if (proto->BuyPrice)
            templatePrice = BotAuctionUtils::RoundAuctionPrice(
                proto->BuyPrice * sRandomPlayerbotMgr.GetBuyMultiplier(bot));
        else if (proto->SellPrice)
            templatePrice = BotAuctionUtils::RoundAuctionPrice(
                proto->SellPrice * std::max(1.0, sRandomPlayerbotMgr.GetSellMultiplier(bot)));

        std::optional<PlayerbotAuctionMarketSnapshot> snapshot =
            GetMarketSnapshot(proto->ItemId, auctioneerFactionTemplate);
        if (!snapshot)
            return BotAuctionUtils::AuctionPriceStatus::NoData;

        if (!snapshot->HasData())
        {
            // Fresh tombstone — queried and found no listings. Use template.
            outUnitPrice = templatePrice;
            return BotAuctionUtils::AuctionPriceStatus::TemplateOnly;
        }

        if (outSnapshot)
            *outSnapshot = *snapshot;

        // Blend min and mean — min alone races bots to the floor, mean alone
        // ignores it.
        uint32 marketUnitPrice = (snapshot->minUnitBuyout + snapshot->avgUnitBuyout) / 2;
        PlayerbotAuctionItemPolicy const policy = GetPolicy(proto->ItemId);
        uint32 marketWeight = std::min<uint32>(100, policy.marketPriceWeightPct);
        outUnitPrice = BotAuctionUtils::RoundAuctionPrice(
            (double(templatePrice) * (100 - marketWeight) + double(marketUnitPrice) * marketWeight) / 100.0);
        return BotAuctionUtils::AuctionPriceStatus::Blended;
    }

private:
    [[nodiscard]] bool IsFresh(uint32 observedAt) const
    {
        if (!observedAt)
            return false;
        uint32 ttl = std::max<uint32>(1, sPlayerbotAIConfig.auctionPriceCacheTtlSeconds);
        return uint32(time(nullptr)) - observedAt < ttl;
    }

    // Pack samples in chronological order (oldest → newest) as uppercase hex.
    // MVP uses host byte order (x86 / x86_64 → little-endian). If we ever move
    // to a mixed-endian deployment, swap this to explicit LE packing.
    [[nodiscard]] static std::string PackHistory(CachedPriceHistory const& h)
    {
        static constexpr char const HEX[] = "0123456789ABCDEF";
        std::string out;
        out.reserve(size_t(h.filled) * sizeof(MarketSample) * 2);
        for (uint8 i = 0; i < h.filled; ++i)
        {
            uint8 idx = uint8((h.head + CachedPriceHistory::kCapacity - h.filled + i)
                              % CachedPriceHistory::kCapacity);
            uint8 const* raw = reinterpret_cast<uint8 const*>(&h.samples[idx]);
            for (size_t b = 0; b < sizeof(MarketSample); ++b)
            {
                out.push_back(HEX[raw[b] >> 4]);
                out.push_back(HEX[raw[b] & 0x0F]);
            }
        }
        return out;
    }

    static void UnpackHistory(std::string const& hex, CachedPriceHistory& out)
    {
        auto hexValue = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return -1;
        };
        size_t const sampleBytes = sizeof(MarketSample);
        size_t const count = std::min<size_t>(hex.size() / (sampleBytes * 2),
                                              CachedPriceHistory::kCapacity);
        for (size_t i = 0; i < count; ++i)
        {
            uint8* raw = reinterpret_cast<uint8*>(&out.samples[i]);
            for (size_t b = 0; b < sampleBytes; ++b)
            {
                int hi = hexValue(hex[(i * sampleBytes + b) * 2]);
                int lo = hexValue(hex[(i * sampleBytes + b) * 2 + 1]);
                if (hi < 0 || lo < 0)
                {
                    out.filled = 0;
                    out.head = 0;
                    return;  // corrupt row — drop the history, don't half-load
                }
                raw[b] = uint8((hi << 4) | lo);
            }
        }
        out.filled = uint8(count);
        out.head = uint8(count % CachedPriceHistory::kCapacity);
        out.dirty = false;
    }

    // Resolves an auctioneer's raw faction-template id into the 3-value AH
    // bucket (Alliance=2, Horde=6, Neutral=7). Two-side-interaction collapses
    // to Neutral and matches how AuctionHouseMgr chooses the AuctionHouseObject.
    [[nodiscard]] static uint8 ResolveAhFactionBucket(uint32 auctioneerFactionTemplate)
    {
        AuctionHouseEntry const* entry =
            AuctionHouseMgr::GetAuctionHouseEntryFromFactionTemplate(auctioneerFactionTemplate);
        if (!entry)
            return static_cast<uint8>(AuctionHouseId::Neutral);
        return static_cast<uint8>(entry->houseId);
    }

    [[nodiscard]] PlayerbotAuctionItemPolicy MakeDefaultPolicy() const
    {
        PlayerbotAuctionItemPolicy policy;
        policy.buyoutMinPct = std::max<uint32>(100, sPlayerbotAIConfig.auctionHouseBuyoutMinPct);
        policy.buyoutMaxPct = std::max<uint32>(policy.buyoutMinPct, sPlayerbotAIConfig.auctionHouseBuyoutMaxPct);
        policy.undercutChance = std::min<uint32>(100, sPlayerbotAIConfig.auctionHouseUndercutChance);
        return policy;
    }

    [[nodiscard]] bool TableExists(char const* tableName) const
    {
        std::string const dbName = PlayerbotsDatabase.GetConnectionInfo()->database;
        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT EXISTS(SELECT 1 FROM information_schema.tables WHERE table_schema = '{}' "
            "AND table_name = '{}')",
            dbName, tableName);
        if (!result)
            return false;
        return result->Fetch()[0].Get<uint32>() != 0;
    }

    void LoadPolicies()
    {
        std::unique_lock<std::shared_mutex> guard(_mtx);
        _policies.clear();

        if (!TableExists("playerbots_auction_item_policy"))
        {
            LOG_WARN("playerbots",
                "playerbots_auction_item_policy table not found. Using built-in auction defaults.");
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

    void LoadPriceCache()
    {
        std::unique_lock<std::shared_mutex> guard(_mtx);
        _prices.clear();

        if (!TableExists("playerbots_auction_price_cache"))
        {
            LOG_WARN("playerbots",
                "playerbots_auction_price_cache table not found. Cache will populate lazily.");
            return;
        }

        // Startup prune: drop rows not touched in the last 7 days. Old history
        // biases pricing toward obsolete markets and inflates the load query.
        static constexpr uint32 kPruneAgeSeconds = 7 * 24 * 60 * 60;
        PlayerbotsDatabase.Execute(
            "DELETE FROM playerbots_auction_price_cache WHERE updated_at < UNIX_TIMESTAMP() - {}",
            kPruneAgeSeconds);

        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT item_entry, ah_faction, updated_at, samples FROM playerbots_auction_price_cache");

        if (!result)
        {
            LOG_INFO("playerbots", "Loaded 0 playerbots auction price cache rows.");
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            PlayerbotAuctionPriceKey key{fields[0].Get<uint32>(), fields[1].Get<uint8>()};
            CachedPriceHistory history;
            UnpackHistory(fields[3].Get<std::string>(), history);
            _prices[key] = std::move(history);
        } while (result->NextRow());

        LOG_INFO("playerbots", "Loaded {} playerbots auction price cache rows.", _prices.size());
    }

    mutable std::shared_mutex _mtx;
    std::unordered_map<uint32, PlayerbotAuctionItemPolicy> _policies;
    std::unordered_map<PlayerbotAuctionPriceKey, CachedPriceHistory,
        PlayerbotAuctionPriceKeyHash> _prices;
};

#define sBotAHUtil BotAHUtil::instance()

#endif
