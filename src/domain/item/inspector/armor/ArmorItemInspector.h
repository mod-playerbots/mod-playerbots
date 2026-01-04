#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "StatsWeightCalculator.h"
#include "AbstractItemInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"
#include "InventoryService.h"

class ArmorItemInspector : public AbstractItemInspector
{
public:
	ArmorItemInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractItemInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemClass = this->getCurrentItemClass();

		return AbstractItemInspector::isInspectable() && itemClass == ITEM_CLASS_ARMOR;
	}

	ItemActionStruct determineItemAction() const override
	{
		if (!this->isInspectable())
			return this->getDefaultItemAction();

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		Player* player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return this->getDefaultItemAction();

		Item* const item = this->getMutableCurrentItem();

		if (item == nullptr)
			return this->getDefaultItemAction();

		const bool canUseItem = player->CanUseItem(item);

		if (!canUseItem)
			return this->getDefaultItemAction();

		player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return this->getDefaultItemAction();

		StatsWeightCalculator statisticsWeightCalculator = StatsWeightCalculator(player);
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return this->getDefaultItemAction();

		std::vector<EquipmentSlots> slots = InventoryService::GetInstance().getItemEquipmentSlots(itemTemplate);
		const ItemTemplate* const refreshedItemTemplate = this->getCurrentItemTemplate();

		if (refreshedItemTemplate == nullptr)
			return this->getDefaultItemAction();

		const float newItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(refreshedItemTemplate->ItemId);

		statisticsWeightCalculator.Reset();

		for (uint8_t i = 0; i < slots.size(); ++i)
		{
			const uint32_t equipmentSlot = slots[i];
			player = ObjectAccessor::FindPlayer(playerGUID);

			if (player == nullptr)
				return this->getDefaultItemAction();

			const Item* const currentlyEquippedItem = player->GetItemByPos(equipmentSlot);

			if (currentlyEquippedItem == nullptr)
				return {
					.action = ItemActionEnum::EQUIP,
					.inventorySlot = this->getItemInventorySlot(),
					.equipmentSlot = equipmentSlot
				};

			const ItemTemplate* const currentlyEquippedItemTemplate = currentlyEquippedItem->GetTemplate();

			if (currentlyEquippedItemTemplate == nullptr)
				return this->getDefaultItemAction();

			const float existingItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(currentlyEquippedItemTemplate->ItemId);

			if (existingItemStatisticsWeight < newItemStatisticsWeight)
			{
				return {
					.action = ItemActionEnum::EQUIP,
					.inventorySlot = this->getItemInventorySlot(),
					.equipmentSlot = equipmentSlot
				};
			}
		}

		return this->getSellAction();
	}

	bool isForbiddenItem() const
	{
		const std::unordered_set<uint32_t> forbiddenItems = this->getForbiddenItemsGUIDs();

		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return true;

		return forbiddenItems.contains(itemTemplate->ItemId);
	}

protected:

	const std::unordered_set<uint32_t> getForbiddenItemsGUIDs() const
	{
		return {};
	}
};
