#ifndef _PLAYERBOT_GENERIC_WARRIOR_ITEM_VALUE_CALCULATION_SERVICE_H
#define _PLAYERBOT_GENERIC_WARRIOR_ITEM_VALUE_CALCULATION_SERVICE_H

#include <cstdint>

#include "AbstractItemValueCalculationService.h"
#include "Player.h"

class GenericWarriorItemValueCalculationService : public AbstractItemValueCalculationService
{
protected:

	// Primary
	inline const uint8_t getStrengthWeight(Player* player) override
	{
		return 4;
	};

	inline const uint8_t getAgilityWeight(Player* player) override
	{
		return 1;
	};

	inline const uint8_t getStaminaWeight(Player* player) override
	{
		return 2;
	};

	// Physical
	inline const uint8_t getAttackPowerWeight(Player* player) override
	{
		return 2;
	};

	inline const uint8_t getHasteRatingWeight(Player* player) override
	{
		return 2;
	};

	inline const uint8_t getHitRatingWeight(Player* player) override
	{
		return 2;
	};

	inline const uint8_t getCriticalStrikeRatingWeight(Player* player) override
	{
		return 2;
	};

	inline const uint8_t getExperiseRatingWeight(Player* player) override
	{
		return 2;
	};

	inline const uint8_t getArmorPenetrationRatingWeight(Player* player) override
	{
		return 2;
	};

	// Defense
	// virtual inline const uint8_t getArmorRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getDefenseRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getDodgeRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getParryRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getBlockRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getBlockValueWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getResilienceRatingWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getFireResistanceWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getNatureResistanceWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getFrostResistanceWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getShadowResistanceWeight(Player* player) { return 0; };
	// virtual inline const uint8_t getArcaneResistanceWeight(Player* player) { return 0; };
};

#endif
