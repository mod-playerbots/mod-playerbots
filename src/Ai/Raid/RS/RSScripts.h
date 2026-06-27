/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOT_RSSCRIPTS_H
#define _PLAYERBOT_RSSCRIPTS_H

#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ObjectGuid.h"
#include "Position.h"

class Map;

namespace RubySanctumHelpers
{

    extern std::recursive_mutex stateMutex;

    struct MeteorPingPong
    {
        uint32 count;
        uint32 lastCastTime;
        uint32 tankMeteorTime;
        uint32 tankReturnTime;
    };

    extern std::unordered_map<uint32, MeteorPingPong> meteorPingPong;

    struct CutterTiming
    {
        uint32 lastShootTime;
        bool active;
        uint32 encounterStart;
        ObjectGuid bossGuid;
        float spinSign;
        float lastOrbAngle;
        bool hasOrbAngle;
    };

    extern std::unordered_map<uint32, CutterTiming> cutterTiming;

    struct PortalAddGate
    {
        uint32 armTime;
        uint32 lastAddAliveTime;
    };

    extern std::unordered_map<uint32, PortalAddGate> portalAddGate;

    extern std::map<std::pair<uint32, ObjectGuid>, bool> p3TwilightAssignment;

    struct HalionCorporeality
    {
        uint8 physicalIndex;
        uint32 physicalStamp;
        uint8 twilightIndex;
        uint32 twilightStamp;
        ObjectGuid physicalGuid;
    };

    extern std::unordered_map<uint32, HalionCorporeality> halionCorporeality;

    // boss health % stamped server-side (visible in every phase), read by bots that cannot see the boss through a portal
    struct BossHealth
    {
        uint8 pct;
        uint32 stamp;
    };

    extern std::unordered_map<uint32, BossHealth> bossHealth;

    // throttle/state keyed by instance, cleared on encounter reset
    extern std::unordered_map<uint32, uint32> tankAuraLastApply;
    extern std::unordered_map<uint32, uint32> addBuffLastApply;
    extern std::unordered_map<uint32, uint32> halionRootLastScan;
    extern std::unordered_map<uint32, uint32> portalCountdownLastShown;
    extern std::unordered_map<uint32, std::map<ObjectGuid, uint32>> portalSeen;

    // per-bot transient fight state keyed by bot guid, cleared on encounter reset
    extern std::unordered_map<ObjectGuid, uint32> breathTwilightGrant;
    extern std::unordered_map<ObjectGuid, uint32> breathPhysicalGrant;
    extern std::unordered_map<ObjectGuid, uint32> p3RescueGrant;
    extern std::unordered_map<ObjectGuid, uint32> consumptionGrant;
    extern std::set<ObjectGuid> meteorCommitted;
    extern std::set<ObjectGuid> rallyCommitted;
    extern std::set<ObjectGuid> combustionReturning;
    extern std::map<ObjectGuid, std::pair<uint32, bool>> cutterDangerCache;
    extern std::map<ObjectGuid, bool> realmThrottled;
    extern std::map<ObjectGuid, bool> meteorSpotUsesA;
    extern std::map<ObjectGuid, ObjectGuid> botPortalTarget;
    extern std::set<ObjectGuid> clearedForConsumption;

    // clear every per-instance and per-bot entry for an instance (encounter reset / kill / idle)
    void ResetInstance(uint32 instanceId, Map* map);

    inline constexpr uint32 HALION_CORPOREALITY_AURAS[11] =
    {
        74836, 74835, 74834, 74833, 74832, 74826, 74827, 74828, 74829, 74830, 74831
    };
}

void AddSC_RubySanctumBotScripts();

#endif
