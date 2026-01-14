#pragma once

#include <cstdint>

#include "ObjectGuid.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"

#include "AiFactory.h"
#include "PlayerbotAI.h"
#include "AbstractPlayerInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"

class GlobalPlayerInspector : public AbstractPlayerInspector
{
public:
    GlobalPlayerInspector(
		uint64_t playerGUID
	) :
	AbstractPlayerInspector(playerGUID)
	{}

	bool canEquipItem(const uint32_t itemLowGUID, uint16_t destinationSlot) const
	{
		Item* const item = this->getItemByGUID(itemLowGUID);

		if (item == nullptr)
			return false;

		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		return player->CanEquipItem(item->GetSlot(), destinationSlot, item, true);
	}

	bool canUseItem(const uint32_t itemGUID) const
	{
		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return false;

		const ObjectGuid itemFullGUID = ObjectGuid::Create<HighGuid::Item>(itemGUID);
		Item* const item = player->GetItemByGuid(itemFullGUID);

		if (item == nullptr)
			return false;

		return player->CanUseItem(item);
	}

	uint8_t getCurrentPlayerClass() const
	{
		const Player* const player = this->getCurrentPlayer();

		// If the player is offline, we bail out by returning a void class to avoid any offline mutation.
		if (player == nullptr)
			return CLASS_NONE;

		return player->getClass();
	}

	bool requiresItemForActiveQuest(const uint32_t itemTemplateId) const
	{
		const Player* const player = this->getCurrentPlayer();

		// If the player is offline, we consider the item as being in use to avoid modifying an offline inventory.
		if (player == nullptr)
			return true;

		return player->HasQuestForItem(itemTemplateId);
	}

	uint32_t getItemCount(const uint32_t itemTemplateId) const
	{
		const Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return 0;

		return player->GetItemCount(itemTemplateId);
	}

	Item* getItemByPosition(const uint16_t slot) const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return nullptr;

		Item* const item = player->GetItemByPos(slot);

		return item;
	}

	Item* getItemByPosition(const uint8_t bag, const uint8_t slot) const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return nullptr;

		Item* const item = player->GetItemByPos(bag, slot);

		return item;
	}

	uint32_t getPlayerMoney() const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return 0;

		return player->GetMoney();
	}

	// Item* getBagItemByPosition(const uint8_t bagNumber, const uint8_t slot) const
	// {
	// 	if (bagNumber >= INVENTORY_SLOT_BAG_END)
	// 		return nullptr;

	// 	if (bagNumber == 0)
	// 	{
	// 		const Player* const player = this->getCurrentPlayer();

	// 		if (player == nullptr)
	// 			return nullptr;

	// 		return player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
	// 	}

	// 	Player* const player = this->getCurrentPlayer();

	// 	if (player == nullptr)
	// 		return nullptr;

	// 	Item* const bagItem = player->GetItemByPos(INVENTORY_SLOT_BAG_START + bagNumber - 1);

	// 	if (bagItem == nullptr)
	// 		return nullptr;

	// 	Bag* const bag = bagItem->ToBag();

	// 	if (bag == nullptr)
	// 		return nullptr;

	// 	return bag->GetItemByPos(slot);
	// }

	Item* getItemByGUID(const uint64_t guid) const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return nullptr;

		const ObjectGuid fullItemGUID = ObjectGuid::Create<HighGuid::Item>(guid);

		Item* const item = player->GetItemByGuid(fullItemGUID);

		return item;
	}

	Item* getMutableItemByGUID(const uint64_t guid) const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return nullptr;

		const ObjectGuid fullItemGUID = ObjectGuid::Create<HighGuid::Item>(guid);

		Item* const item = player->GetItemByGuid(fullItemGUID);

		return item;
	}

	uint8_t getDominantPlayerTalentTab() const
	{
		Player* const player = this->getCurrentPlayer();

		if (player == nullptr)
			return 255;

		return AiFactory::GetPlayerSpecTab(player);
	}

	Stats getPrimaryStat() const
	{
		const uint8_t playerClass = this->getCurrentPlayerClass();

		switch (playerClass)
		{
			case CLASS_WARRIOR:
				return STAT_STRENGTH;
			case CLASS_PALADIN:
			{
    			const uint8_t dominantTab = this->getDominantPlayerTalentTab();

				switch (dominantTab)
				{
					case PALADIN_TAB_HOLY:
						return STAT_INTELLECT;
					case PALADIN_TAB_PROTECTION:
						return STAT_STAMINA;
					case PALADIN_TAB_RETRIBUTION:
						return STAT_STRENGTH;
				}

				return STAT_STAMINA;
			}
			case CLASS_HUNTER:
				return STAT_AGILITY;
			case CLASS_ROGUE:
				return STAT_AGILITY;
			case CLASS_PRIEST:
				return STAT_SPIRIT;
			case CLASS_DEATH_KNIGHT:
				return STAT_STRENGTH;
			case CLASS_SHAMAN:
			{
    			const uint8_t dominantTab = this->getDominantPlayerTalentTab();

				switch (dominantTab)
				{
					case SHAMAN_TAB_ELEMENTAL:
						return STAT_INTELLECT;
					case SHAMAN_TAB_ENHANCEMENT:
						return STAT_AGILITY;
					case SHAMAN_TAB_RESTORATION:
						return STAT_INTELLECT;
				}

				return STAT_STAMINA;
			}
			case CLASS_MAGE:
				return STAT_INTELLECT;
			case CLASS_DRUID:
			{
    			const uint8_t dominantTab = this->getDominantPlayerTalentTab();

				switch (dominantTab)
				{
					case DRUID_TAB_BALANCE:
						return STAT_INTELLECT;
					case DRUID_TAB_FERAL:
						return STAT_AGILITY;
					case DRUID_TAB_RESTORATION:
						return STAT_SPIRIT;
				}

				return STAT_STAMINA;
			}
		}
	}
};
