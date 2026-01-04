/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "ItemTemplate.h"
#include "AbstractItemVisitor.h"
#include "Player.h"
#include "SpellAuraDefines.h"
#include "SpellMgr.h"

#include "AiFactory.h"
#include "PlayerbotAI.h"

class ConsumableElixirItemVisitor final : public AbstractItemVisitor
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

		if (!this->isAppropriateForPlayer(player, itemTemplate))
			return false;

		const uint32_t stackSize = itemTemplate->GetMaxStackSize();
		const uint32_t totalQuantity = player->GetItemCount(itemTemplate->ItemId);

		if (totalQuantity > stackSize)
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

		if (itemTemplate->SubClass != ITEM_SUBCLASS_ELIXIR)
		{
			LOG_ERROR("playerbots", "Item {} was not of subclass ELIXIR ({}), found {}", itemTemplate->ItemId, std::to_string(ITEM_SUBCLASS_CONSUMABLE), std::to_string(itemTemplate->SubClass));

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

private:

	bool isAppropriateForPlayer(Player* player, const ItemTemplate* itemTemplate)
	{
		if (player == nullptr)
			return false;

		const SpellInfo* const spellInfo = SpellMgr::instance()->GetSpellInfo(itemTemplate->Spells[0].SpellId);

		if (spellInfo == nullptr)
			return false;

		if (!spellInfo->HasEffect(SPELL_EFFECT_APPLY_AURA))
			return false;

		if (!spellInfo->HasAura(SPELL_AURA_MOD_STAT))
			return false;

		const std::array<SpellEffectInfo, MAX_SPELL_EFFECTS> effects = spellInfo->GetEffects();

		bool isUseful = false;

		for (const SpellEffectInfo effect : effects)
		{
			if (effect.MiscValue < STAT_STRENGTH || effect.MiscValue > STAT_SPIRIT)
				continue;

			const Stats primaryStat = this->getPrimaryStat(player);

			if (primaryStat == effect.MiscValue)
				return true;
		}

		return false;
	}

	const Stats getPrimaryStat(Player* player)
	{
		if (player == nullptr)
			return STAT_STAMINA;

		const uint8_t playerClass = player->getClass();

		switch (playerClass)
		{
			case CLASS_WARRIOR:
				return STAT_STRENGTH;
			case CLASS_PALADIN:
			{
    			const uint8_t tab = AiFactory::GetPlayerSpecTab(player);

				switch (tab)
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
    			const uint8_t tab = AiFactory::GetPlayerSpecTab(player);

				switch (tab)
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
    			const uint8_t tab = AiFactory::GetPlayerSpecTab(player);

				switch (tab)
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
