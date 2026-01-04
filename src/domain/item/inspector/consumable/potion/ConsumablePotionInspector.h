#pragma once

#include <cstdint>

#include "SharedDefines.h"
#include "ItemTemplate.h"

#include "AbstractConsumableInspector.h"
#include "ItemActionStruct.h"
#include "ItemActionEnum.h"
#include "GlobalPlayerInspector.h"

class ConsumablePotionInspector : public AbstractConsumableInspector
{
public:
	ConsumablePotionInspector(
		uint64_t playerLowGUID,
		uint64_t itemLowGUID
	) : AbstractConsumableInspector(playerLowGUID, itemLowGUID)
	{}

	bool isInspectable() const
	{
		const uint8_t itemSubclass = this->getCurrentItemSubclass();

		return AbstractConsumableInspector::isInspectable() && itemSubclass == ITEM_SUBCLASS_POTION;
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

		if (totalQuantity < maxStackSize)
			return {
				.action = ItemActionEnum::NONE,
				.inventorySlot = 0,
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
		return {
			2461, // Deprecated Elemental Resistance Potion
			2462, // Deprecated Potion of Lesser Invulnerability (Fix)
			5632, // Deprecated Cowardly Flight Potion
			23578, // Diet McWeaksauce
			23579, // The McWeaksauce Classic
			23696, // [PH] Potion of Heightened Senses [DEP]
			23698, // [PH] Nature Resist Potion [DEP]
			30793, // NPC Equip 30793 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			32762, // Rulkster's Brain Juice
			32763, // Rulkster's Secret Sauce
			34646, // NPC Equip 34646 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			37926, // NPC Equip 37926 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			39971, // NPC Equip 39971 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			40677, // NPC Equip 40677 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			42548, // NPC Equip 42548 => Illegal item that should not even be present in the core as it is not present in any WotLK database
			45276, // Jillian's Genius Juice
			45277, // Jillian's Savior Sauce
		};
	}

};
