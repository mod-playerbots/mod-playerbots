/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AhActions.h"

#include <unordered_map>

#include "BudgetValues.h"
#include "Event.h"
#include "PlayerbotUtils.h"
#include "ItemPackets.h"
#include "ItemUsageValue.h"
#include "Log.h"
#include "PlayerbotAuctionHouseUtil.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"

bool AhSellAction::PostAuctionSell(Item* item, ItemTemplate const* proto, ObjectGuid const& auctioneerGuid,
    uint32 auctioneerFaction, PlayerbotAuctionItemPolicy const& policy, uint32 itemCount, uint32 unitPrice,
    PlayerbotAuctionMarketSnapshot const& marketSnapshot)
{
    if (policy.undercutChance && urand(1, 100) <= policy.undercutChance)
    {
        uint32 minPct = std::max<uint32>(100, sPlayerbotAIConfig.auctionHouseUndercutMinPct);
        uint32 maxPct = std::max<uint32>(minPct, sPlayerbotAIConfig.auctionHouseUndercutMaxPct);
        uint32 anchorUnitPrice = marketSnapshot.HasData() ? marketSnapshot.minUnitBuyout : unitPrice;
        unitPrice = BotAuctionUtils::RoundAuctionPrice(
            double(anchorUnitPrice) * 100.0 / urand(minPct, maxPct));
    }

    uint32 startBid = std::max<uint32>(sPlayerbotAIConfig.auctionHouseMinBidPrice,
        BotAuctionUtils::RoundAuctionPrice(double(itemCount) * unitPrice * policy.minBidPct / 100.0));
    uint32 minBuyoutPct = std::max<uint32>(100, policy.buyoutMinPct);
    uint32 maxBuyoutPct = std::max<uint32>(minBuyoutPct, policy.buyoutMaxPct);
    uint32 buyout = BotAuctionUtils::RoundAuctionPrice(double(startBid) * urand(minBuyoutPct, maxBuyoutPct) / 100.0);
    if (buyout <= startBid)
        buyout = startBid + 1;

    uint32 etime = uint32(12 * HOUR / MINUTE);

    WorldPacket packet(CMSG_AUCTION_SELL_ITEM);
    packet << auctioneerGuid;
    packet << uint32(1);
    packet << item->GetGUID();
    packet << itemCount;
    packet << startBid;
    packet << buyout;
    packet << etime;

    auto op = std::make_unique<AuctionPacketOperation>(
        bot->GetGUID(), auctioneerGuid, std::move(packet));

    if (!PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(op)))
        return false;

    // Invalidate cache so the next pricing pass re-queries and includes this
    // freshly posted listing.
    sPlayerbotAuctionHouseUtil.InvalidateMarketSnapshot(proto->ItemId, auctioneerFaction);
    return true;
}

bool AhSellAction::QueueMarketQuery(ItemTemplate const* proto, ObjectGuid const& auctioneerGuid)
{
    WorldPacket queryPacket(CMSG_AUCTION_LIST_ITEMS);
    queryPacket << auctioneerGuid;
    queryPacket << uint32(0);                    // listfrom (page 0)
    queryPacket << std::string(proto->Name1);    // name filter
    queryPacket << uint8(0);                     // levelMin
    queryPacket << uint8(0);                     // levelMax
    queryPacket << uint32(0xFFFFFFFF);           // inventoryType (any)
    queryPacket << uint32(0xFFFFFFFF);           // itemClass (any)
    queryPacket << uint32(0xFFFFFFFF);           // itemSubClass (any)
    queryPacket << uint32(0xFFFFFFFF);           // quality (any)
    queryPacket << uint8(0);                     // usable only = false
    queryPacket << uint8(0);                     // getAll = false
    queryPacket << uint8(0);                     // no sort keys

    auto queryOp = std::make_unique<AuctionPacketOperation>(
        bot->GetGUID(), auctioneerGuid, std::move(queryPacket));
    return PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(queryOp));
}

