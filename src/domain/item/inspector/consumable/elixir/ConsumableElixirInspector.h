#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "AbstractConsumableInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"
#include "GlobalPlayerInspector.h"

class ConsumableElixirInspector : public AbstractConsumableInspector
{
public:
	ConsumableElixirInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractConsumableInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemSubclass = this->getCurrentItemSubclass();

		return AbstractConsumableInspector::isInspectable() && itemSubclass == ITEM_SUBCLASS_ELIXIR;
	}

	ItemActionStruct determineItemAction() const
	{
		if (!this->isInspectable())
			return this->getDefaultItemAction();

		if (this->isForbiddenItem())
			return this->getForbiddenItemAction();

		const uint32_t maxStackSize = this->getItemMaxStackSize();
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return this->getDefaultItemAction();

		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		Player* player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return this->getDefaultItemAction();

		const uint32_t totalQuantity = player->GetItemCount(itemTemplate->ItemId);
		const ItemTemplate* const refreshedItemTemplate = this->getCurrentItemTemplate();

		if (refreshedItemTemplate == nullptr)
			return this->getDefaultItemAction();

		if (totalQuantity < maxStackSize)
			return {
				.action = ItemActionEnum::NONE,
				.bagSlot = 0,
				.containerSlot = 0,
				.equipmentSlot = 0
			};

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

	const std::unordered_set<uint32_t> getForbiddenItemsGUIDs() const
	{
		return {};
	}

};
