/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPSHAREDCONSTANTS_H
#define PLAYERBOTS_SWPSHAREDCONSTANTS_H

#include "Common.h"
#include <type_traits>

namespace SwpHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class SwpSpells : uint32
{
    // Trash - Apocalypse Guard
    SPELL_INFERNAL_DEFENSE             = 46287,

    // Kalecgos
    SPELL_SPECTRAL_EXHAUSTION          = 44867,
    SPELL_SPECTRAL_BLAST_PORTAL        = 44866,
    SPELL_ARCANE_BUFFET                = 45018,
    SPELL_CURSE_OF_BOUNDLESS_AGONY     = 45032,
    SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC = 45034,
    SPELL_TELEPORT_SPECTRAL            = 46019,
    SPELL_SPECTRAL_REALM               = 46021,

    // Brutallus
    SPELL_METEOR_SLASH                 = 45150, // 120° cone
    SPELL_BURN                         = 46394, // Spread radius is 2y, no CombatReaches added

    // Felmyst
    SPELL_SUMMON_DEMONIC_VAPOR         = 45391,
    SPELL_ENCAPSULATE                  = 45661,
    SPELL_GAS_NOVA                     = 45855,
    SPELL_FELMYST_SPEED_BURST          = 45495,
    SPELL_FOG_OF_CORRUPTION            = 45582,
    SPELL_FOG_OF_CORRUPTION_CHARM      = 45717,
    SPELL_FELMYST_STRAFE_TOP           = 45585,
    SPELL_FELMYST_STRAFE_MIDDLE        = 45633,
    SPELL_FELMYST_STRAFE_BOTTOM        = 45635,

    // Eredar Twins
    SPELL_BLAZE                        = 45235,
    SPELL_CONFLAGRATION                = 45342,
    SPELL_FLAME_TOUCHED                = 45348,
    SPELL_FLAME_SEAR                   = 46771,

    // M'uru
    SPELL_DARKNESS                     = 45996,
    SPELL_DARKNESS_PRE_EFFECT          = 45999,
    SPELL_ENTROPIUS_DARKNESS           = 46269,
    SPELL_SHADOW_BOLT_VOLLEY           = 46082,
    SPELL_FEL_FIREBALL                 = 46101,
    SPELL_SPELL_FURY                   = 46102,
    SPELL_FLURRY                       = 46160,

    // Kil'jaeden <The Deceiver>
    SPELL_FIRE_BLOOM                   = 45641,
    SPELL_SHIELD_OF_THE_BLUE           = 45848,
    SPELL_DRAGON_BREATH_HASTE          = 45856,
    SPELL_DRAGON_BREATH_REVITALIZE     = 45860,
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT = 45839,
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS = 46605,
    SPELL_SHADOW_SPIKE                 = 46680,

    // Hunter
    SPELL_MISDIRECTION                 = 35079,

    // Mage
    SPELL_SPELLSTEAL                   = 30449,
    SPELL_ICE_BLOCK                    = 45438,

    // Paladin
    SPELL_DIVINE_SHIELD                = 642,

    // Priest
    SPELL_DISPEL_MAGIC_RANK_1          = 527,
    SPELL_SHADOWFORM                   = 15473,
    SPELL_MASS_DISPEL                  = 32375,

    // Rogue
    SPELL_CLOAK_OF_SHADOWS             = 31224,

    // Shaman
    SPELL_PURGE_RANK_1                 = 370,

    // Warlock
    SPELL_METAMORPHOSIS                = 47241,
};

enum class SwpNpcs : uint32
{
    // Trash
    NPC_APOCALYPSE_GUARD         = 25593,
    NPC_VOLATILE_FIEND           = 25851,

    // Kalecgos
    NPC_KALECGOS_DRAGON          = 24850,
    NPC_KALECGOS_HUMANOID        = 24891,

    // Felmyst
    NPC_FELMYST                  = 25038,
    NPC_DEMONIC_VAPOR            = 25265, // The vapor "head" that chases a player
    NPC_DEMONIC_VAPOR_TRAIL      = 25267,

    // Eredar Twins
    NPC_GRAND_WARLOCK_ALYTHESS   = 25166,

    // M'uru
    NPC_MURU                     = 25741,
    NPC_VOID_SENTINEL            = 25772,
    NPC_DARK_FIEND               = 25744,
    NPC_DARKNESS                 = 25879,
    NPC_SHADOWSWORD_BERSERKER    = 25798,
    NPC_SHADOWSWORD_FURY_MAGE    = 25799,
    NPC_VOID_SPAWN               = 25824,
    NPC_ENTROPIUS                = 25840,
    NPC_SINGULARITY              = 25855,

    // Kil'jaeden <The Deceiver>
    NPC_SHIELD_ORB               = 25502,
    NPC_HAND_OF_THE_DECEIVER     = 25588,
    NPC_POWER_OF_THE_BLUE_FLIGHT = 25653,
    NPC_SINISTER_REFLECTION      = 25708,
    NPC_ARMAGEDDON_TARGET        = 25735,
};

enum class SwpObjects : uint32
{
    // Kalecgos
    GO_SPECTRAL_RIFT = 187055,

    // Eredar Twins
    GO_BLAZE         = 187366,

    // Kil'jaeden <The Deceiver>
    GO_DRAGON_ORB_1  = 187869,
    GO_DRAGON_ORB_2  = 188114,
    GO_DRAGON_ORB_3  = 188115,
    GO_DRAGON_ORB_4  = 188116,
};

inline constexpr uint32 SWP_MAP_ID = 580;
inline constexpr float SWP_PULL_COMPLETE_HP_PERCENT = 95.0f;

// Feeds the "swp volatile fiend" value.
inline constexpr uint32 VOLATILE_FIEND_CACHE_INTERVAL_MS = 200;
// Shared by the trigger and the action so the two cannot drift apart.
inline constexpr float VOLATILE_FIEND_SEARCH_RADIUS = 25.0f;

}

#endif
