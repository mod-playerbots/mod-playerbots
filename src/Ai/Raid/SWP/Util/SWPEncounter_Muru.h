/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <optional>
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

inline constexpr float MURU_MISDIRECT_MIN_TARGET_HP_PERCENT = 80.0f;
// DPS cooldowns are held until 97% to allow for initial positioning.
inline constexpr float MURU_MAX_DPS_HP_PERCENT = 97.0f;

inline constexpr uint32 MURU_UNIT_CACHE_INTERVAL_MS = 200;
// Around MURU_CENTER_POSITION. The Shadowsword adds are summoned 55 yd away, and the Void Sentinel
// portals are within 41 yd.
inline constexpr float MURU_ROOM_SEARCH_RADIUS = 70.0f;

// Darkness cycle: 45998 ticks every 45s and triggers the 3s pre-effect 45999 on M'uru, whose own
// tick casts 45996, a zone at M'uru's position doing 3k a second for 20s. The zone is M'uru's
// dynamic object, so it outlasts the aura wipe at the transition to Entropius.
inline constexpr uint32 MURU_DARKNESS_PRE_EFFECT_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 MURU_DARKNESS_DURATION_MS = 20 * IN_MILLISECONDS;
// This is an arbitrary window to allow tanks a bit more time to get positioned after Darkness.
inline constexpr uint32 MURU_DARKNESS_EARLY_WINDOW_MS = 10 * IN_MILLISECONDS;
// The zone hits a player whose center is within 16.5 yd of M'uru's (15 yd plus the player's reach).
inline constexpr float MURU_DARKNESS_RADIUS = 15.0f;
inline constexpr float MURU_DARKNESS_CLEARANCE = MURU_DARKNESS_RADIUS + 5.0f;
// Melee don't attack a unit this close to M'uru during Darkness: 1.5 yd for their reach, 5 yd for
// melee range, and a 1.5 yd margin.
inline constexpr float MURU_DARKNESS_MELEE_TARGET_DISTANCE = MURU_DARKNESS_RADIUS + 8.0f;
// Tanks won't try to pick up anything farther than this distance from the ranged stack.
inline constexpr float MURU_MAX_TARGET_DIST_FROM_STACK = 30.0f;
// The maximum distance from the melee dps holding spot that they wander to attack during Darkness.
inline constexpr float MURU_HOLDING_POSITION_RADIUS = 25.0f;
// Targeting is based on the nearest mob; this buffer is to keep targets sticky.
inline constexpr float MURU_TARGET_SWITCH_MARGIN = 10.0f;
// Radius of Shadow Bolt Volley (46082), which is centered on the enslaved Void Spawn.
inline constexpr float MURU_SHADOW_BOLT_VOLLEY_RADIUS = 20.0f;
// Shadow Pulse (46087) is a 10 yd AoE centered on the Void Sentinel, pulsed every 3s by 46086.
// Melee end up ~4 yd from their target, and there's also variability from the tank moving adds.
inline constexpr float MURU_MELEE_ADD_MIN_DIST_FROM_SENTINEL = 15.0f;

