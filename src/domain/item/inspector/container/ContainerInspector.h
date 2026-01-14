#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "AbstractItemInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"
#include "GlobalPlayerInspector.h"

class ContainerInspector : public AbstractItemInspector
{
public:
	ContainerInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractItemInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemSubclass = this->getCurrentItemSubclass();

		return AbstractItemInspector::isInspectable() && itemSubclass == ITEM_SUBCLASS_CONTAINER;
	}

	ItemActionStruct determineItemAction() const
	{
		if (!this->isInspectable())
			return this->getDefaultItemAction();

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		const ItemTemplate* itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return this->getDestroyItemAction();

		const GlobalPlayerInspector playerInspector(this->playerLowGUID);

		const uint8_t playerClass = playerInspector.getCurrentPlayerClass();
		const uint32_t inventoryType = itemTemplate->InventoryType;

		// @TODO: Add hunter quiver equipment decision (NOT process) here.
		if (inventoryType == INVTYPE_QUIVER && playerClass != CLASS_HUNTER)
			return this->getSellAction();

		const Item* item = this->getCurrentItem();

		if (item == nullptr)
			return this->getKeepItemAction();

		const Bag* bag = item->ToBag();

		if (bag == nullptr)
			return this->getKeepItemAction();

		const uint8_t bagSize = bag->GetBagSize();

		for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
		{
			const Item* const equippedItem = playerInspector.getItemByPosition(INVENTORY_SLOT_BAG_0, i);

			if (equippedItem == nullptr)
				return {
					.action = ItemActionEnum::EQUIP,
					.bagSlot = this->getBagSlot(),
					.containerSlot = this->getItemSlot(),
					.equipmentSlot = i
				};

			const Bag* const equippedBag = equippedBag->ToBag();

			if (equippedBag == nullptr)
				return this->getKeepItemAction();

			const uint8_t equippedBagSize = equippedBag->GetBagSize();

			if (equippedBagSize < bagSize)
				return {
					.action = ItemActionEnum::EQUIP,
					.bagSlot = this->getBagSlot(),
					.containerSlot = this->getItemSlot(),
					.equipmentSlot = i
				};
		}

		return this->getSellAction();
	}

	bool isForbiddenItem() const
	{
		const std::unordered_set<uint32_t> forbiddenItems = this->getForbiddenItemsGUIDs();

		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return false;

		return forbiddenItems.contains(itemTemplate->ItemId);
	}

protected:

	std::unordered_set<uint32_t> getForbiddenItemsGUIDs() const
	{
		return {};
	}

};
