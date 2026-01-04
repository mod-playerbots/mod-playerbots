/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <unordered_set>
#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"

// Currency items
class MoneyItemVisitor : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{
		return true;
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

		if (itemTemplate->Class != ITEM_CLASS_MONEY)
		{
			LOG_ERROR("playerbots", "Item {} was not of class MONEY ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (!this->isAllowed(item))
		{
			this->destroyAll(player, item);

			return true;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful MONEY item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}

protected:

	const std::unordered_set<uint32_t> getForbiddenItemsGUIDs() override
	{
		return {
			37711, // Currency Token Test Token 1
			37742, // Currency Token Test Token 2
			38644, // Currency Token Test Token 3
			41749, // Birmingham Test Item 3
			43308, // Honor Points (item, not the real currency)
			43949, // zzzOLDDaily Quest Faction Token
			44209, // NPC Equip 44209 => Illegal item that should not even be present in the core as it is not present in any WotLK database
		};
	}
};
