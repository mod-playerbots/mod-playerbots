#ifndef _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H

#include <ctime>
#include <unordered_map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

namespace MagtheridonHelpers
{
    enum MagtheridonSpells
    {
        // Magtheridon
        SPELL_SHADOW_CAGE   = 30205,
        SPELL_BLAST_NOVA    = 30616,
        SPELL_SHADOW_GRASP  = 30410,

        // Warlock
        SPELL_BANISH        = 18647,
        SPELL_FEAR          =  6215,

        // Hunter
        SPELL_MISDIRECTION  = 35079,
    };

    enum MagtheridonNPCs
    {
        NPC_BURNING_ABYSSAL = 17454,
        NPC_TARGET_TRIGGER  = 17474,
    };

    enum MagtheridonObjects
    {
        GO_BLAZE            = 181832,
    };

    constexpr uint32 SOUTH_CHANNELER     = 90978;
    constexpr uint32 WEST_CHANNELER      = 90979;
    constexpr uint32 NORTHWEST_CHANNELER = 90980;
    constexpr uint32 EAST_CHANNELER      = 90982;
    constexpr uint32 NORTHEAST_CHANNELER = 90981;

    Creature* GetChanneler(Player* bot, uint32 dbGuid);
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithCross(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z);
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot);

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
    extern std::unordered_map<uint32, bool> lastBlastNovaState;
    extern std::unordered_map<uint32, time_t> blastNovaTimer;
    extern std::unordered_map<uint32, time_t> spreadWaitTimer;
    extern std::unordered_map<uint32, time_t> dpsWaitTimer;
}

#endif
