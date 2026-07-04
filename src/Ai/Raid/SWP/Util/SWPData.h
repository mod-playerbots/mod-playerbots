/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SWPDATA_H
#define PLAYERBOTS_SWPDATA_H

#include "Common.h"

namespace SunwellHelpers
{

enum class SunwellSpells : uint32
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
    SPELL_TELEPORT_NORMAL_REALM        = 46020,
    SPELL_SPECTRAL_REALM               = 46021,

    // Brutallus
    SPELL_METEOR_SLASH                 = 45150,
    SPELL_BURN                         = 46394,

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
    SPELL_ICE_BLOCK                    = 45438,

    // Paladin
    SPELL_DIVINE_SHIELD                = 642,

    // Priest
    SPELL_SHADOWFORM                   = 15473,

    // Warlock
    SPELL_METAMORPHOSIS                = 47241,
};

enum class SunwellNpcs : uint32
{
    // Trash
    NPC_APOCALYPSE_GUARD        = 25593,
    NPC_VOLATILE_FIEND          = 25851,

    // Kalecgos
    NPC_KALECGOS_HUMANOID       = 24891,

    // Felmyst
    NPC_FELMYST                 = 25038,
    NPC_DEMONIC_VAPOR           = 25265,
    NPC_DEMONIC_VAPOR_TRAIL     = 25267,

    // Eredar Twins
    NPC_GRAND_WARLOCK_ALYTHESS  = 25166,

    // M'uru
    NPC_MURU                    = 25741,
    NPC_VOID_SENTINEL           = 25772,
    NPC_DARK_FIEND              = 25744,
    NPC_DARKNESS                = 25879,
    NPC_SHADOWSWORD_BERSERKER   = 25798,
    NPC_SHADOWSWORD_FURY_MAGE   = 25799,
    NPC_VOID_SPAWN              = 25824,
    NPC_ENTROPIUS               = 25840,
    NPC_SINGULARITY             = 25855,

    // Kil'jaeden <The Deceiver>
    NPC_SHIELD_ORB              = 25502,
    NPC_HAND_OF_THE_DECEIVER    = 25588,
    NPC_FELFIRE_PORTAL          = 25603,
    NPC_SINISTER_REFLECTION     = 25708,
    NPC_ARMAGEDDON_TARGET       = 25735,
};

enum class SunwellObjects : uint32
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

constexpr uint32 SUNWELL_MAP_ID = 580;

}

#endif
