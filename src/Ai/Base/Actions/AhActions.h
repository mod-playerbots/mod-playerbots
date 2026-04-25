/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AHACTIONS_H
#define _PLAYERBOT_AHACTIONS_H

#include <ctime>
#include <unordered_map>

#include "Action.h"
#include "Common.h"

struct PlayerbotAuctionItemPolicy;
struct PlayerbotAuctionMarketSnapshot;

class Item;
class ObjectGuid;
class PlayerbotAI;

struct ItemTemplate;

// Per-item/slot state in the sell/buy action loop.
enum class AhStatus : uint8
{
    Idle,
    PendingCheck,
    Watch,
    Complete,
    Failed,
};

struct AhItemState
{
    AhStatus status = AhStatus::Idle;
    time_t changedAt = 0;
    time_t retryAfter = 0;
    uint32 auctionId = 0;
};

constexpr time_t AH_PENDING_CHECK_TIMEOUT_SECONDS = 10;   // drop stuck PendingCheck
constexpr time_t AH_FAILED_BACKOFF_SECONDS = MINUTE;      // sell cooldown after Failed
constexpr time_t AH_BUY_SLOT_COOLDOWN_SECONDS = HOUR;     // buy-slot cooldown after a bid or empty result

// Auction-house candidate shortlist entry produced by AhSearchResultAction's
// parse phase and consumed by the bid phase.
struct AhItem
{
    uint32 auctionId;
    uint32 itemEntry;
    uint32 buyout;
    uint32 bidPrice;
    uint32 itemCount;
};

using AhListMap = std::unordered_map<uint32, AhItemState>;

class AhSellAction : public Action
{
public:
    AhSellAction(PlayerbotAI* botAI) : Action(botAI, "ah sell") {}

    bool Execute(Event event) override;

private:
    bool PostAuctionSell(Item* item, ItemTemplate const* proto, ObjectGuid const& auctioneerGuid, uint32 auctioneerFaction,
        PlayerbotAuctionItemPolicy const& policy, uint32 itemCount, uint32 unitPrice,
        PlayerbotAuctionMarketSnapshot const& marketSnapshot);
    bool QueueMarketQuery(ItemTemplate const* proto, ObjectGuid const& auctioneerGuid);
};

class AhSearchResultAction : public Action
{
public:
    AhSearchResultAction(PlayerbotAI* botAI) : Action(botAI, "ah search result") {}

    bool Execute(Event event) override;

private:
    bool ParseAuctionPacket(WorldPacket& p, uint32 gearBudget, uint32 auctioneerFaction, std::vector<AhItem>& buyCandidates);
    AhItem const* PickBestCandidate(std::vector<AhItem> const& buyCandidates);
    bool PostAuctionBid(AhItem const& candidate, ObjectGuid auctioneerGuid, uint32 auctioneerFaction, uint32 gearBudget);
};

class AhCommandResultAction : public Action
{
public:
    AhCommandResultAction(PlayerbotAI* botAI) : Action(botAI, "ah command result") {}

    bool Execute(Event event) override;
};


class AhBidderNotificationAction : public Action
{
public:
    AhBidderNotificationAction(PlayerbotAI* botAI) : Action(botAI, "ah bidder notification") {}

    bool Execute(Event event) override;
};

#endif
