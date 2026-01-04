/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Log.h"
#include "Player.h"

class QuestItemVisitor : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		const bool hasQuestForItem = player->HasQuestForItem(itemTemplate->ItemId, 0, true);

		LOG_ERROR("playerbots", "Has quest for item {}: {}", std::to_string(itemTemplate->ItemId), std::to_string(hasQuestForItem));

		return hasQuestForItem;
	}

	bool visit(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		if (itemTemplate->Class != ITEM_CLASS_QUEST)
		{
			LOG_ERROR("playerbots", "Item {} was not of class QUEST ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful QUEST item", itemTemplate->ItemId);

			return true;
		}

		this->destroy(player, item);

		return true;
	}
};
