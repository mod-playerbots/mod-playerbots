#ifndef RAID_GRUULSLAIRHELPERS_H
#define RAID_GRUULSLAIRHELPERS_H

#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

enum GruulsLairSpells
{
	// High King Maulgar
	SPELL_WHIRLWIND  	   = 33238,

	// Krosh Firehand
	SPELL_SPELL_SHIELD   = 33054,

	// Hunter
	SPELL_MISDIRECTION   = 34477,

	// Warlock
	SPELL_BANISH     	   = 18647, // Rank 2

	// Gruul the Dragonkiller
	SPELL_GROUND_SLAM_1  = 33525,
	SPELL_GROUND_SLAM_2  = 39187,
};

constexpr uint32 NPC_WILD_FEL_STALKER = 18847;

namespace GruulsLairHelpers
{

inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;

bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
bool FindSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);

struct Location 
{
	float x, y, z;
};

namespace GruulsLairLocations 
{
    extern const Location MaulgarTankPosition;
    extern const Location OlmTankPosition;
    extern const Location BlindeyeTankPosition;
	extern const Location KroshTankPosition;
	extern const Location MaulgarRoomCenter;
    extern const Location GruulTankPosition;
}

}

#endif
