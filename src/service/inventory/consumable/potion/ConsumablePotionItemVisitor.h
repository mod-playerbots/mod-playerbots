/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"
#include "SpellMgr.h"

class ConsumablePotionItemVisitor final : public AbstractItemVisitor
{
public:
	bool process(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		const uint32_t maxStackSize = itemTemplate->GetMaxStackSize();
		const uint32_t totalQuantity = player->GetItemCount(itemTemplate->ItemId);

		if (totalQuantity > maxStackSize)
			return false;

		return true;
	}

	bool visit(Player* player, Item* item) override
	{
		if (player == nullptr)
			return false;

		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		if (itemTemplate->Class != ITEM_CLASS_CONSUMABLE)
		{
			LOG_ERROR("playerbots", "Item {} was not of class CONSUMABLE ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_CLASS_ARMOR), std::to_string(itemTemplate->Class));

			return false;
		}

		if (itemTemplate->SubClass != ITEM_SUBCLASS_CONSUMABLE)
		{
			LOG_ERROR("playerbots", "Item {} was not of subclass CONSUMABLE ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_SUBCLASS_CONSUMABLE), std::to_string(itemTemplate->SubClass));

			return false;
		}

		if (!this->isAllowed(item))
		{
			this->destroyAll(player, item);

			return true;
		}

		if (this->process(player, item))
		{
			LOG_ERROR("playerbots", "Item {} was a useful CONSUMABLE item", itemTemplate->ItemId);

			return true;
		}

		return false;
	}

protected:

	const bool isHealthPotion(Item* item)
	{
		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		return this->isHealthPotion(itemTemplate);
	}

	const bool isHealthPotion(const ItemTemplate* itemTemplate)
	{
		const SpellInfo* const spellInfo = SpellMgr::instance()->GetSpellInfo(itemTemplate->Spells[0].SpellId);

		if (spellInfo == nullptr)
			return false;

		if (spellInfo->HasEffect(SPELL_EFFECT_HEAL))
			return true;

		return false;
	}

	const bool isManaPotion(const Item* item)
	{
		const ItemTemplate* itemTemplate = item->GetTemplate();

		if (itemTemplate == nullptr)
		{
			LOG_ERROR("playerbots", "Item {} had nullptr template", std::to_string(item->GetGUID().GetRawValue()));

			return false;
		}

		return this->isManaPotion(itemTemplate);
	}

	const bool isManaPotion(const ItemTemplate* ItemTemplate)
	{
		const SpellInfo* const spellInfo = SpellMgr::instance()->GetSpellInfo(ItemTemplate->Spells[0].SpellId);

		if (spellInfo == nullptr)
			return false;

		if (spellInfo->HasEffect(SPELL_EFFECT_ENERGIZE) && spellInfo->PowerType == POWER_MANA)
			return true;

		return false;
	}

	const std::unordered_set<uint32_t> getForbiddenItemsGUIDs()
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
