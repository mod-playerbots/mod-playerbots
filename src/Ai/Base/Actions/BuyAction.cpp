/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BuyAction.h"

#include "BudgetValues.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemVisitors.h"
#include "Log.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"

bool BuyAction::Execute(Event event)
{
    bool buyUseful = false;
    ItemIds itemIds;
    std::string const link = event.getParam();

    if (link == "vendor")
        buyUseful = true;
    else
    {
        itemIds = chat->parseItems(link);
    }

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    bool vendored = false;
    bool result = false;
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        vendored = true;

        if (buyUseful)
        {
            // Items are evaluated from high-level to low level.
            // For each item the bot checks again if an item is usefull.
            // Bot will buy until no usefull items are left.

            VendorItemData const* tItems = pCreature->GetVendorItems();
            if (!tItems)
                continue;

            VendorItemList m_items_sorted = tItems->m_items;

            m_items_sorted.erase(std::remove_if(m_items_sorted.begin(), m_items_sorted.end(),
                                                [](VendorItem* i)
                                                {
                                                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item);
                                                    return !proto;
                                                }),
                                 m_items_sorted.end());

            if (m_items_sorted.empty())
                continue;

            StatsWeightCalculator calculator(bot);
            calculator.SetItemSetBonus(false);
            calculator.SetOverflowPenalty(false);

            std::sort(m_items_sorted.begin(), m_items_sorted.end(),
                [&calculator](VendorItem* i, VendorItem* j)
                {
                    ItemTemplate const* item1 = sObjectMgr->GetItemTemplate(i->item);
                    ItemTemplate const* item2 = sObjectMgr->GetItemTemplate(j->item);

                    if (!item1 || !item2)
                        return false;

                    float score1 = calculator.CalculateItem(item1->ItemId);
                    float score2 = calculator.CalculateItem(item2->ItemId);

                    // Fallback to itemlevel if either score is 0
                    if (score1 == 0 || score2 == 0)
                    {
                        score1 = item1->ItemLevel;
                        score2 = item2->ItemLevel;
                    }
                    return score1 > score2; // Sort in descending order (highest score first)
                });

            std::unordered_map<uint32, float> bestPurchasedItemScore;  // Track best item score per InventoryType

            for (auto& tItem : m_items_sorted)
            {
                uint32 maxPurchases = 1;  // Default to buying once
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tItem->item);
                if (!proto)
                    continue;

                if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_PROJECTILE)
                {
                    maxPurchases = 10;  // Allow up to 10 purchases if it's a consumable or projectile
                }

                for (uint32 i = 0; i < maxPurchases; i++)
                {
                    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", tItem->item);

                    uint32 invType = proto->InventoryType;

                    // Calculate item score
                    float newScore = calculator.CalculateItem(proto->ItemId);

                    // Skip if we already bought a better item for this slot
                    if (bestPurchasedItemScore.find(invType) != bestPurchasedItemScore.end() &&
                        bestPurchasedItemScore[invType] > newScore)
                    {
                        break;  // Skip lower-scoring items
                    }

                    // Check the bot's currently equipped item for this slot
                    uint8 dstSlot = botAI->FindEquipSlot(proto, NULL_SLOT, true);
                    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);

                    float oldScore = 0.0f;
                    if (oldItem)
                    {
                        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
                        if (oldItemProto)
                            oldScore = calculator.CalculateItem(oldItemProto->ItemId);
                    }

                    // Skip if the bot already has a better or equal item equipped
                    if (oldScore > newScore)
                        break;

                    uint32 price = proto->BuyPrice;
                    price = uint32(floor(price * bot->GetReputationPriceDiscount(pCreature)));

                    NeedMoneyFor needMoneyFor = NeedMoneyFor::none;
                    switch (usage)
                    {
                        case ITEM_USAGE_REPLACE:
                        case ITEM_USAGE_EQUIP:
                        case ITEM_USAGE_BAD_EQUIP:
                        case ITEM_USAGE_BROKEN_EQUIP:
                            needMoneyFor = NeedMoneyFor::gear;
                            break;
                        case ITEM_USAGE_AMMO:
                            needMoneyFor = NeedMoneyFor::ammo;
                            break;
                        case ITEM_USAGE_QUEST:
                            needMoneyFor = NeedMoneyFor::anything;
                            break;
                        case ITEM_USAGE_USE:
                            needMoneyFor = NeedMoneyFor::consumables;
                            break;
                        case ITEM_USAGE_SKILL:
                            needMoneyFor = NeedMoneyFor::tradeskill;
                            break;
                        default:
                            break;
                    }

                    if (needMoneyFor == NeedMoneyFor::none)
                        break;

                    if (AI_VALUE2(uint32, "free money for", uint32(needMoneyFor)) < price)
                        break;

                    if (!BuyItem(tItems, vendorguid, proto))
                        break;

                    // Store the best item score per InventoryType
                    bestPurchasedItemScore[invType] = newScore;

                    if (needMoneyFor == NeedMoneyFor::gear)
                    {
                        botAI->DoSpecificAction("equip upgrades packet action");
                    }
                }
            }
        }
        else
        {
            if (itemIds.empty())
                return false;

            for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
            {
                uint32 itemId = *i;
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                result |= BuyItem(pCreature->GetVendorItems(), vendorguid, proto);

                if (!result)
                {
                    std::ostringstream out;
                    out << "Nobody sells " << ChatHelper::FormatItem(proto) << " nearby";
                    botAI->TellMaster(out.str());
                    continue;
                }

                ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemId);
                if (usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_EQUIP ||
                    usage == ITEM_USAGE_BAD_EQUIP || usage == ITEM_USAGE_BROKEN_EQUIP)
                {
                    botAI->DoSpecificAction("equip upgrades packet action");
                    break;
                }
            }
        }
    }

    if (!vendored)
    {
        botAI->TellError("There are no vendors nearby");
        return false;
    }

    return true;
}