// A Dark Fiend detonates when it comes within 7 yd (center to center) of the player it chases.
// Everyone keeps clear of it, as touching one is almost a guaranteed wipe.
inline constexpr float DARK_FIEND_BLAST_RANGE = 7.0f;
inline constexpr float DARK_FIEND_CLEARANCE = DARK_FIEND_BLAST_RANGE + 5.0f;
// A void zone (25879) hits within 3 yd of its center (46264). For its first 3s it carries 46265,
// which summons a Dark Fiend at its center when it ends, so a new zone is kept at the fiend's
// clearance.
inline constexpr float MURU_VOID_ZONE_HAZARD_RADIUS = 3.0f;
inline constexpr float MURU_VOID_ZONE_CLEARANCE = MURU_VOID_ZONE_HAZARD_RADIUS + 1.5f;
// How far a bot looks for a position clear of every void zone and fiend.
inline constexpr float MURU_CLEAR_POSITION_SEARCH_RADIUS = 15.0f;
inline constexpr float MURU_CLEAR_POSITION_ARRIVAL_DISTANCE = 0.5f;
// A Singularity throws everyone within 7 yd of its center (46230) from 8s after it appears. Bots
// leave from the moment it appears, for time to react. Entropius's victim keeps farther away, so
// melee on Entropius have room.
inline constexpr float SINGULARITY_HAZARD_RADIUS = 7.0f;
inline constexpr float SINGULARITY_CLEARANCE = SINGULARITY_HAZARD_RADIUS + 3.0f;
inline constexpr float SINGULARITY_TANK_CLEARANCE = 20.0f;
inline constexpr float SINGULARITY_SEARCH_RADIUS = 30.0f;
// Hunters and shamans leave a new Spell Fury this long for a mage to Spellsteal, which allows for a
// mage's global cooldown.
inline constexpr uint32 SPELL_FURY_MAGE_PRIORITY_MS = 2 * IN_MILLISECONDS;
// DPS hold Entropius this long after he first appears, so the main tank gets aggro first.
inline constexpr uint32 ENTROPIUS_DPS_HOLD_MS = 5 * IN_MILLISECONDS;

inline constexpr float MURU_RANGED_STACK_ARRIVAL_DISTANCE = 3.5f;
inline constexpr float MURU_MELEE_STACK_ARRIVAL_DISTANCE = 4.5f;
inline constexpr float MURU_TANK_STACK_ARRIVAL_DISTANCE = 3.0f;
inline constexpr float MURU_TANK_HOLDING_ARRIVAL_DISTANCE = 2.5f;
inline constexpr float MURU_ENTRANCE_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float MURU_VOID_SENTINEL_TANK_ARRIVAL_DISTANCE = 2.0f;
inline constexpr float ENTROPIUS_RANGED_ARRIVAL_DISTANCE = 2.0f;

inline Position const MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };
inline Position const MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
inline Position const MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
inline Position const MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
inline Position const MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };

bool IsMuruPhaseActive(Unit* muru);
bool IsMuruDarknessActive(Unit* muru);
bool IsMuruDarknessEarly(Unit* muru);
bool IsMeleeTargetInMuruDarkness(Unit* muru, Unit* target);
// Phase 1 Darkness, from the pre-effect, whatever the owner's role.
bool IsMuruPetHoldActive(Unit* muru);
// The owner's current target, but never a Dark Fiend, and during Darkness never one inside it.
Unit* GetMuruPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* muru);
bool IsSpellFuryReservedForMages(Unit* furyMage);
bool IsEntropiusDpsHoldActive(Player* bot);
// M'uru, Entropius, their adds, the void zones (Darkness), the Dark Fiends, and the Singularity.
GuidVector FindMuruUnitGuids(Player* bot);
GuidVector const& GetMuruUnits(Player* bot);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Unit* SelectNearestMuruTargetByEntry(
    Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates,
    Position const& origin);
bool IsMuruAddInVoidSentinelPulse(Unit* add, std::vector<Unit*> const& voidSentinels);
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI);
Position const& GetAssignedVoidSentinelTankPosition(Unit* voidSentinel);
bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI);
// The nearest to the bot within SINGULARITY_SEARCH_RADIUS.
Creature* GetMuruSingularity(Player* bot);
std::vector<Creature*> GetMuruDarkFiends(PlayerbotAI* botAI);
Creature* GetNearestMuruDarkFiend(PlayerbotAI* botAI, float radius);
float GetMuruVoidZoneClearance(Unit* voidZone);
Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI);
Creature* FindMuruDarkFiendToAvoid(PlayerbotAI* botAI);
bool TryGetMuruClearPosition(PlayerbotAI* botAI, Position& destination);
float GetSingularityClearance(Player* bot, Unit* entropius);
Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI);

// Shared encounter state

struct MuruEncounterState
{
    std::unordered_map<ObjectGuid, uint8> voidSentinelTankAssignments;
    std::optional<uint32> entropiusFirstSeenMs;
};

MuruEncounterState& MuruState(uint32 instanceId);
MuruEncounterState* FindMuruState(uint32 instanceId);

}

#endif
