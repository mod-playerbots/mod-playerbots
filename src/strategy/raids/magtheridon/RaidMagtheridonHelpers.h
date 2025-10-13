#ifndef _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H

#include <ctime>
#include <unordered_map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

enum MagtheridonSpells
{
    // Magtheridon
    SPELL_SHADOW_CAGE  = 30205,
    SPELL_BLAST_NOVA   = 30616,
    SPELL_SHADOW_GRASP = 30410,

    // Warlock
    SPELL_BANISH       = 18647,
    SPELL_FEAR         =  6215,

    // Hunter
    SPELL_MISDIRECTION = 34477,
};

enum MagtheridonNPCs
{
    NPC_BURNING_ABYSSAL    = 17454,
    NPC_TARGET_TRIGGER     = 17474,
};

enum MagtheridonObjects
{
    GAMEOBJECT_BLAZE       = 181832,
};

namespace MagtheridonHelpers
{

constexpr uint32 SOUTH_CHANNELER     = 90978;
constexpr uint32 WEST_CHANNELER      = 90979;
constexpr uint32 NORTHWEST_CHANNELER = 90980;
constexpr uint32 EAST_CHANNELER      = 90982;
constexpr uint32 NORTHEAST_CHANNELER = 90981;

inline constexpr uint8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr uint8 starIcon = RtiTargetValue::starIndex;
inline constexpr uint8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr uint8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr uint8 triangleIcon = RtiTargetValue::triangleIndex;
inline constexpr uint8 crossIcon = RtiTargetValue::crossIndex;

Creature* GetChanneler(Player* bot, uint32 dbGuid);
bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z);

struct Location
{
    float x, y, z, orientation;
};

namespace MagtheridonsLairLocations 
{
    extern const Location WaitingForMagtheridonPosition;
    extern const Location MagtheridonTankPosition;
    extern const Location NWChannelerTankPosition;
    extern const Location NEChannelerTankPosition;
    extern const Location RangedSpreadPosition;
    extern const Location HealerSpreadPosition;
}

struct CubeInfo
{
    ObjectGuid guid;
    float x, y, z;
};

extern const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS;
extern std::unordered_map<ObjectGuid, CubeInfo> botToCubeAssignment;
std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, const std::vector<uint32>& cubeDbGuids);
void AssignBotsToCubesByGuidAndCoords(Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI);
extern std::unordered_map<uint32, bool> lastShadowCageState;
extern std::unordered_map<uint32, bool> lastBlastNovaState;
extern std::unordered_map<uint32, time_t> magtheridonBlastNovaTimer;
extern std::unordered_map<uint32, time_t> magtheridonSpreadWaitTimer;
extern std::unordered_map<uint32, time_t> magtheridonAggroWaitTimer;

}

#endif