bool BuyAction::BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto)
{
    if (!tItems || !proto)
        return false;

    uint32 itemId = proto->ItemId;
    uint32 oldCount = bot->GetItemCount(itemId, false);

    for (uint32 slot = 0; slot < tItems->GetItemCount(); ++slot)
    {
        if (tItems->GetItem(slot)->item != itemId)
            continue;

        uint32 botMoney = bot->GetMoney();
        if (botAI->HasCheat(BotCheatMask::gold))
            bot->SetMoney(10000000);

        bot->BuyItemFromVendorSlot(vendorguid, slot, itemId, 1, NULL_BAG, NULL_SLOT);

        if (botAI->HasCheat(BotCheatMask::gold))
            bot->SetMoney(botMoney);

        uint32 newCount = bot->GetItemCount(itemId, false);
        if (newCount > oldCount)
        {
            std::ostringstream out;
            out << "Buying " << ChatHelper::FormatItem(proto);
            botAI->TellMaster(out.str());
            return true;
        }

        return false;
    }

    return false;
}

// === AH Buy Action — receives SMSG_AUCTION_LIST_RESULT, scores top items, bids/buys ===
bool AhBuyAction::ParseAuctionPacket(WorldPacket& p, uint32 gearBudget, std::vector<AhItem>& candidates)
{
    uint32 count;
    p >> count;

    if (!count)
        return false;

    candidates.reserve(std::min(count, uint32(50)));

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

        if (ownerGuid == bot->GetGUID())
            continue;

        uint32 bidPrice = minOutbid ? minOutbid : startbid;
        bool canBuyout = buyout && buyout <= gearBudget;
        bool canBid = bidPrice && bidPrice <= gearBudget;
        if (!canBuyout && !canBid)
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
        if (!proto || proto->RequiredLevel > bot->GetLevel())
            continue;

        if (bot->BotCanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        candidates.push_back({auctionId, itemEntry, buyout, bidPrice, itemCount});
    }

    return !candidates.empty();
}

bool AhBuyAction::BuyBestCandidate(std::vector<AhItem>& candidates)
{
    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    AhItem const* bestCandidate = nullptr;
    float bestScore = 0.0f;
    uint32 evaluated = 0;

    for (AhItem const& candidate : candidates)
    {
        //evaluate only top 10 items sent.
        if (++evaluated > 10)
            break;

        float score = calculator.CalculateItem(candidate.itemEntry);
        if (score > bestScore)
        {
            bestScore = score;
            bestCandidate = &candidate;
        }
    }

    if (!bestCandidate || bestScore <= 0.0f)
        return false;

    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    ObjectGuid auctioneerGuid;
    for (ObjectGuid const& guid : npcs)
    {
        if (bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER))
        {
            auctioneerGuid = guid;
            break;
        }
    }

    if (auctioneerGuid.IsEmpty())
        return false;

    // Buyout if affordable, otherwise bid
    uint32 gearBudget = AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::gear));
    uint32 price = (bestCandidate->buyout && bestCandidate->buyout <= gearBudget)
        ? bestCandidate->buyout
        : bestCandidate->bidPrice;

    uint32 botMoney = bot->GetMoney();

    WorldPacket bidPacket(CMSG_AUCTION_PLACE_BID);
    bidPacket << auctioneerGuid;
    bidPacket << bestCandidate->auctionId;
    bidPacket << price;

    bot->GetSession()->HandleAuctionPlaceBid(bidPacket);

    if (botAI->HasCheat(BotCheatMask::gold))
        bot->SetMoney(botMoney);

    ItemTemplate const* boughtProto = sObjectMgr->GetItemTemplate(bestCandidate->itemEntry);
    LOG_DEBUG("playerbots", "[AH Buy] Bot {} {} {} for {}",
              bot->GetName(),
              price == bestCandidate->buyout ? "bought" : "bid on",
              boughtProto ? boughtProto->Name1 : std::to_string(bestCandidate->itemEntry),
              price);

    return true;
}

bool AhBuyAction::Execute(Event event)
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

    uint32 gearBudget = AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::gear));
    if (!gearBudget)
    {
        LOG_DEBUG("playerbots", "[AH Buy] Bot {} no gear budget, skipping", bot->GetName());
        return false;
    }

    std::vector<AhItem> candidates;
    if (!ParseAuctionPacket(p, gearBudget, candidates))
    {
        LOG_DEBUG("playerbots", "[AH Buy] Bot {} no viable candidates from AH packet (budget={})",
                  bot->GetName(), gearBudget);
        return false;
    }

    LOG_DEBUG("playerbots", "[AH Buy] Bot {} found {} candidates (budget={})",
              bot->GetName(), candidates.size(), gearBudget);
    return BuyBestCandidate(candidates);
}
