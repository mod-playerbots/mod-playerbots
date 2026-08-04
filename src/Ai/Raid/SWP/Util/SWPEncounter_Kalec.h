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

class Group;
class Player;
class PlayerbotAI;

namespace SwpHelpers
{

struct KalecgosRealmState
{
    uint32 lastEnterMs = 0;
    uint32 lastExitMs = 0;
    bool inSpectralRealm = false;
};

inline constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
inline constexpr uint8 KALECGOS_TANK_COUNT = 3;

struct KalecgosEncounterState
{
    uint32 encounterStartMs = 0;
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    bool surfaceHealthAnnounced = false;
    bool spectralHealthAnnounced = false;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

inline constexpr uint8 KALECGOS_GROUP_COUNT = 4;
inline constexpr float KALECGOS_SPECTRAL_REALM_Z = -74.5f;

inline Position const KALECGOS_TANK_POSITION =           { 1703.584f, 895.626f, 53.076f };
inline Position const KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;

bool IsExhausted(Player* bot);
bool IsInSpectralRealm(Player* bot);
bool IsKalecgosDecurser(Player* bot);
void EnsureKalecgosRaidAssignments(Player* bot);
Player* GetKalecgosDesignatedTank(Player* player);
Player* GetNextSurfaceTankInOrder(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid afterGuid, ObjectGuid excludedGuid = ObjectGuid::Empty,
    bool fallbackToFirst = false);
bool ShouldEnterKalecgosPortal(Player* bot);
void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI);
void RecordSpectralRealmEnter(Player* player);
void UpdateKalecgosRealmState(Player* bot, bool inSpectralRealm, uint32 timestamp);

}

#endif
