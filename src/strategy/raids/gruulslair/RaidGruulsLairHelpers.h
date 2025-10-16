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

enum GruulsLairNPCs
{
	NPC_WILD_FEL_STALKER = 18847,
};

namespace GruulsLairHelpers
{

bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
void MarkTargetWithIcon(Unit* target, uint8 iconId);
void MarkTargetWithSquare(Unit* target);
void MarkTargetWithStar(Unit* target);
void MarkTargetWithCircle(Unit* target);
void MarkTargetWithDiamond(Unit* target);
void MarkTargetWithTriangle(Unit* target);
bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);

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
