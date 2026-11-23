/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKJ_H
#define PLAYERBOTS_SWPENCOUNTERKJ_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct KiljaedenRangedBotAssignment
{
    ObjectGuid guid;
    uint8 slotIndex = 0;
};

struct KiljaedenArmageddon
{
    Position destination;
    uint32 expireMs = 0;
    float safeDistance = 0.0f;
};

inline std::array const KILJAEDEN_DRAGON_ORB_ENTRIES = {
    Id(SwpObjects::GO_DRAGON_ORB_1),
    Id(SwpObjects::GO_DRAGON_ORB_2),
    Id(SwpObjects::GO_DRAGON_ORB_3),
    Id(SwpObjects::GO_DRAGON_ORB_4),
};

inline std::array const KILJAEDEN_DRAGON_SPELLS = {
    Id(SwpSpells::SPELL_SHIELD_OF_THE_BLUE),
    Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE),
    Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE),
};

inline constexpr float KILJAEDEN_PHASE3_HP_THRESHOLD = 85.0f;
inline constexpr float KILJAEDEN_PHASE4_HP_THRESHOLD = 55.0f;
inline constexpr float KILJAEDEN_PHASE5_HP_THRESHOLD = 25.0f;

// After Kil'jaeden becomes attackable, every bot but the main tank holds DPS on him this long.
inline constexpr uint32 KILJAEDEN_DPS_HOLD_MS = 5 * IN_MILLISECONDS;

inline constexpr uint32 HAND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 DRAGON_ORB_CACHE_INTERVAL_MS = 200;

// The orb user is announced out of combat within this 2D distance of the controller's spawn.
inline constexpr float KILJAEDEN_ORB_USER_ANNOUNCE_DISTANCE = 100.0f;
// Since M'uru is above Kil'jaeden, a Z offset ensures announcement only after M'uru.
inline constexpr float KILJAEDEN_ORB_USER_ANNOUNCE_Z = 40.0f;
// Beyond this 2D distance the announcement is forgotten, so a raid that returns hears it again.
inline constexpr float KILJAEDEN_ORB_USER_REARM_DISTANCE = 150.0f;

// Knockback (47115) hits everyone within 9 yd of KJ's center (3D, no combat reach) every second.
inline constexpr float KILJAEDEN_KNOCKBACK_RADIUS = 9.0f;
// Bots go round him on this ring when a straight line would pass through the Knockback.
inline constexpr float KILJAEDEN_KNOCKBACK_CLEARANCE = KILJAEDEN_KNOCKBACK_RADIUS + 1.5f;
// A straight line that keeps at least this far from his center is taken as is.
inline constexpr float KILJAEDEN_ROUTE_LINE_CLEARANCE = KILJAEDEN_KNOCKBACK_RADIUS + 1.0f;
// How far past its tangent a bot going round him aims along the ring, in radians.
inline constexpr float KILJAEDEN_ROUTE_LEAD_ANGLE = 0.3f;

inline constexpr float HAND_SEARCH_RADIUS = 80.0f;
// Hammer of Justice is a single-target spell that counts both combat reaches so its actual range is
// its tooltip range of 10 yd + 1.5 (player) + 2.5 (Hand) = 14 yd. Holding a little inside that
// threshold leaves room for the Hand to shift between the range check and the cast landing.
inline constexpr float HAND_HOLY_PALADIN_STANDOFF = 12.0f;
// Timing between stuns for bots to coordinate them. A gate based purely on UNIT_STATE_STUNNED
// still results in spam stuns due to the delay between a spell casting and resolving.
inline constexpr uint32 HAND_CONTROL_CLAIM_MS = 1500;
// Hands cast Shadow Infusion (45772) at or below 20% HP, which makes them permanently immune
// to both stun and silence.
inline constexpr float HAND_CC_IMMUNE_HP_PERCENT = 20.0f;

// Throttle assigned ranged position rebuilds since they should be stable during the encounter.
inline constexpr uint32 KILJAEDEN_RANGED_ASSIGNMENT_REBUILD_INTERVAL_MS = 1000;
inline constexpr uint32 ARMAGEDDON_ASSIGNMENT_REBUILD_INTERVAL_MS = 250;
// Only up to two ranged bots may share a slot when an Armageddon forces a reshuffle.
inline constexpr uint8 KILJAEDEN_MAX_BOTS_PER_RANGED_SLOT = 2;
inline constexpr uint32 ARMAGEDDON_HAZARD_DURATION_MS = 10 * IN_MILLISECONDS;
inline constexpr float ARMAGEDDON_HAZARD_RADIUS = 9.0f; // no combat reaches added
inline constexpr float ARMAGEDDON_CLEARANCE = ARMAGEDDON_HAZARD_RADIUS + 2.0f;

inline constexpr float KILJAEDEN_REFLECTION_SEARCH_RADIUS = 100.0f;
inline constexpr float DRAGON_ORB_SEARCH_RADIUS = 200.0f;
inline constexpr float DRAGON_ORB_IN_USE_HOLD_DISTANCE = 15.0f;
// Shield of the Blue (45848) lasts 5s and Darkness of a Thousand Souls (46605) is an 8s channel, so
// the dragon casts once <4.5s remain.
inline constexpr int32 SHIELD_OF_THE_BLUE_CAST_WINDOW_MS = 4500;
// How far bots with Fire Bloom stay away from the Darkness stack until the Shield cast window.
inline constexpr float FIRE_BLOOM_STANDOFF = 15.0f;
// Breath: Haste and Breath: Revitalize are 13 yd cones on allies, so the dragon stops a little
// under half that from its target and looks for a cluster of roughly the same to cover at once.
inline constexpr float KILJAEDEN_DRAGON_BREATH_STANDOFF = 6.0f;
inline constexpr float KILJAEDEN_DRAGON_STANDOFF_TOLERANCE = 1.0f;
inline constexpr float KILJAEDEN_DRAGON_CLUSTER_RADIUS = 6.0f;
inline constexpr uint8 KILJAEDEN_DRAGON_MIN_CLUSTER_SIZE = 3;

