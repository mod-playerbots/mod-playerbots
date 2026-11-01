/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLHELPERS_H
#define PLAYERBOTS_BWLHELPERS_H

#include "Player.h"
#include "PlayerbotAI.h"
#include <vector>

namespace BlackwingLairHelpers
{
    enum class BlackwingLairSpells : uint32
    {
        // General
        SPELL_ONYXIA_SCALE_CLOAK = 22683,

        // Razorgore the Untamed
        SPELL_MINDCONTROL = 19832,

        // Vaelastrasz the Corrupt
        SPELL_BURNING_ADRENALINE = 18173,

        // Chromaggus
        SPELL_BROOD_AFFLICTION_BRONZE = 23170,
        SPELL_HOURGLASS_SAND = 23645,

        // Nefarian
        SPELL_WILD_MAGIC = 23410,
        SPELL_CORRUPTED_HEALING = 23401,

        // Death Talon Wyrmguard
        SPELL_RED_BROOD_POWER = 22283,
        SPELL_BLUE_BROOD_POWER = 22285,
        SPELL_BRONZE_BROOD_POWER = 22286,
        SPELL_GREEN_BROOD_POWER = 22288,

        // Priest talent and glyph
        SPELL_EMPOWERED_RENEW_R1 = 63534,
        SPELL_EMPOWERED_RENEW_R2 = 63542,
        SPELL_EMPOWERED_RENEW_R3 = 63543,
        SPELL_GLYPH_OF_POWER_WORD_SHIELD = 55672
    };

    enum class BlackwingLairGameObjects : uint32
    {
        // General
        GO_SUPPRESSION_DEVICE = 179784,

        // Razorgore the Untamed
        GO_BLACK_DRAGON_EGG = 177807
    };

    enum class BlackwingLairNPCs : uint32
    {
        // Broodlord Lashlayer
        NPC_CORRUPTED_RED_WHELP = 14022,
        NPC_CORRUPTED_GREEN_WHELP = 14023,
        NPC_CORRUPTED_BLUE_WHELP = 14024,
        NPC_CORRUPTED_BRONZE_WHELP = 14025,

        // Nefarian
        NPC_CORRUPTED_FIRE_NOVA_TOTEM = 14662,
        NPC_CORRUPTED_STONESKIN_TOTEM = 14663,
        NPC_CORRUPTED_HEALING_TOTEM = 14664,
        NPC_CORRUPTED_WINDFURY_TOTEM = 14666,

        // Black drakes and Chromaggus
        NPC_FIREMAW = 11983,
        NPC_FLAMEGOR = 11981,
        NPC_EBONROC = 14601,
        NPC_CHROMAGGUS = 14020,

        // Trash
        NPC_DEATH_TALON_WYRMGUARD = 12460
    };

    constexpr float BROODLORD_SAFE_DISTANCE = 18.0f;

    bool IsActiveSuppressionDeviceInRange(GameObject const* go, Player const* bot);
    bool AreRazorgoreEggsAlive(PlayerbotAI* botAI);
    bool IsRazorgoreOffTank(Player* bot);
    bool IsNonBABotNearPosition(Player const* bot, Position const& position, float distance);
    bool IsCorruptedWhelp(Unit const* unit);
    bool IsDeathTalonWyrmguard(Unit const* unit);
    std::vector<Unit*> GetWyrmguardAttackers(PlayerbotAI* botAI);
    Unit* GetAssignedWyrmguard(PlayerbotAI* botAI);
    Unit* GetWyrmguardKillTarget(std::vector<Unit*> const& wyrmguards);
    bool IsAssignedToOtherTank(Player* bot, Unit const* wyrmguard);
    Position const* GetWyrmguardSpot(Player* bot, Unit const* wyrmguard);
    bool IsWyrmguardHoldRunning(Player* bot);
    bool AreWyrmguardsInPosition(Player* bot, std::vector<Unit*> const& wyrmguards);
    std::vector<Unit*> GetUnengagedDrakes(Player* bot);
    bool IsStepTowardsDrake(Player const* bot, std::vector<Unit*> const& drakes, float x, float y);
    bool IsCorruptedTotemEntry(uint32 entry);
    bool IsCorruptedTotem(Unit const* unit);
    Unit* FindCorruptedTotem(PlayerbotAI* botAI);
}

#endif