bool AhSellAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    auto& sellList = AI_VALUE(AhListMap&, "ah sell list");
    if (sellList.empty())
        return false;

    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    Creature* auctioneer = ai::npc::FindNpcByFlag(bot, UNIT_NPC_FLAG_AUCTIONEER, npcs);
    if (!auctioneer)
        return false;

    ObjectGuid auctioneerGuid = auctioneer->GetGUID();
    uint32 auctioneerFaction = auctioneer->GetFaction();
    time_t now = time(nullptr);

    // Scan-pick: first Complete, else first Idle+cache-hit, else first Idle.
    uint32 readyItem = 0;
    uint32 idleItem = 0;

    for (auto& kv : sellList)
    {
        AhItemState& st = kv.second;

        if (st.status == AhStatus::Failed && now >= st.retryAfter)
        {
            st.status = AhStatus::Idle;
            st.retryAfter = 0;
        }

        if (st.status == AhStatus::Complete)
        {
            readyItem = kv.first;
            break;
        }

        if (st.status == AhStatus::PendingCheck)
        {
            // Scan response never arrived. Tombstone the cache so the next
            // pass posts at template price with default purchasing rules
            // instead of looping back into NoData.
            if (now - st.changedAt > AH_PENDING_CHECK_TIMEOUT_SECONDS)
            {
                sPlayerbotAuctionHouseUtil.StoreMarketSnapshot(
                    kv.first, auctioneerFaction, PlayerbotAuctionMarketSnapshot{});
                st.status = AhStatus::Complete;
                st.changedAt = now;
            }
            continue;
        }

        if (st.status == AhStatus::Idle)
        {
            if (sPlayerbotAuctionHouseUtil.GetMarketSnapshot(kv.first, auctioneerFaction))
            {
                readyItem = kv.first;
                break;
            }
            if (!idleItem)
                idleItem = kv.first;
        }
    }

    if (!readyItem && !idleItem)
        return false;

    uint32 entry = readyItem ? readyItem : idleItem;
    AhItemState& st = sellList[entry];

    Item* item = bot->GetItemByEntry(entry);
    ItemTemplate const* proto = item ? item->GetTemplate() : nullptr;
    if (!item || !proto)
    {
        sellList.erase(entry);
        return false;  // inventory moved — next reconcile drops the entry.
    }
    PlayerbotAuctionItemPolicy policy = sPlayerbotAuctionHouseUtil.GetPolicy(entry);
    if (!policy.chanceToSell || urand(1, 100) > policy.chanceToSell)
        return false;

    uint32 itemCount = BotAuctionUtils::GetAuctionStackCount(item, policy);
    if (!itemCount)
        return false;

    uint32 unitPrice = 0;
    PlayerbotAuctionMarketSnapshot marketSnapshot;
    BotAuctionUtils::AuctionPriceStatus priceStatus =
        sPlayerbotAuctionHouseUtil.GetAuctionPrice(bot, proto, auctioneerFaction, unitPrice, &marketSnapshot);

    if (priceStatus == BotAuctionUtils::AuctionPriceStatus::NoData)
    {
        if (!QueueMarketQuery(proto, auctioneerGuid))
            return false;

        st.status = AhStatus::PendingCheck;
        st.changedAt = now;
        st.retryAfter = 0;
        return false;
    }

    // Blended or TemplateOnly — post and reset state. Next reconcile drops
    // the entry if the item has left inventory.
    bool posted = PostAuctionSell(item, proto, auctioneerGuid, auctioneerFaction,
        policy, itemCount, unitPrice, marketSnapshot);
    st = AhItemState{};
    return posted;
}

