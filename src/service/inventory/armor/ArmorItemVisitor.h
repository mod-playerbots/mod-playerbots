/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <cstdint>
#include <string>

#include "InventoryService.h"
#include "Log.h"
#include "StatsWeightCalculator.h"
#include "ItemTemplate.h"
#include "Player.h"

#include "AbstractItemVisitor.h"

class ArmorItemVisitor : public AbstractItemVisitor
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

		const float newItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(itemTemplate->ItemId);

		statisticsWeightCalculator.Reset();

		bool hasEquippedArmor = false;

		for (uint8_t i = 0; i < slots.size(); ++i)
		{
			Item* currentlyEquippedItem = player->GetItemByPos(slots[i]);

			if (currentlyEquippedItem == nullptr)
				continue;

			hasEquippedArmor = true;

			const float existingItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(currentlyEquippedItem->GetTemplate()->ItemId);

			if (existingItemStatisticsWeight < newItemStatisticsWeight)
			{
				this->equipArmor(player, item, slots[i]);

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

		if (itemTemplate->Class != ITEM_CLASS_ARMOR)
		{
			LOG_ERROR("playerbots", "Item {} was not of class ARMOR ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (!this->isAllowed(item))
		{
			this->destroyAll(player, item);

			return true;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful ARMOR item", itemTemplate->ItemId);

			return true;
		}

		this->magicSell(player, item);

		return true;
	}
};
