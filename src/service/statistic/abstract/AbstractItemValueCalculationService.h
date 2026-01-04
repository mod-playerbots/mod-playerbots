#ifndef _PLAYERBOT_ABSTRACT_ITEM_VALUE_CALCULATION_SERVICE_H
#define _PLAYERBOT_ABSTRACT_ITEM_VALUE_CALCULATION_SERVICE_H

#include <cstdint>

#include "ItemTemplate.h"
#include "Player.h"

class AbstractItemValueCalculationService
{
public:

	inline uint64_t calculateItemValue(Player* player, ItemTemplate* itemTemplate)
	{
		uint64_t itemValue = 0;

		for (uint8_t i = 0; i < itemTemplate->StatsCount; ++i)
		{
			uint8_t statisticWeight = this->getStatisticWeight(player, itemTemplate, itemTemplate->ItemStat[i].ItemStatType);

			itemValue += statisticWeight * itemTemplate->ItemStat[i].ItemStatValue;
		}

		if (itemTemplate->Armor > 0)
			itemValue += this->getArmorRatingWeight(player) * itemTemplate->Armor;

		if (itemTemplate->FireRes > 0)
			itemValue += this->getFireResistanceWeight(player);

		if (itemTemplate->NatureRes > 0)
			itemValue += this->getNatureResistanceWeight(player);

		if (itemTemplate->FrostRes > 0)
			itemValue += this->getFrostResistanceWeight(player);

		if (itemTemplate->ShadowRes > 0)
			itemValue += this->getShadowResistanceWeight(player);

		if (itemTemplate->ArcaneRes > 0)
			itemValue += this->getArcaneResistanceWeight(player);

		return itemValue;
	}

	inline uint8_t getStatisticWeight(Player* player, ItemTemplate* itemTemplate, uint32_t type)
	{
		uint8_t primaryStatisticWeight = this->getPrimaryStatisticWeight(player, type);

		if (primaryStatisticWeight != 0)
			return primaryStatisticWeight;

		uint8_t physicalStatisticWeight = this->getPhysicalStatisticWeight(player, type);

		if (physicalStatisticWeight != 0)
			return physicalStatisticWeight;

		uint8_t spellStatisticWeight = this->getSpellStatisticWeight(player, type);

		if (spellStatisticWeight != 0)
			return spellStatisticWeight;

		return this->getDefenseStatisticWeight(player, type);
	}

	inline uint8_t getPrimaryStatisticWeight(Player* player, uint32_t type)
	{
		switch (type)
		{
			case ITEM_MOD_STRENGTH:
				return this->getStrengthWeight(player);
			case ITEM_MOD_AGILITY:
				return this->getAgilityWeight(player);
			case ITEM_MOD_STAMINA:
				return this->getStaminaWeight(player);
			case ITEM_MOD_INTELLECT:
				return this->getIntellectWeight(player);
			case ITEM_MOD_SPIRIT:
				return this->getSpiritWeight(player);
			default:
				return 0;
		}
	}

	inline uint8_t getPhysicalStatisticWeight(Player* player, uint32_t type)
	{
		switch (type)
		{
			case ITEM_MOD_ATTACK_POWER:
				return this->getAttackPowerWeight(player);
			case ITEM_MOD_HASTE_RATING:
				return this->getHasteRatingWeight(player);
			case ITEM_MOD_HIT_RATING:
				return this->getHitRatingWeight(player);
			case ITEM_MOD_CRIT_RATING:
				return this->getCriticalStrikeRatingWeight(player);
			case ITEM_MOD_EXPERTISE_RATING:
				return this->getExperiseRatingWeight(player);
			case ITEM_MOD_ARMOR_PENETRATION_RATING:
				return this->getArmorPenetrationRatingWeight(player);
			default:
				return 0;
		}
	}

