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
		{
			LOG_ERROR("playerbots", "Item is not inspectable");

			return this->getDefaultItemAction();
		}

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		Player* player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
		{
			LOG_ERROR("playerbots", "player nullptr");

			return this->getDefaultItemAction();
		}

		Item* const item = this->getMutableCurrentItem();

		if (item == nullptr)
		{
			LOG_ERROR("playerbots", "item nullptr");

			return this->getDefaultItemAction();
		}

		const bool canUseItem = player->CanUseItem(item);

		if (!canUseItem)
		{
			LOG_ERROR("playerbots", "player can't use item");

			return this->getDefaultItemAction();
		}

		StatsWeightCalculator statisticsWeightCalculator(player);
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "item template nullptr");

			return this->getDefaultItemAction();
		}

		std::vector<EquipmentSlots> slots = InventoryService::GetInstance().getItemEquipmentSlots(itemTemplate);
		const float newItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(itemTemplate->ItemId);

		statisticsWeightCalculator.Reset();

		for (uint8_t i = 0; i < slots.size(); ++i)
		{
			const uint32_t equipmentSlot = slots[i];
			player = ObjectAccessor::FindPlayer(playerGUID);

			if (player == nullptr)
				return this->getDefaultItemAction();

			const Item* const currentlyEquippedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot);

			if (currentlyEquippedItem == nullptr)
				return {
					.action = ItemActionEnum::EQUIP,
					.bagSlot = this->getBagSlot(),
					.containerSlot = this->getItemSlot(),
					.equipmentSlot = equipmentSlot
				};

			const ItemTemplate* const currentlyEquippedItemTemplate = currentlyEquippedItem->GetTemplate();

			if (currentlyEquippedItemTemplate == nullptr)
			{
				LOG_ERROR("playerbots", "current item template nullptr");

				return this->getDefaultItemAction();
			}

			const float existingItemStatisticsWeight = statisticsWeightCalculator.CalculateItem(currentlyEquippedItemTemplate->ItemId);

			if (existingItemStatisticsWeight < newItemStatisticsWeight)
			{
				return {
					.action = ItemActionEnum::EQUIP,
					.bagSlot = this->getBagSlot(),
					.containerSlot = this->getItemSlot(),
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
