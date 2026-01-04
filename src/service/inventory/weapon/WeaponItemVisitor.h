/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <cstdint>

#include "ItemTemplate.h"
#include "Player.h"
#include "StatsWeightCalculator.h"

#include "AbstractItemVisitor.h"
#include "InventoryService.h"

class WeaponItemVisitor : public AbstractItemVisitor
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

		if (!player->CanUseItem(item))
			return false;

		StatsWeightCalculator statisticsWeightCalculator = StatsWeightCalculator(player);
		std::vector<EquipmentSlots> slots = InventoryService::GetInstance().getItemEquipmentSlots(itemTemplate);

		float newItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(itemTemplate->ItemId);

		statisticsWeightCalculator.Reset();

		for (uint8_t i = 0; i < slots.size(); ++i)
		{
			Item* currentlyEquippedItem = player->GetItemByPos(slots[i]);

			if (currentlyEquippedItem == nullptr)
				continue;

			float existingItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(currentlyEquippedItem->GetTemplate()->ItemId);

			if (existingItemStatisticsWeight < newItemStatisticsWeight)
			{
				this->equipWeapon(player, item, slots[i]);

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

		if (itemTemplate->Class != ITEM_CLASS_WEAPON)
		{
			LOG_ERROR("playerbots", "Item {} was not of class WEAPON ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful WEAPON item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}
};
