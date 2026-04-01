/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SellAction.h"

#include "AuctionHouseBotHelper.h"
#include "Db/PlayerbotSpellRepository.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Log.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotAuctionHouseUtil.h"
#include "PlayerbotOperations.h"
#include "Playerbots.h"

#include "PlayerbotWorldThreadProcessor.h"

class SellItemsVisitor : public IterateItemsVisitor
{
public:
    SellItemsVisitor(SellAction* action) : IterateItemsVisitor(), action(action) {}

    bool Visit(Item* item) override
    {
        action->Sell(item);
        return true;
    }

private:
    SellAction* action;
};

class SellGrayItemsVisitor : public SellItemsVisitor
{
public:
    SellGrayItemsVisitor(SellAction* action) : SellItemsVisitor(action) {}

    bool Visit(Item* item) override
    {
        if (item->GetTemplate()->Quality != ITEM_QUALITY_POOR)
            return true;

        return SellItemsVisitor::Visit(item);
    }
};

class SellVendorItemsVisitor : public SellItemsVisitor
{
public:
    SellVendorItemsVisitor(SellAction* action, AiObjectContext* con) : SellItemsVisitor(action) { context = con; }

    AiObjectContext* context;

    bool Visit(Item* item) override
    {
        ItemUsage usage = context->GetValue<ItemUsage>("item usage", item->GetEntry())->Get();
        if (usage != ITEM_USAGE_VENDOR)
            return true;

        return SellItemsVisitor::Visit(item);
    }
};

bool SellAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "gray" || text == "*")
    {
        SellGrayItemsVisitor visitor(this);
        IterateItems(&visitor);
        return true;
    }

    if (text == "vendor")
    {
        SellVendorItemsVisitor visitor(this, context);
        IterateItems(&visitor);
        return true;
    }

    if (text != "")
    {
        std::vector<Item*> items = parseItems(text, ITERATE_ITEMS_IN_BAGS);
        for (Item* item : items)
        {
            Sell(item);
        }
        return true;
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "auction_sell_usage_error",
        "Usage: s gray/*/vendor/auction/[item link]", {}));
    return false;
}

void SellAction::Sell(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    for (Item* item : items)
    {
        Sell(item);
    }
}

void SellAction::Sell(Item* item)
{
    if (!item)
        return;

    std::ostringstream out;

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    for (ObjectGuid const vendorguid : vendors)
    {
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        ObjectGuid itemguid = item->GetGUID();
        uint32 count = item->GetCount();

        uint32 botMoney = bot->GetMoney();

        WorldPacket p(CMSG_SELL_ITEM);
        p << vendorguid << itemguid << count;

        WorldPackets::Item::SellItem nicePacket(std::move(p));
        nicePacket.Read();
        bot->GetSession()->HandleSellItemOpcode(nicePacket);

        if (botAI->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(botMoney);
        }

        out << "Selling " << chat->FormatItem(item->GetTemplate());
        botAI->TellMaster(out);

        bot->PlayDistanceSound(120);
        break;
    }
}

// === AH Sell Action — posts one item per call from the cached sell list ===
bool AhSellAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig.enableAuctionHouseBotting)
        return false;

    std::vector<uint32> sellList = AI_VALUE(std::vector<uint32>, "ah sell list");
    if (sellList.empty())
        return false;

    uint32 entry = sellList.front();

    Item* item = bot->GetItemByEntry(entry);
    if (!item)
        return false;

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto || !item->CanBeTraded())
        return false;

    if (proto->Quality == ITEM_QUALITY_POOR)
        return false;

    if (proto->Class == ITEM_CLASS_PROJECTILE)
        return false;

    if (proto->Quality == ITEM_QUALITY_NORMAL && !IsAuctionHouseMaterial(proto))
        return false;

    if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM)
        return false;

    if (sPlayerbotAIConfig.IsInAuctionHouseExcludedItemList(entry))
        return false;

    if (PlayerbotSpellRepository::Instance().IsItemBuyable(entry) &&
        ItemUsageValue::IsSpellReagentItem(proto))
        return false;

    PlayerbotAuctionItemPolicy policy = sPlayerbotAHUtil.GetPolicy(entry);
    if (!policy.sellable)
        return false;

    if (!policy.chanceToSell || urand(1, 100) > policy.chanceToSell)
        return false;

    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    ObjectGuid auctioneerGuid;
    if (!HasNearbyAuctioneer(bot, npcs, auctioneerGuid))
    {
        LOG_DEBUG("playerbots", "{}: cannot post item {} to auction house - no nearby auctioneer",
            bot->GetName(), proto->ItemId);
        return false;
    }
    return false;

    uint32 itemCount = GetAuctionStackCount(item, policy);
    if (!itemCount)
        return false;

    auto sellOp = std::make_unique<AuctionSellOperation>(
        bot->GetGUID(), auctioneerGuid, item->GetGUID(), proto->ItemId,
        itemCount, policy);

    bool queued = PlayerbotWorldThreadProcessor::instance().QueueOperation(
        std::move(sellOp));

    if (queued)
    {
        LOG_DEBUG("playerbots", "[AH Sell] Bot {} posted {} x{} to auction house",
                  bot->GetName(), proto->ItemId, itemCount);
    }

    return queued;
}
