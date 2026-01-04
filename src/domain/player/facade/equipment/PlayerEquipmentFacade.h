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
#include "PlayerEquipmentFacadeResultEnum.h"
#include "GlobalPlayerInspector.h"
#include "InventoryService.h"

class PlayerEquipmentFacade : public AbstractPlayerFacade
{
public:
    PlayerEquipmentFacade(
		uint64_t playerGUID
	) :
	AbstractPlayerFacade(playerGUID)
	{}

	PlayerEquipmentFacadeResultEnum equipItem(uint64_t itemLowGUID, EquipmentSlots destinationSlot)
	{
		if (destinationSlot < EQUIPMENT_SLOT_START || destinationSlot > EQUIPMENT_SLOT_TABARD)
			return PlayerEquipmentFacadeResultEnum::IMPOSSIBLE;

		GlobalPlayerInspector playerInspector(this->playerGUID);
		GlobalItemInspector itemInspector(this->playerGUID, itemLowGUID);

		if (!playerInspector.canEquipItem(itemLowGUID, destinationSlot))
			return PlayerEquipmentFacadeResultEnum::IMPOSSIBLE;

		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return PlayerEquipmentFacadeResultEnum::IMPOSSIBLE;

		player->SwapItem(itemInspector.getItemPosition(), destinationSlot);

		return PlayerEquipmentFacadeResultEnum::REQUESTED;
	}
};
