/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"

#include "ConsumableConsumableItemVisitor.h"
#include "ConsumablePotionItemVisitor.h"
#include "ConsumableElixirItemVisitor.h"

class ConsumableItemVisitor : public AbstractItemVisitor
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

		switch (itemTemplate->SubClass)
		{
			case ITEM_SUBCLASS_CONSUMABLE:
			{
				ConsumableConsumableItemVisitor visitor = ConsumableConsumableItemVisitor();
				bool visited = visitor.visit(player, item);

				return visited;
			}

			case ITEM_SUBCLASS_POTION:
			{
				ConsumablePotionItemVisitor visitor = ConsumablePotionItemVisitor();
				bool visited = visitor.visit(player, item);

				return visited;
			}

			case ITEM_SUBCLASS_ELIXIR:
			{
				ConsumableElixirItemVisitor visitor = ConsumableElixirItemVisitor();
				bool visited = visitor.visit(player, item);

				return visited;
			}

			default:

				return false;
		}

		return false;
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

		if (itemTemplate->Class != ITEM_CLASS_CONSUMABLE)
		{
			LOG_ERROR("playerbots", "Item {} was not of class CONSUMABLE ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful CONSUMABLE item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}

	void destroyAll(Player* player, Item* item)
	{
		if (player == nullptr)
			return;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return;
		}

		uint32_t count = item->GetCount();

		LOG_ERROR("playerbots", "Destroy multiple items of {}, ({})", std::to_string(item->GetTemplate()->ItemId), std::to_string(count));

		player->DestroyItemCount(item, count, true);
	}

	void magicSell(Player* player, Item* item)
	{
		if (player == nullptr || item == nullptr)
			return;

		if (item->GetState() == ITEM_REMOVED)
			return;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return;
		}

		LOG_ERROR("playerbots", "Magic selling item {}", std::to_string(itemTemplate->ItemId));

		uint32_t count = itemTemplate->GetMaxStackSize();

		if (itemTemplate->SellPrice)
		{
			player->ModifyMoney(itemTemplate->SellPrice * count);
			LOG_ERROR("playerbots", "Magic sold item {} for {}", std::to_string(itemTemplate->ItemId), std::to_string(itemTemplate->SellPrice * count));
		}

		player->DestroyItemCount(item, count, true);
	}
};
