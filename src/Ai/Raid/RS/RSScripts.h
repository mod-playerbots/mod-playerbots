/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSSCRIPTS_H
#define PLAYERBOTS_RSSCRIPTS_H

#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include "ObjectGuid.h"
#include "Position.h"

namespace RubySanctumHelpers
{

    struct MeteorPingPong
    {
        uint32 count = 0;
        uint32 lastCastTime = 0;
        uint32 tankMeteorTime = 0;
        uint32 tankReturnTime = 0;
    };

    struct CutterTiming
    {
        uint32 lastShootTime = 0;
        bool active = false;
        uint32 encounterStart = 0;
        ObjectGuid bossGuid;
        float spinSign = 0.0f;
        float lastOrbAngle = 0.0f;
        bool hasOrbAngle = false;
    };

    struct PortalAddGate
    {
        uint32 armTime = 0;
        uint32 lastAddAliveTime = 0;
    };

    struct HalionCorporeality
    {
        uint8 physicalIndex = 0;
        uint32 physicalStamp = 0;
        uint8 twilightIndex = 0;
        uint32 twilightStamp = 0;
        ObjectGuid physicalGuid;
    };

    // boss health % stamped server-side (visible in every phase), read by bots that cannot see the boss through a portal
    struct BossHealth
    {
        uint8 pct = 0;
        uint32 stamp = 0;
    };

    struct TankCache
    {
        uint32 stamp = 0;
        ObjectGuid guid;
    };

    struct RsInstanceState
    {
        MeteorPingPong meteorPingPong;
        CutterTiming cutterTiming;
        PortalAddGate portalAddGate;
        HalionCorporeality halionCorporeality;
        BossHealth bossHealth;
        TankCache bossTankCache;
        TankCache twilightTankCache;
        uint32 tankAuraLastApply = 0;
        uint32 addBuffLastApply = 0;
        uint32 halionRootLastScan = 0;
        uint32 portalCountdownLastShown = 0;
        std::map<ObjectGuid, bool> p3TwilightAssignment;
        std::map<ObjectGuid, uint32> portalSeen;
        std::unordered_map<ObjectGuid, uint32> breathTwilightGrant;
        std::unordered_map<ObjectGuid, uint32> breathPhysicalGrant;
        std::unordered_map<ObjectGuid, uint32> p3RescueGrant;
        std::unordered_map<ObjectGuid, uint32> consumptionGrant;
        std::set<ObjectGuid> meteorCommitted;
        std::set<ObjectGuid> rallyCommitted;
        std::set<ObjectGuid> combustionReturning;
        std::map<ObjectGuid, std::pair<uint32, bool>> cutterDangerCache;
        std::map<ObjectGuid, bool> realmThrottled;
        std::map<ObjectGuid, bool> meteorSpotUsesA;
        std::map<ObjectGuid, ObjectGuid> botPortalTarget;
        std::set<ObjectGuid> clearedForConsumption;
        std::set<ObjectGuid> seenMeteorMarks;
    };

    // maps update on a worker-thread pool, so bots in different RS instances tick on
    // different threads; only the outer instanceId lookup is locked - the returned state
    // is only ever touched by that instance's own map thread
    RsInstanceState& RsState(uint32 instanceId);

    // drop the whole state for an instance (encounter reset / kill / idle), freeing all of it
    void ResetInstance(uint32 instanceId);

    inline constexpr uint32 HALION_CORPOREALITY_AURAS[11] =
    {
        74836, 74835, 74834, 74833, 74832, 74826, 74827, 74828, 74829, 74830, 74831
    };
}

void AddSC_RubySanctumBotScripts();

#endif
