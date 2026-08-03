/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAHELPERS_H
#define PLAYERBOTS_KARAHELPERS_H

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"
#include <array>
#include <ctime>
#include <unordered_map>

namespace KaraHelpers
{

enum class KaraSpells : uint32
{
    // Maiden of Virtue
    SPELL_REPENTANCE              = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD  = 30756,

    // The Curator
    SPELL_CURATOR_EVOCATION       = 30254,

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST       = 30004,
    SPELL_FLAME_WREATH_AURA       = 29946,
    SPELL_BLIZZARD                = 29951,
    SPELL_ARCANE_EXPLOSION        = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF         = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF       = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF        = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_NETHER_EXHAUSTION_RED   = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE  = 38639,
    SPELL_NETHERSPITE_BANISHED    = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH           = 30129,
    SPELL_RAIN_OF_BONES           = 37091,

    // Priest
    SPELL_FEAR_WARD               = 6346,

    // Shaman
    SPELL_TREMOR_TOTEM            = 8143,
    SPELL_GROUNDING_TOTEM         = 8177,
};

enum class KaraNpcs : uint32
{
    // Trash
    NPC_MANA_WARP                 = 16530,

    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN      = 16152, // ID for mounted version

    // Terestian Illhoof
    NPC_TERESTIAN_ILLHOOF         = 15688,
    NPC_DEMON_CHAINS              = 17248,
    NPC_KILREK                    = 17229,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL        = 17167,

    // Netherspite
    NPC_VOID_ZONE                 = 16697,
    NPC_GREEN_PORTAL              = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL               = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL      = 17646,
};

// General
constexpr uint32 KARA_MAP_ID = 532;
bool IsSafePosition (float x, float y, const std::vector<Unit*>& hazards, float hazardRadius);

// Attumen the Huntsman
extern Position const ATTUMEN_TANK_POSITION;
extern std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
Unit* GetAttumenMounted(Player* bot);

// Maiden of Virtue
extern Position const MAIDEN_OF_VIRTUE_TANK_POSITION;
extern std::array<Position, 8> const MAIDEN_OF_VIRTUE_RANGED_POSITIONS;

// The Big Bad Wolf
extern Position const BIG_BAD_WOLF_TANK_POSITION;
extern std::array<Position, 4> const BIG_BAD_WOLF_RUN_POSITIONS;

// Wizard of Oz
std::array<const char*, 5> const& GetOzTargets();

// The Curator
extern Position const THE_CURATOR_TANK_POSITION;

// Shade of Aran
bool IsAranCastingArcaneExplosion(Unit* aran);
bool IsFlameWreathActive(Player* bot);

// Netherspite
extern std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
bool IsBanishPhase(Unit* netherspite);
std::vector<Player*> GetRedBlockers(Player* bot);
std::vector<Player*> GetBlueBlockers(Player* bot);
std::vector<Player*> GetGreenBlockers(Player* bot);
std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot);
std::vector<Unit*> GetAllVoidZones(Player* bot);
bool FindBeamPosition(
    Unit* netherspite, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos);

// Prince Malchezaar
std::vector<Unit*> GetSpawnedInfernals(Player* bot);
bool IsStraightPathSafe(
    float startX, float startY, float targetX, float targetY,
    std::vector<Unit*> const& hazards, float hazardRadius);
bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY);

// Nightbane
constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
constexpr float NIGHTBANE_GROUND_Z = 91.473f;
extern Position const TERRACE_DOME_CENTER;
extern Position const TERRACE_EAST_END;
extern Position const TERRACE_WEST_END;
extern std::array<Position, 2> const NIGHTBANE_FLIGHT_STACK_POSITIONS;
extern std::array<Position, 2> const NIGHTBANE_RAIN_OF_BONES_POSITIONS;
extern Position const NIGHTBANE_TELEPORT_POSITION;
extern std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
extern std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

}

#endif
