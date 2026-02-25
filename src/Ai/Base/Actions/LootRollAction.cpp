/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LootRollAction.h"

#include <string>
#include <vector>

#include "AiObjectContext.h"
#include "Event.h"
#include "Group.h"
#include "ItemUsageValue.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"

static inline int32 EncodeRandomEnchantParam(uint32 randomPropertyId, uint32 randomSuffixId)
{
    if (randomPropertyId)
        return static_cast<int32>(randomPropertyId);

    if (randomSuffixId)
        return -static_cast<int32>(randomSuffixId);

    return 0;
}

bool LootRollAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Roll*> const& rolls = group->GetRolls();
    for (Roll* const roll : rolls)
    {
        if (!roll)
            continue;

        // Avoid server crash, key may not exit for the bot on login
        auto it = roll->playerVote.find(bot->GetGUID());
        if (it != roll->playerVote.end() && it->second != NOT_EMITED_YET)
            continue;

        ObjectGuid guid = roll->itemGUID;
        uint32 itemId = roll->itemid;
        int32 randomProperty = EncodeRandomEnchantParam(roll->itemRandomPropId, roll->itemRandomSuffix);

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        std::string const itemUsageParam = ItemUsageValue::BuildItemUsageParam(itemId, randomProperty);
        ItemUsage usage = AI_VALUE2(ItemUsage, "loot usage", itemUsageParam);

        // Central smart-loot decision
        RollVote const vote = CalculateLootRollVote(bot, proto, randomProperty, usage, group);

        // Announce + send the roll vote (if ML/FFA => PASS)
        RollVote sent = vote;
        if (group->GetLootMethod() == MASTER_LOOT || group->GetLootMethod() == FREE_FOR_ALL)
            sent = PASS;

        group->CountRollVote(bot->GetGUID(), guid, sent);
        // One item at a time
        return true;
    }

    return false;
}

bool MasterLootRollAction::isUseful() { return !botAI->HasActivePlayerMaster(); }

bool MasterLootRollAction::Execute(Event event)
{
    Player* bot = QueryItemUsageAction::botAI->GetBot();

    WorldPacket p(event.getPacket());  // WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid creatureGuid;
    uint32 mapId;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    uint32 randomPropertyId;
    uint32 count;
    uint32 timeout;

    p.rpos(0);              // reset packet pointer
    p >> creatureGuid;      // creature guid what we're looting
    p >> mapId;             // 3.3.3 mapid
    p >> itemSlot;          // the itemEntryId for the item that shall be rolled for
    p >> itemId;            // the itemEntryId for the item that shall be rolled for
    p >> randomSuffix;      // randomSuffix
    p >> randomPropertyId;  // item random property ID
    p >> count;             // items in stack
    p >> timeout;           // the countdown time to choose "need" or "greed"

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Compute random property and usage, same pattern as LootRollAction::Execute
    int32 randomProperty = EncodeRandomEnchantParam(randomPropertyId, randomSuffix);

    std::string const itemUsageParam = ItemUsageValue::BuildItemUsageParam(itemId, randomProperty);
    ItemUsage usage = AI_VALUE2(ItemUsage, "loot usage", itemUsageParam);

    // 1) Token heuristic: ONLY NEED if the target slot is a likely upgrade
    RollVote vote = CalculateLootRollVote(bot, proto, randomProperty, usage, group);

    RollVote sent = vote;
    if (group->GetLootMethod() == MASTER_LOOT || group->GetLootMethod() == FREE_FOR_ALL)
        sent = PASS;

    group->CountRollVote(bot->GetGUID(), creatureGuid, sent);

    return true;
}

bool RollAction::Execute(Event event)
{
    std::string link = event.getParam();

    if (link.empty())
    {
        bot->DoRandomRoll(0, 100);
        return false;
    }
    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;
    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    std::string const itemUsageParam = ItemUsageValue::BuildItemUsageParam(itemId, 0);
    ItemUsage usage = AI_VALUE2(ItemUsage, "item upgrade", itemUsageParam);
    switch (proto->Class)
    {
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_ARMOR:
            if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
                bot->DoRandomRoll(0, 100);
    }

    return true;
}