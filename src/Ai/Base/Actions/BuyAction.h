/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BUYACTION_H
#define _PLAYERBOT_BUYACTION_H

#include "InventoryAction.h"

class FindItemVisitor;
class ObjectGuid;
class Item;
class Player;
class PlayerbotAI;

struct ItemTemplate;
struct VendorItemData;

class BuyAction : public InventoryAction
{
public:
    BuyAction(PlayerbotAI* botAI) : InventoryAction(botAI, "buy") {}

    bool Execute(Event event) override;

private:
    bool BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto);
    bool TradeItem(FindItemVisitor* visitor, int8 slot);
    bool TradeItem(Item const* item, int8 slot);
};

struct AhItem
{
    uint32 auctionId;
    uint32 itemEntry;
    uint32 buyout;
    uint32 bidPrice;
    uint32 itemCount;
};

class AhBuyAction : public Action
{
public:
    AhBuyAction(PlayerbotAI* botAI) : Action(botAI, "ah buy") {}

    bool Execute(Event event) override;

private:
    bool ParseAuctionPacket(WorldPacket& p, uint32 gearBudget, std::vector<AhItem>& candidates);
    bool BuyBestCandidate(std::vector<AhItem>& candidates);
};

#endif