bool AhSearchResultAction::ParseAuctionPacket(WorldPacket& p, uint32 gearBudget, uint32 auctioneerFaction,
                                              std::vector<AhItem>& buyCandidates)
{
    uint32 count;
    p >> count;

    auto& sellList = AI_VALUE(AhListMap&, "ah sell list");
    auto& buyList = AI_VALUE(AhListMap&, "ah buy list");

    // Empty responses are not tombstoned here: with multiple sell-side queries
    // potentially in flight, we cannot tell which item this empty response is
    // for. The PendingCheck timeout in AhSellAction::Execute handles that case.
    if (!count)
        return false;

    buyCandidates.reserve(std::min(count, uint32(50)));

    // Per-item aggregation for cache write-through.
    struct Aggregate { uint32 minUnit = 0; uint64 totalUnit = 0; uint32 sampleCount = 0; };
    std::unordered_map<uint32, Aggregate> perItem;

    for (uint32 i = 0; i < count; ++i)
    {
        uint32 auctionId, itemEntry;
        p >> auctionId >> itemEntry;

        // Skip enchant data (MAX_INSPECTED_ENCHANTMENT_SLOT * 3 uint32s)
        for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
        {
            uint32 enchantId, enchantDuration, enchantCharges;
            p >> enchantId >> enchantDuration >> enchantCharges;
        }

        int32 randomPropertyId;
        uint32 suffixFactor, itemCount;
        int32 spellCharges;
        uint32 flags;
        ObjectGuid ownerGuid;
        uint32 startbid, minOutbid, buyout, timeLeft;
        ObjectGuid bidderGuid;
        uint32 currentBid;

        p >> randomPropertyId >> suffixFactor >> itemCount >> spellCharges >> flags;
        p >> ownerGuid >> startbid >> minOutbid >> buyout >> timeLeft;
        p >> bidderGuid >> currentBid;

        if (buyout && itemCount)
        {
            uint32 unitBuyout = std::max<uint32>(1, buyout / itemCount);
            Aggregate& aggregate = perItem[itemEntry];
            if (!aggregate.minUnit || unitBuyout < aggregate.minUnit)
                aggregate.minUnit = unitBuyout;

            aggregate.totalUnit += unitBuyout;
            ++aggregate.sampleCount;
        }

        if (ownerGuid == bot->GetGUID())
            continue;

        // Skip items we're about to list ourselves (PendingCheck/Complete).
        auto stateItr = sellList.find(itemEntry);
        if (stateItr != sellList.end() && stateItr->second.status != AhStatus::Idle)
            continue;

        uint32 bidPrice = minOutbid ? minOutbid : startbid;
        bool canBuyout = buyout && buyout <= gearBudget;
        bool canBid = bidPrice && bidPrice <= gearBudget;
        if (!canBuyout && !canBid)
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
        if (!proto || proto->RequiredLevel > bot->GetLevel())
            continue;

        uint8 dstSlot = botAI->FindEquipSlot(proto, NULL_SLOT, true);
        if (dstSlot == NULL_SLOT || !buyList.count(dstSlot))
            continue;

        if (bot->BotCanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        buyCandidates.push_back({auctionId, itemEntry, buyout, bidPrice, itemCount});
    }

    // Write-through to cache.
    time_t now = time(nullptr);
    for (auto const& keyValue : perItem)
    {
        PlayerbotAuctionMarketSnapshot snapshot;
        snapshot.minUnitBuyout = keyValue.second.minUnit;
        snapshot.avgUnitBuyout = keyValue.second.sampleCount
            ? uint32(keyValue.second.totalUnit / keyValue.second.sampleCount) : 0;
        snapshot.sampleCount = keyValue.second.sampleCount;
        sPlayerbotAuctionHouseUtil.StoreMarketSnapshot(keyValue.first, auctioneerFaction, snapshot);

        auto stateItr = sellList.find(keyValue.first);
        if (stateItr != sellList.end() && stateItr->second.status == AhStatus::PendingCheck)
        {
            stateItr->second.status = AhStatus::Complete;
            stateItr->second.changedAt = now;
        }
    }

    return !buyCandidates.empty();
}

AhItem const* AhSearchResultAction::PickBestCandidate(std::vector<AhItem> const& buyCandidates)
{
    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    AhItem const* bestCandidate = nullptr;
    float bestScore = 0.0f;
    uint32 evaluated = 0;

    for (AhItem const& candidate : buyCandidates)
    {
        // Evaluate only top 10 items sent.
        if (++evaluated > 10)
            break;

        float score = calculator.CalculateItem(candidate.itemEntry);
        if (score > bestScore)
        {
            bestScore = score;
            bestCandidate = &candidate;
        }
    }

    return (bestCandidate && bestScore > 0.0f) ? bestCandidate : nullptr;
}

bool AhSearchResultAction::PostAuctionBid(AhItem const& candidate, ObjectGuid auctioneerGuid,
                                          uint32 auctioneerFaction, uint32 gearBudget)
{
    if (auctioneerGuid.IsEmpty())
        return false;

    // Buyout if affordable, otherwise bid at the minimum outbid price.
    uint32 price = (candidate.buyout && candidate.buyout <= gearBudget)
        ? candidate.buyout
        : candidate.bidPrice;

    WorldPacket bidPacket(CMSG_AUCTION_PLACE_BID);
    bidPacket << auctioneerGuid;
    bidPacket << candidate.auctionId;
    bidPacket << price;

    auto op = std::make_unique<AuctionPacketOperation>(
        bot->GetGUID(), auctioneerGuid, std::move(bidPacket));
    if (!PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(op)))
        return false;

    sPlayerbotAuctionHouseUtil.InvalidateMarketSnapshot(candidate.itemEntry, auctioneerFaction);

    ItemTemplate const* boughtProto = sObjectMgr->GetItemTemplate(candidate.itemEntry);
    LOG_DEBUG("playerbots", "[AH Buy] Bot {} {} {} for {}",
              bot->GetName(),
              price == candidate.buyout ? "bought" : "bid on",
              boughtProto ? boughtProto->Name1 : std::to_string(candidate.itemEntry),
              price);

    return true;
}

