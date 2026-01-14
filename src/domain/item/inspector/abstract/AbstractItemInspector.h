#pragma once

#include <cstdint>

#include "ObjectGuid.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ItemTemplate.h"
#include "Item.h"

#include "AbstractInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"

class AbstractItemInspector : public AbstractInspector
{
public:
	virtual ItemActionStruct determineItemAction() const
	{
		return this->getDefaultItemAction();
	};

	uint8_t getBagSlot() const
	{
		const Item* const item = this->getCurrentItem();

		if  (item == nullptr)
			return 0;

		return item->GetBagSlot();
	}

	uint8_t getItemSlot() const
	{
		const Item* const item = this->getCurrentItem();

		if  (item == nullptr)
			return 0;

		return item->GetSlot();
	}

	uint8_t getItemPosition() const
	{
		const Item* const item = this->getCurrentItem();

		if  (item == nullptr)
			return 0;

		return item->GetPos();
	}

	uint32_t getCurrentItemClass() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return 0;

		return itemTemplate->Class;
	}

	uint8_t getCurrentItemInventoryType() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return 0;

		return itemTemplate->InventoryType;
	}

	uint32_t getCurrentItemSubclass() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return 255;

		return itemTemplate->SubClass;
	}

	uint32_t getItemMaxStackSize() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return 0;

		return itemTemplate->GetMaxStackSize();
	}

	ItemUpdateState getItemUpdateState() const
	{
		const Item* const item = this->getCurrentItem();

		if (item == nullptr)
			return ITEM_REMOVED;

		return item->GetState();
	}

	uint32_t getItemSellPrice() const
	{
		const ItemTemplate* const itemTemplate = this->getCurrentItemTemplate();

		if (itemTemplate == nullptr)
			return 0;

		return itemTemplate->SellPrice;
	}

	uint32_t getItemCurrentCount() const
	{
		const Item* const item = this->getCurrentItem();

		if (item == nullptr)
			return 0;

		return item->GetCount();
	}

	bool itemIsSellable() const
	{
		return this->getItemSellPrice() != 0;
	}

	bool itemIsInWorld() const
	{
		const Item* const item = this->getCurrentItem();

		if (item == nullptr)
			return false;

		return item->IsInWorld();
	}

	bool itemIsInUnsafeContainer() const
	{
		const Item* const item = this->getCurrentItem();

		if (item == nullptr)
			return true;

		const ObjectGuid containingField = item->GetGuidValue(ITEM_FIELD_CONTAINED);

		return containingField == ObjectGuid::Empty;
	}

	const ItemTemplate* getCurrentItemTemplate() const
	{
		ObjectMgr* const objectManager = ObjectMgr::instance();

		if (objectManager == nullptr)
			return nullptr;

		const Item* const item = this->getCurrentItem();

		if (item == nullptr)
			return nullptr;

		return item->GetTemplate();
	}

	uint32_t getCurrentItemTemplateLowGUID() const
	{
		return this->itemTemplateLowGUID;
	}

	const Item* getCurrentItem() const
	{
		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		const Player* const player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return nullptr;

		const ObjectGuid itemGUID = ObjectGuid::Create<HighGuid::Item>(this->itemLowGUID);
		Item* const item = player->GetItemByGuid(itemGUID);

		return item;
	}

	Item* getMutableCurrentItem() const
	{
		const ObjectGuid playerGUID = ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID);
		const Player* const player = ObjectAccessor::FindPlayer(playerGUID);

		if (player == nullptr)
			return nullptr;

		const ObjectGuid itemGUID = ObjectGuid::Create<HighGuid::Item>(this->itemLowGUID);
		Item* const item = player->GetItemByGuid(itemGUID);

		return item;
	}

protected:
    AbstractItemInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) :
	itemLowGUID(itemLowGUID),
	playerLowGUID(playerLowGUID),
	itemTemplateLowGUID(0)
	{
		const Player* const player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(this->playerLowGUID));

		if (player == nullptr)
			return;

		const Item* const item = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(this->itemLowGUID));

		if (item == nullptr)
		{
			this->itemTemplateLowGUID = 0;

			return;
		}

		const ItemTemplate* const itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			this->itemTemplateLowGUID = 0;

			return;
		}

		this->itemTemplateLowGUID = itemTemplate->ItemId;
	}

	AbstractItemInspector& operator=(AbstractItemInspector const&) = delete;

	ItemActionStruct getDefaultItemAction() const
	{
		return this->getKeepItemAction();
	}

	ItemActionStruct getKeepItemAction() const
	{
		return {
			.action = ItemActionEnum::NONE,
			.bagSlot = 0,
			.containerSlot = 0,
			.equipmentSlot = 0
		};
	}

	ItemActionStruct getForbiddenItemAction() const
	{
		return this->getDestroyItemAction();
	}

	ItemActionStruct getDestroyItemAction() const
	{
		return {
			.action = ItemActionEnum::DESTROY,
			.bagSlot = this->getBagSlot(),
			.containerSlot = 0,
			.equipmentSlot = 0
		};
	}

	ItemActionStruct getSellAction() const
	{
		const bool sellable = this->itemIsSellable();

		if (sellable)
			return {
				.action = ItemActionEnum::SELL,
				.bagSlot = this->getBagSlot(),
				.containerSlot = 0,
				.equipmentSlot = 0
			};

		return {
			.action = ItemActionEnum::DESTROY,
			.bagSlot = this->getBagSlot(),
			.containerSlot = 0,
			.equipmentSlot = 0
		};
	}

	const uint64_t itemLowGUID;
	const uint64_t playerLowGUID;
	uint32_t itemTemplateLowGUID;
};
