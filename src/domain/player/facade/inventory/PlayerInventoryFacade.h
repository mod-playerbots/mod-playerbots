#pragma once

#include <cstdint>
#include <vector>

#include "Packet.h"
#include "ItemPackets.h"
#include "ObjectGuid.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"
#include "WorldSession.h"
#include "SharedDefines.h"
#include "DBCStructure.h"
#include "AuctionHouseMgr.h"

#include "AbstractPlayerFacade.h"
#include "PlayerInventoryFacadeResultEnum.h"
#include "GlobalPlayerInspector.h"
#include "InventoryService.h"
#include "GlobalItemInspector.h"

class PlayerInventoryFacade : public AbstractPlayerFacade
{
public:
    PlayerInventoryFacade(
		uint64_t playerGUID
	) :
	AbstractPlayerFacade(playerGUID)
	{}

	PlayerInventoryFacadeResultEnum equipItem(uint64_t itemLowGUID, uint8_t equipmentSlot)
	{
		if (equipmentSlot < EQUIPMENT_SLOT_START || equipmentSlot > EQUIPMENT_SLOT_TABARD)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		GlobalItemInspector itemInspector(this->playerGUID, itemLowGUID);

		const uint8_t inventoryType = itemInspector.getCurrentItemInventoryType();

		if (inventoryType == INVTYPE_AMMO)
		{
			Player* const player = this->getCurrentPlayer();

			if (player == nullptr)
				return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

			player->SetAmmo(itemLowGUID);

			return PlayerInventoryFacadeResultEnum::OK;
		}

		const uint32_t itemClass = itemInspector.getCurrentItemClass();

		if (itemClass == ITEM_CLASS_WEAPON || itemClass == ITEM_CLASS_ARMOR || itemClass == ITEM_CLASS_CONTAINER)
		{
			Player* const player = this->getCurrentPlayer();

			if (player == nullptr)
				return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

			const uint16_t overlyCleverSrc = (itemInspector.getBagSlot() << 8) | itemInspector.getItemSlot();
			const uint16_t overlyCleverDest = (INVENTORY_SLOT_BAG_0 << 8) | equipmentSlot;

			player->SwapItem(overlyCleverSrc, overlyCleverDest);

			return PlayerInventoryFacadeResultEnum::OK;
		}

		return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;
	}

	PlayerInventoryFacadeResultEnum destroyItem(uint64_t itemLowGUID)
	{
		GlobalPlayerInspector playerInspector(this->playerGUID);
		GlobalItemInspector itemInspector(this->playerGUID, itemLowGUID);
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		Item* item = itemInspector.getMutableCurrentItem();

		if (item == nullptr)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		LOG_ERROR("playerbots", "destroying item");

		uint32_t itemCount = itemInspector.getItemCurrentCount();

		player->DestroyItemCount(item, itemCount, true);

		return PlayerInventoryFacadeResultEnum::OK;
	}

	PlayerInventoryFacadeResultEnum sellItem(uint64_t itemLowGUID)
	{
		LOG_ERROR("playerbots", "Selling item {}", std::to_string(itemLowGUID));

		GlobalPlayerInspector playerInspector(this->playerGUID);
		GlobalItemInspector itemInspector(this->playerGUID, itemLowGUID);

		const uint32_t sellPrice = itemInspector.getItemSellPrice();

		LOG_ERROR("playerbots", "Item sell price {}", std::to_string(sellPrice));

		if (sellPrice == 0)
			return this->destroyItem(itemLowGUID);

		const uint32_t itemCount = itemInspector.getItemCurrentCount();

		LOG_ERROR("playerbots", "Item count {}", std::to_string(itemCount));

		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		Item* item = itemInspector.getMutableCurrentItem();

		if (item == nullptr)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		const uint32_t totalItemStackValue = sellPrice * itemCount;

		LOG_ERROR("playerbots", "Total item stack value {}", std::to_string(totalItemStackValue));

		LOG_ERROR("playerbots", "modifiying player money");
		player->ModifyMoney(totalItemStackValue);
		// LOG_ERROR("playerbots", "destroying item after sell");
		// player->DestroyItemCount(itemInspector.getBagSlot(), itemInspector.getItemSlot(), true);


		LOG_ERROR("playerbots", "removing item");
		player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
		LOG_ERROR("playerbots", "removing item from update queue");
		item->RemoveFromUpdateQueueOf(player);
		LOG_ERROR("playerbots", "adding item to buy back slot");
		player->AddItemToBuyBackSlot(item, totalItemStackValue);
		LOG_ERROR("playerbots", "Done selling item");

		return PlayerInventoryFacadeResultEnum::OK;
	}

	PlayerInventoryFacadeResultEnum auctionItem(uint64_t itemLowGUID)
	{
		const AuctionHouseMgr* const auctionHouseManager = AuctionHouseMgr::instance();
		// const AuctionHouseEntry* const auctionHouseEntry = auctionHouseManager->GetAuctionHouseEntryFromFactionTemplate();
	}
};