bool AhSearchResultAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    WorldPacket p(event.getPacket());
    if (p.empty())
    {
        LOG_DEBUG("playerbots", "[AH Buy] Bot {} received empty AH packet", bot->GetName());
        return false;
    }

    LOG_DEBUG("playerbots", "[AH Buy] Bot {} received AH packet (opcode={}, size={})",
              bot->GetName(), p.GetOpcode(), p.size());

    p.rpos(0);

    // Resolve auctioneer once so ParseAuctionPacket + PostAuctionBid agree on
    // which AH bucket we're dealing with.
    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    Creature* auctioneer = ai::npc::FindNpcByFlag(bot, UNIT_NPC_FLAG_AUCTIONEER, npcs);
    ObjectGuid auctioneerGuid = auctioneer ? auctioneer->GetGUID() : ObjectGuid::Empty;
    uint32 auctioneerFaction = auctioneer ? auctioneer->GetFaction() : 0;

    uint32 gearBudget = AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::gear));

    // First lets parse the packet and add to cache and see if there is anything useful in the packet.
    std::vector<AhItem> buyCandidates;
    bool haveCandidates = ParseAuctionPacket(p, gearBudget, auctioneerFaction, buyCandidates);

    auto& buyList = AI_VALUE(AhListMap&, "ah buy list");
    uint32 pendingSlot = 0xFF;
    for (auto& kv : buyList)
    {
        if (kv.second.status == AhStatus::PendingCheck)
        {
            pendingSlot = kv.first;
            break;
        }
    }

    if (pendingSlot == 0xFF)
        return false;

    if (!haveCandidates)
    {
        time_t now = time(nullptr);
        AhItemState& st = buyList[pendingSlot];
        st.status = AhStatus::Failed;
        st.auctionId = 0;
        st.changedAt = now;
        st.retryAfter = now + AH_BUY_SLOT_COOLDOWN_SECONDS;
        return true;
    }

    AhItem const* best = nullptr;
    bool bidPlaced = false;
    if (gearBudget && haveCandidates)
    {
        best = PickBestCandidate(buyCandidates);
        if (best)
            bidPlaced = PostAuctionBid(*best, auctioneerGuid, auctioneerFaction, gearBudget);
    }

    time_t now = time(nullptr);
    AhItemState& st = buyList[pendingSlot];
    st.changedAt = now;

    if (bidPlaced && best)
    {
        st.status = AhStatus::Watch;
        st.auctionId = best->auctionId;
        st.retryAfter = 0;
    }
    else
    {
        st.status = AhStatus::Failed;
        st.auctionId = 0;
        st.retryAfter = now + AH_BUY_SLOT_COOLDOWN_SECONDS;
    }

    LOG_DEBUG("playerbots", "[AH Buy] Bot {} slot {} => {} (budget={}, candidates={})",
              bot->GetName(), pendingSlot, bidPlaced ? "bid placed" : "no bid",
              gearBudget, buyCandidates.size());

    return bidPlaced;
}