inline Position const SUNWELL_CENTER_POSITION =       { 1698.450f, 628.030f, 28.199f };
// Also where the raid stacks for Shield of the Blue during Darkness of a Thousand Souls.
inline Position const KILJAEDEN_TANK_POSITION =       { 1705.876f, 636.145f, 27.836f };
inline Position const KILJAEDEN_S_MELEE_POSITION =    { 1684.160f, 630.488f, 27.215f };
inline Position const KILJAEDEN_E_MELEE_POSITION =    { 1702.162f, 614.013f, 27.516f };
// Melee with Fire Bloom wait here so that their group isn't splashed, and melee also go here when
// Armageddons target both melee positions.
inline Position const KILJAEDEN_MELEE_BACK_POSITION = { 1687.648f, 616.227f, 27.447f };

// Assigned in order, healers first: 0-6 healers, 7-8 an 8th and 9th healer, else ranged DPS.
// 11.5 yd apart: Fire Bloom on one bot, or an Armageddon or Shadow Spike at one, spares the rest.
inline std::array const KILJAEDEN_RANGED_POSITIONS = {
    Position{ 1714.670f, 628.720f, 27.326f },
    Position{ 1697.702f, 644.247f, 27.402f },
    Position{ 1717.152f, 639.949f, 28.405f },
    Position{ 1708.668f, 647.713f, 28.458f },
    Position{ 1700.184f, 655.477f, 28.050f },
    Position{ 1719.633f, 651.178f, 28.050f },
    Position{ 1711.149f, 658.942f, 28.050f },
    Position{ 1689.218f, 652.011f, 28.050f },
    Position{ 1725.635f, 632.186f, 28.050f },
    Position{ 1678.253f, 648.546f, 28.050f },
    Position{ 1723.154f, 620.957f, 28.050f },
    Position{ 1691.700f, 663.240f, 28.050f },
    Position{ 1736.601f, 635.651f, 28.050f },
    Position{ 1728.117f, 643.415f, 28.050f },
    Position{ 1702.665f, 666.706f, 28.050f },
    Position{ 1730.598f, 654.644f, 28.357f },
    Position{ 1722.115f, 662.408f, 28.246f },
    Position{ 1734.119f, 624.422f, 28.050f },
    Position{ 1720.672f, 609.728f, 28.050f },
};
inline constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    std::tuple_size_v<decltype(KILJAEDEN_RANGED_POSITIONS)>;

GuidVector FindKiljaedenHandGuids(Player* bot);
std::vector<Unit*> GetKiljaedenHands(PlayerbotAI* botAI);
bool IsKiljaedenHandControlClaimed(Unit* hand);
void ClaimKiljaedenHandControl(Unit* hand);
void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
bool IsOutsideKiljaedenArmageddons(uint32 instanceId, Position const& position);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
void EnsureKiljaedenRangedAssignments(Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(Player* bot);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
int32 GetKiljaedenDarknessTimeLeftMs(Unit* kiljaeden);
bool IsKiljaedenStartDpsHoldActive(PlayerbotAI* botAI);
bool IsLineThroughKiljaedenKnockback(Position const& from, Position const& to);
bool IsMeleeBlockedByKiljaedenKnockback(Position const& from, Position const& target);
bool GetKiljaedenStepToPosition(
    Player* bot, Position const& destination, float arrivalDist, float& stepX, float& stepY);
Position GetKiljaedenFireBloomWaitPosition(Player* bot);
bool CanClearKiljaedenFireBloomWithImmunity(Player* bot, Unit* kiljaeden);
GuidVector FindKiljaedenDragonOrbGuids(Player* bot);
bool UpdateKiljaedenDragonOrbUser(PlayerbotAI* botAI);
bool UpdateKiljaedenDragonOrbUserAnnouncement(PlayerbotAI* botAI);
Player* GetKiljaedenDragonOrbUser(Player* bot);
bool HasUsedKiljaedenDragonOrb(Player* bot);
bool HasKiljaedenDragonAura(Player* bot);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);
bool HasStaleRootFlag(Player* bot);
bool ReleaseStaleRootFlag(Player* bot);

// Shared encounter state

struct KiljaedenEncounterState
{
    std::vector<KiljaedenArmageddon> armageddons;
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    std::unordered_map<ObjectGuid, uint8> rangedArmageddonAssignments;
    std::unordered_map<ObjectGuid, uint32> handControlClaims;
    std::unordered_set<ObjectGuid> dragonOrbUsers;
    std::unordered_set<ObjectGuid> trackedArmageddonTargets;
    uint32 rangedAssignmentRebuildMs = 0;
    uint32 rangedArmageddonRebuildMs = 0;
    std::optional<uint32> dpsHoldStartMs;
};

KiljaedenEncounterState& KiljaedenState(uint32 instanceId);
KiljaedenEncounterState* FindKiljaedenState(uint32 instanceId);

}

#endif
