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

	PlayerInventoryFacadeResultEnum destroyItem(uint64_t itemLowGUID, EquipmentSlots slot)
	{
		if (slot < INVENTORY_SLOT_BAG_START || slot > INVENTORY_SLOT_ITEM_END)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		GlobalPlayerInspector playerInspector(this->playerGUID);
		GlobalItemInspector itemInspector(this->playerGUID, itemLowGUID);
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return PlayerInventoryFacadeResultEnum::IMPOSSIBLE;

		player->DestroyItem(itemInspector.getItemInventorySlot(), itemInspector.getItemSlot(), true);

		return PlayerInventoryFacadeResultEnum::OK;
	}
};