bool AhCommandResultAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    WorldPacket p(event.getPacket());
    if (p.size() < 12)
        return false;
    p.rpos(0);

    uint32 auctionId, action, errorCode;
    p >> auctionId >> action >> errorCode;

    // Only bid results matter for buy-side state; sell/cancel errors are
    // logged at the core level and don't drive our state machine.
    constexpr uint32 AUCTION_PLACE_BID = 2;
    if (action != AUCTION_PLACE_BID || errorCode == 0)
    {
        LOG_DEBUG("playerbots", "[AH Cmd] Bot {} auctionId={} action={} err={}",
                  bot->GetName(), auctionId, action, errorCode);
        return true;
    }

    auto& buyList = AI_VALUE(AhListMap&, "ah buy list");
    for (auto& kv : buyList)
    {
        if (kv.second.status == AhStatus::Watch && kv.second.auctionId == auctionId)
        {
            kv.second.status = AhStatus::Idle;
            kv.second.auctionId = 0;
            kv.second.changedAt = time(nullptr);
            kv.second.retryAfter = 0;

            LOG_DEBUG("playerbots", "[AH Cmd] Bot {} bid failed (err={}) for slot {} (auctionId={}); reset to Idle",
                      bot->GetName(), errorCode, kv.first, auctionId);
            return true;
        }
    }

    return false;
}

// ============================================================================
// AhBidderNotificationAction — SMSG_AUCTION_BIDDER_NOTIFICATION
// ============================================================================
// Packet layout (AuctionHouseHandler.cpp:88):
//   uint32 location       (AH faction bucket)
//   uint32 auctionId
//   ObjectGuid bidder     (new top bidder; == bot on win, != bot on outbid)
//   uint32 bidSum
//   uint32 diff           (0 on win, non-zero on outbid — we use bidder GUID instead)
//   uint32 item_template

bool AhBidderNotificationAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    WorldPacket p(event.getPacket());
    if (p.empty())
        return false;
    p.rpos(0);

    uint32 location, auctionId;
    ObjectGuid bidder;
    p >> location >> auctionId >> bidder;

    auto& buyList = AI_VALUE(AhListMap&, "ah buy list");
    for (auto& kv : buyList)
    {
        if (kv.second.status != AhStatus::Watch || kv.second.auctionId != auctionId)
            continue;

        if (bidder == bot->GetGUID())
        {
            kv.second.status = AhStatus::Complete;
            kv.second.changedAt = time(nullptr);

            LOG_DEBUG("playerbots", "[AH Bidder] Bot {} won auctionId={} slot={}",
                      bot->GetName(), auctionId, kv.first);
        }
        else
        {
            kv.second.status = AhStatus::Idle;
            kv.second.auctionId = 0;
            kv.second.changedAt = time(nullptr);
            kv.second.retryAfter = 0;

            LOG_DEBUG("playerbots", "[AH Bidder] Bot {} outbid on auctionId={} slot={}; reset to Idle",
                      bot->GetName(), auctionId, kv.first);
        }
        return true;
    }

    return false;
}
