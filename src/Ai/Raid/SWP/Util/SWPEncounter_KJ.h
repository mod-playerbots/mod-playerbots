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
#include <unordered_map>
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

constexpr uint32 KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS = 10000;
constexpr float KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE = 11.0f;
constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;
extern uint32 const KILJAEDEN_DRAGON_ORB_ENTRIES[4];

extern Position const KILJAEDEN_CENTER_POSITION;
extern Position const KILJAEDEN_TANK_POSITION;
extern Position const KILJAEDEN_S_MELEE_POSITION;
extern Position const KILJAEDEN_E_MELEE_POSITION;
extern Position const KILJAEDEN_DARKNESS_POSITION;

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
