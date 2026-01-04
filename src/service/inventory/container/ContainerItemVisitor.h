/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"
#include "Player.h"

#include "AbstractItemVisitor.h"

class ContainerItemVisitor : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{

		if (player == nullptr)
			return false;

		const Bag* bag = item->ToBag();

		// @TODO: Handle all container type
		if (bag == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		if (itemTemplate->InventoryType == INVTYPE_QUIVER && player->getClass() != CLASS_HUNTER)
			return false;

		uint8_t initialSlot = INVENTORY_SLOT_BAG_START;

		if (itemTemplate->InventoryType == INVTYPE_QUIVER)
			initialSlot = INVENTORY_SLOT_BAG_END - 1;

		for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
		{
			Bag* equipedBag = player->GetBagByPos(i);

			if (!equipedBag || bag->GetBagSize() > equipedBag->GetBagSize())
			{
				player->EquipItem(i, item, true);

				return true;
			}
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

		if (itemTemplate->Class != ITEM_CLASS_CONTAINER)
		{
			LOG_ERROR("playerbots", "Item {} was not of class CONTAINER ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (itemTemplate->InventoryType != INVTYPE_BAG)
			return false;

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful CONTAINER item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}
};
