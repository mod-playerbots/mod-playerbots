/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include "Define.h"
#include "ItemTemplate.h"
#include "Player.h"

#include "Log.h"

class AbstractItemVisitor
{
public:
	virtual ~AbstractItemVisitor() = default;

	virtual bool process(Player* player, Item* item)
	{
		return false;
	}

	virtual bool visit(Player* player, Item* item)
	{
		return false;
	}

	bool equipArmor(Player* player, Item* item, EquipmentSlots slot)
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
			return false;

		// Necessary because CanEquipItem is poorly typed.
		// We also don't want our parameter to be mutated so we use a disposable variable instead.
		uint16_t destinationSlot = slot;
		bool canEquipItem = player->CanEquipItem(item->GetSlot(), destinationSlot, item, true);

		if (!canEquipItem)
			return false;

		const Item* equippedItem = player->GetItemByPos(slot);

		if (equippedItem == nullptr)
		{
			player->EquipItem(slot, item, true);

			return true;
		}

		player->SwapItem(item->GetSlot(), slot);

		return true;
	}

	bool equipWeapon(Player* player, Item* item, EquipmentSlots slot)
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
			return false;

		// Necessary because CanEquipItem is poorly typed.
		// We also don't want our parameter to be mutated so we use a disposable variable instead.
		uint16_t destinationSlot = slot;
		bool canEquipItem = player->CanEquipItem(item->GetSlot(), destinationSlot, item, true);

		if (!canEquipItem)
			return false;

		// In case we need to unequip the off-hand we need at least one slot
		if (player->GetFreeInventorySpace() == 0)
			return false;

		const Item* equippedItem = player->GetItemByPos(slot);

		if (equippedItem == nullptr)
		{
			player->EquipItem(slot, item, true);
			player->AutoUnequipOffhandIfNeed();

			return true;
		}

		player->SwapItem(item->GetSlot(), slot);

		return true;
	}

	void destroy(Player* player, Item* item)
	{
		if (player == nullptr)
			return;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return;
		}

		LOG_ERROR("playerbots", "Destroy item {}", std::to_string(itemTemplate->ItemId));

		player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
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

		LOG_ERROR("playerbots", "Destroy multiple items of {}, ({})", std::to_string(itemTemplate->ItemId), std::to_string(count));

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

		if (itemTemplate->SellPrice)
		{
			uint32_t count = item->GetCount();

			player->ModifyMoney(itemTemplate->SellPrice * count);
			LOG_ERROR("playerbots", "Magic sold item {} for {}", std::to_string(itemTemplate->ItemId), std::to_string(itemTemplate->SellPrice * count));
		}

		this->destroy(player, item);
	}

protected:
	bool isAllowed(const ItemTemplate* itemTemplate)
	{
		if (itemTemplate == nullptr)
			return false;

		const std::unordered_set<uint32_t> forbiddenItemsGUIDs = this->getForbiddenItemsGUIDs();

		return forbiddenItemsGUIDs.find(itemTemplate->ItemId) == forbiddenItemsGUIDs.end();
	}

	bool isAllowed(Item* item)
	{
		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		return this->isAllowed(itemTemplate);
	}

	virtual const std::unordered_set<uint32_t> getForbiddenItemsGUIDs()
	{
		return {};
	}
};
