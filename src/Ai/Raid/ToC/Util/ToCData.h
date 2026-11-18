/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCDATA_H
#define PLAYERBOTS_RAID_TOCDATA_H

#include <algorithm>

#include "Define.h"
#include "ObjectDefines.h"
#include "Position.h"

struct RaidDefensiveScope;

// Raid-wide values only. Boss-specific ids, constants and positions go in that boss's
// Util/ToCHelpers_<Stem>.h.
namespace TrialOfTheCrusaderHelpers
{

inline constexpr uint32 TRIAL_OF_THE_CRUSADER_MAP_ID = 649;

// Unit::GetMeleeRange for a player on a unit of this combat reach, centre to centre. A boss's reach is
// creature_model_info's CombatReach for its display, and core DB updates move it.
constexpr float PlayerMeleeRange(float unitReach)
{
    return std::max(unitReach + DEFAULT_COMBAT_REACH + 4.0f / 3.0f, NOMINAL_MELEE_RANGE);
}

// Covers and personal picks on map 649
extern RaidDefensiveScope const TOC_DEFENSIVE_SCOPE;

// Center of the Crusaders' Coliseum arena (trial_of_the_crusader.h Locs[LOC_CENTER])
extern const Position ARENA_CENTER;

// Boss entries from trial_of_the_crusader.h NPCs, adds and hazards from the NPC enum of the boss
// script named on each section.
enum class ToCNpcs : uint32
{
    // Northrend Beasts (boss_northrend_beasts.cpp GormokNPCs and JormungarNPCs)
    NPC_GORMOK          = 34796,
    NPC_ACIDMAW         = 35144,
    NPC_DREADSCALE      = 34799,
    NPC_ICEHOWL         = 34797,
    NPC_SNOBOLD_VASSAL  = 34800,
    NPC_SLIME_POOL      = 35176, // Acidmaw/Dreadscale slime pool: a persistent ground hazard

    // Lord Jaraxxus (boss_lord_jaraxxus.cpp JaraxxusNPCs)
    NPC_JARAXXUS        = 34780,
    NPC_MISTRESS_OF_PAIN = 34826,
    NPC_FEL_INFERNAL    = 34815,
    NPC_LEGION_FLAME    = 34784,
    NPC_NETHER_PORTAL   = 34825,
    NPC_INFERNAL_VOLCANO = 34813,

    // Anub'arak (boss_anubarak_trial.cpp AnubNPCs)
    NPC_ANUBARAK            = 34564,
    NPC_FROST_SPHERE        = 34606, // flying sphere; becomes a grounded Permafrost patch when killed
    NPC_NERUBIAN_BURROWER   = 34607, // phase 1 add
    NPC_SWARM_SCARAB        = 34605, // submerge-phase add
    NPC_PURSUING_SPIKE      = 34660, // submerge-phase chase mob

    // Twin Val'kyr (boss_twin_valkyr.cpp ValkyrNPCs)
    NPC_FJOLA_LIGHTBANE     = 34497, // the Light twin (controls the encounter, casts Light Vortex)
    NPC_EYDIS_DARKBANE      = 34496, // the Dark twin (casts Dark Vortex)
    NPC_LIGHT_ESSENCE       = 34568, // portal NPC; its gossip grants the Light Essence aura
    NPC_DARK_ESSENCE        = 34567, // portal NPC; its gossip grants the Dark Essence aura
};

// From the spell enum of the boss script named on each section, as the 10N id. Every cast carries
// the map difficulty's id, so read a remapped one through sSpellMgr->GetSpellIdForDifficulty.

// Northrend Beasts: Gormok the Impaler (boss_northrend_beasts.cpp GormokSpells)
inline constexpr uint32 SPELL_IMPALE            = 66331; // stacking bleed on the current tank, drives the tank swap

// Acidmaw & Dreadscale (boss_northrend_beasts.cpp JormungarSpells)
// 15 yd knockback circle around the worm (DBC radius index 18)
inline constexpr uint32 SPELL_SWEEP             = 66794;

// Lord Jaraxxus (boss_lord_jaraxxus.cpp JaraxxusSpells)
inline constexpr uint32 SPELL_FEL_FIREBALL      = 66532; // interruptible cast on the current tank
inline constexpr uint32 SPELL_INCINERATE_FLESH  = 66237; // heal-absorb debuff on a random player
inline constexpr uint32 SPELL_LEGION_FLAME      = 66197; // spawns the pursuing ground fire

// Nether Power: stacking spell-power buff on the boss
inline constexpr uint32 SPELL_NETHER_POWER_10N  = 66228;
inline constexpr uint32 SPELL_NETHER_POWER_25N  = 67106;
inline constexpr uint32 SPELL_NETHER_POWER_10H  = 67107;
inline constexpr uint32 SPELL_NETHER_POWER_25H  = 67108;

// Anub'arak (boss_anubarak_trial.cpp AnubSpells)
inline constexpr uint32 SPELL_MARK              = 67574; // on the player the Pursuing Spike is chasing
inline constexpr uint32 SPELL_LEECHING_SWARM    = 66118; // P3 area aura, carried by the players and never by the boss
inline constexpr uint32 SPELL_SUBMERGE_ANUB     = 65981; // boss submerge aura (P2)

// Twin Val'kyr (boss_twin_valkyr.cpp ValkyrSpells). Essences are player auras from the portal
// NPCs' gossip.
inline constexpr uint32 SPELL_LIGHT_ESSENCE     = 65686; // matches Light Vortex / Light Touch
inline constexpr uint32 SPELL_DARK_ESSENCE      = 65684; // matches Dark Vortex / Dark Touch
// Vortex casts, read from the casting twin's current spell
inline constexpr uint32 SPELL_LIGHT_VORTEX      = 66046;
inline constexpr uint32 SPELL_DARK_VORTEX       = 66058;
// Touch DoTs, heroic only. The script casts the 10H ids 67297/67282, same difficulty rows.
inline constexpr uint32 SPELL_LIGHT_TOUCH       = 65950;
inline constexpr uint32 SPELL_DARK_TOUCH        = 66001;
// Twin's Pact heal-to-full 15 s cast (Fjola casts Light, Eydis Dark)
inline constexpr uint32 SPELL_LIGHT_TWIN_PACT   = 65876;
inline constexpr uint32 SPELL_DARK_TWIN_PACT    = 65875;

}

#endif
