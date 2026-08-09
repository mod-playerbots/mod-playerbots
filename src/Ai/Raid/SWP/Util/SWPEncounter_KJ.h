/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKJ_H
#define PLAYERBOTS_SWPENCOUNTERKJ_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
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

struct KiljaedenDarknessShieldState
{
    bool inDarkness = false;
    bool shieldCastThisDarkness = false;
    uint32 darknessStartMs = 0;
    uint32 lastDarknessCastMsLeft = 0;
};

struct KiljaedenEncounterState
{
    std::vector<KiljaedenArmageddon> armageddons;
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    std::unordered_map<ObjectGuid, uint8> rangedArmageddonAssignments;
    uint32 dragonOrbAnnouncementMs = 0;
};

inline std::array const KILJAEDEN_DRAGON_ORB_ENTRIES = {
    Id(SwpObjects::GO_DRAGON_ORB_1),
    Id(SwpObjects::GO_DRAGON_ORB_2),
    Id(SwpObjects::GO_DRAGON_ORB_3),
    Id(SwpObjects::GO_DRAGON_ORB_4),
};

inline constexpr uint32 KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS = 10000;
inline constexpr float KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE = 11.0f;

inline constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
inline constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
inline constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
inline constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
inline constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
inline constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

inline Position const KILJAEDEN_CENTER_POSITION =   { 1698.450f, 628.030f, 28.199f };
inline Position const KILJAEDEN_TANK_POSITION =     { 1704.729f, 634.891f, 27.787f };
inline Position const KILJAEDEN_S_MELEE_POSITION =  { 1689.487f, 632.119f, 27.823f };
inline Position const KILJAEDEN_E_MELEE_POSITION =  { 1700.542f, 619.589f, 27.786f };
inline Position const KILJAEDEN_DARKNESS_POSITION = { 1709.768f, 642.241f, 27.706f };

extern std::unordered_set<ObjectGuid> kiljaedenTrackedArmageddonTargets;
extern std::unordered_map<uint32, KiljaedenEncounterState> kiljaedenEncounterStates;
extern std::unordered_map<uint32, std::array<ObjectGuid, 3>> kiljaedenHandTankAssignments;
extern std::unordered_map<ObjectGuid::LowType, uint32> kiljaedenDragonOrbUseTimes;

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
void EnsureKiljaedenRangedAssignments(Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(Player* bot);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
Player* GetKiljaedenDragonOrbUser(Player* bot);
bool ResetKiljaedenDragonOrbUserAnnouncement(uint32 instanceId);
bool HasRecentKiljaedenDragonOrbUse(Player* bot, uint32 recentMs);
bool HasKiljaedenDragonAura(Player* bot);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);

}

#endif
