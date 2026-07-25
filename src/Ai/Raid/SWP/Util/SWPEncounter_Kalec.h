/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKALEC_H
#define PLAYERBOTS_SWPENCOUNTERKALEC_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <array>
#include <limits>
#include <unordered_map>

class Player;
class PlayerbotAI;

namespace SwpHelpers
{

constexpr uint8 KALECGOS_TANK_COUNT = 3;
constexpr uint8 KALECGOS_GROUP_COUNT = 4;
constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
constexpr float KALECGOS_SPECTRAL_REALM_Z = -74.5f;

struct KalecgosRealmState
{
    uint32 lastEnterMs = 0;
    uint32 lastExitMs = 0;
    bool inSpectralRealm = false;
};

struct KalecgosEncounterState
{
    uint32 encounterStartMs = 0;
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

extern Position const KALECGOS_TANK_POSITION;
extern Position const KALECGOS_INITIAL_RANGED_POSITION;

extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;

bool IsExhausted(Player* bot);
bool IsInSpectralRealm(Player* bot);
bool IsKalecgosDecurser(Player* bot);
void EnsureKalecgosGroupAssignments(Player* bot);
Player* GetKalecgosCurrentTank(Player* bot);
Player* GetKalecgosReplacementTank(Player* bot);
bool ShouldEnterKalecgosSpectralRift(Player* bot);
void RecordKalecgosSpectralBlastTarget(Player* bot);
void RecordKalecgosSpectralRealmEnter(Player* bot);
void UpdateKalecgosRealmState(Player* bot, bool inSpectralRealm, uint32 timestamp);

}

#endif
