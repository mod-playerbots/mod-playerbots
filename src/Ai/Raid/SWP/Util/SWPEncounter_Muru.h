/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPSharedConstants.h"
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct MuruEncounterTargets
{
    Unit* muru = nullptr;
    Unit* entropius = nullptr;
    std::vector<Unit*> voidSentinels;
    std::vector<Unit*> voidSpawns;
    std::vector<Unit*> furyMages;
    std::vector<Unit*> berserkers;
};

// What the "muru encounter targets" value stores
struct MuruEncounterGuids
{
    ObjectGuid muru;
    ObjectGuid entropius;
    GuidVector voidSentinels;
    GuidVector voidSpawns;
    GuidVector furyMages;
    GuidVector berserkers;
};

struct MuruDarknessState
{
    uint32 startMs = 0;
    uint32 expireMs = 0;
};

inline Position const MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };
inline Position const MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
inline Position const MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
inline Position const MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
inline Position const MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };

// Ability reaches from SpellRange.dbc (MaxRangeHostile). Berserker and Fury Mage selection filters
// candidates based on these distances.
inline constexpr float MURU_MELEE_ABILITY_REACH = 5.0f;
inline constexpr float MURU_WAR_STOMP_REACH = 8.0f;
inline constexpr float MURU_HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float MURU_WIND_SHEAR_REACH = 25.0f;
inline constexpr float MURU_RANGED_ABILITY_REACH = 30.0f;
inline constexpr float MURU_SILENCING_SHOT_REACH = 35.0f;

// For the "muru encounter targets" value. Only list membership is cached, not states read (like
// auras, casting, health).
inline constexpr uint32 MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS = 200;

// Darkness cycle: 45998 ticks every 45s and triggers the 3s pre-effect 45999, whose own tick casts
// 45996, a 15y zone doing 3k a second. 45996 is also applied to M'uru itself (via a separate
// effect), so once it is applied, the Darkness window is read off that aura and these two are only
// estimates used before the aura is applied.
inline constexpr uint32 MURU_DARKNESS_PRE_EFFECT_MS = 3000;
inline constexpr uint32 MURU_DARKNESS_AURA_MS = 20000;

// This long before Darkness expires, melee are allowed to run back in (to account for run distance,
// finishing current actions, tick delay).
inline constexpr uint32 MURU_DARKNESS_RUN_BACK_ALLOWANCE_MS = 2000;

// This is an arbitrary window to allow tanks a bit more time to get positioned after Darkness.
inline constexpr uint32 MURU_DARKNESS_EARLY_WINDOW_MS = 10000;

// Darkness damages within 15 yards of M'uru; the rest is the usual avoidance padding
inline constexpr float MURU_DARKNESS_SAFE_DISTANCE = 20.0f;

// The maximum distance from the melee dps holding spot that melee dps will wander to attack during
// Darkness
inline constexpr float MURU_HOLDING_POSITION_RADIUS = 20.0f;

// Targeting is based on the nearest mob; this buffer is to keep targets sticky
inline constexpr float MURU_TARGET_SWITCH_MARGIN = 10.0f;

// Radius of Shadow Bolt Volley (46082), which is centred on the enslaved Void Spawn
inline constexpr float MURU_SHADOW_BOLT_VOLLEY_RADIUS = 20.0f;

// Void Zones (25879) have aura 46262, ticking 46264 for 3k in a 3y radius, but more importantly
// they spawn Dark Fiends. The wide safe distance is in anticipation of the Dark Fiend spawn.
inline constexpr float MURU_VOID_ZONE_SAFE_DISTANCE = 8.0f;
inline constexpr float MURU_VOID_ZONE_SEARCH_RADIUS = 12.0f;

// Feeds the "muru void zones" value.
inline constexpr uint32 MURU_VOID_ZONE_CACHE_INTERVAL_MS = 200;

inline constexpr float MURU_SINGULARITY_SEARCH_RADIUS = 30.0f;

// Feeds the "muru singularity" value. Only one exists at a time: Entropius casts Black Hole every
// 29s, and Singularities despawn after 18s.
inline constexpr uint32 MURU_SINGULARITY_CACHE_INTERVAL_MS = 200;

// A Dark Fiend detonates within 2y of whoever it is chasing. The safe distance is deliberately
// wide as touching a single Dark Fiend is almost a guaranteed wipe.
inline constexpr float MURU_DARK_FIEND_SAFE_DISTANCE = 10.0f;

// Tanks drag nothing further than this from the ranged stack
inline constexpr float MURU_MAX_TARGET_DIST_FROM_STACK = 25.0f;

inline constexpr float MURU_MISDIRECT_MIN_TARGET_HP_PERCENT = 80.0f;

// Dps cooldowns are held until 97% to allow for initial positioning
inline constexpr float MURU_MAX_DPS_HP_PERCENT = 97.0f;

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    muruVoidSentinelTankAssignments;

bool IsMuruPhaseActive(Unit* muru);
bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool TryGetMuruDarknessEarlyState(
    Player* bot, Unit* muru, uint32 earlyWindowMs = MURU_DARKNESS_EARLY_WINDOW_MS);
MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI);
bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI);
GuidVector FindMuruVoidZoneGuids(Player* bot);
ObjectGuid FindMuruSingularityGuid(Player* bot);
Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI);
Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI);

}

#endif