	inline uint8_t getSpellStatisticWeight(Player* player, uint32_t type)
	{
		switch (type)
		{
			case ITEM_MOD_SPELL_POWER:
				return this->getSpellPowerWeight(player);
			case ITEM_MOD_SPELL_HEALING_DONE:
				return this->getHealingPowerWeight(player);
			case ITEM_MOD_CRIT_SPELL_RATING:
				return this->getSpellCriticalStrikeRatingWeight(player);
			case ITEM_MOD_HIT_SPELL_RATING:
				return this->getSpellHitRatingWeight(player);
			case ITEM_MOD_HASTE_SPELL_RATING:
				return this->getSpellHasteRatingWeight(player);
			case ITEM_MOD_MANA_REGENERATION:
				return this->getManaPer5SecondsWeight(player);
			case ITEM_MOD_SPELL_PENETRATION:
				return this->getSpellPenetrationRatingWeight(player);
			default:
				return 0;
		}
	}

	inline uint8_t getDefenseStatisticWeight(Player* player, uint32_t type)
	{
		switch (type)
		{
			case ITEM_MOD_DEFENSE_SKILL_RATING:
				return this->getDefenseRatingWeight(player);
			case ITEM_MOD_DODGE_RATING:
				return this->getDodgeRatingWeight(player);
			case ITEM_MOD_PARRY_RATING:
				return this->getParryRatingWeight(player);
			case ITEM_MOD_BLOCK_RATING:
				return this->getBlockRatingWeight(player);
			case ITEM_MOD_BLOCK_VALUE:
				return this->getBlockValueWeight(player);
			case ITEM_MOD_RESILIENCE_RATING:
				return this->getResilienceRatingWeight(player);
			default:
				return 0;
		}
	}

protected:

	// Primary
	virtual inline const uint8_t getStrengthWeight(Player* player) { return 0; };
	virtual inline const uint8_t getAgilityWeight(Player* player) { return 0; };
	virtual inline const uint8_t getStaminaWeight(Player* player) { return 0; };
	virtual inline const uint8_t getIntellectWeight(Player* player) { return 0; };
	virtual inline const uint8_t getSpiritWeight(Player* player) { return 0; };

	// Physical
	virtual inline const uint8_t getAttackPowerWeight(Player* player) { return 0; };
	virtual inline const uint8_t getHasteRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getHitRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getCriticalStrikeRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getExperiseRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getArmorPenetrationRatingWeight(Player* player) { return 0; };

	// Spell
	virtual inline const uint8_t getSpellPowerWeight(Player* player) { return 0; };

	// Unable to find these in the ItemTemplate struct
	// virtual inline const uint8_t getHolySpellPowerWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getFireSpellPowerWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getNatureSpellPowerWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getFrostSpellPowerWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getShadowSpellPowerWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getArcaneSpellPowerWeight(Player* player) { return 0; };
	virtual inline const uint8_t getHealingPowerWeight(Player* player) { return 0; };
	virtual inline const uint8_t getSpellHitRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getSpellCriticalStrikeRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getSpellHasteRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getManaPer5SecondsWeight(Player* player) { return 0; };
	virtual inline const uint8_t getSpellPenetrationRatingWeight(Player* player) { return 0; };

	// Defense
	virtual inline const uint8_t getArmorRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getDefenseRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getDodgeRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getParryRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getBlockRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getBlockValueWeight(Player* player) { return 0; };
	virtual inline const uint8_t getResilienceRatingWeight(Player* player) { return 0; };
	virtual inline const uint8_t getFireResistanceWeight(Player* player) { return 0; };
	virtual inline const uint8_t getNatureResistanceWeight(Player* player) { return 0; };
	virtual inline const uint8_t getFrostResistanceWeight(Player* player) { return 0; };
	virtual inline const uint8_t getShadowResistanceWeight(Player* player) { return 0; };
	virtual inline const uint8_t getArcaneResistanceWeight(Player* player) { return 0; };
};

#endif
