/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITHELPERS_H_
#define _PLAYERBOT_RAIDHYJALSUMMITHELPERS_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace HyjalSummitHelpers
{
    enum class HyjalSummitSpells : uint32
    {
        // Anetheron
        SPELL_INFERNO          = 31299,

        // Kaz'rogal
        SPELL_MARK_OF_KAZROGAL = 31447,

        // Azgalor
        SPELL_RAIN_OF_FIRE     = 31340,
        SPELL_DOOM             = 31347,

        // Archimonde
        SPELL_DOOMFIRE         = 31944, // Damaging part of trail
        SPELL_DOOMFIRE_DOT     = 31969, // DoT after exiting trail
        SPELL_ARCHIMONDE_FEAR  = 31970,
        SPELL_AIR_BURST        = 32014,

        // Hunter
        SPELL_MISDIRECTION     = 35079,

        // Priest
        SPELL_FEAR_WARD        =  6346,
    };

    enum class HyjalSummitNPCs : uint32
    {
        // Anetheron
        NPC_TOWERING_INFERNAL  = 17818,

        // Archimonde
        NPC_DOOMFIRE           = 18095,
    };

    // General
    constexpr uint32 HYJAL_SUMMIT_MAP_ID = 534;
    struct RangedGroups
    {
        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
    };
    RangedGroups GetRangedGroups(Player* bot, PlayerbotAI* botAI);
    std::pair<size_t, size_t> GetBotCircleIndexAndCount(Player* bot, PlayerbotAI* botAI,
                                                        const RangedGroups& groups);

    // Rage Winterchill
    extern const Position WINTERCHILL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron
    extern const Position ANETHERON_TANK_POSITION;
    extern const Position ANETHERON_E_INFERNAL_POSITION;
    extern const Position ANETHERON_W_INFERNAL_POSITION;
    extern std::unordered_map<ObjectGuid, bool> hasReachedAnetheronPosition;
    Player* GetInfernoTarget(Unit* anetheron);
    const Position& GetClosestInfernalTankPosition(Player* bot);

    // Kaz'rogal
    extern const Position KAZROGAL_TANK_TRANSITION_POSITION;
    extern const Position KAZROGAL_TANK_FINAL_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> kazrogalTankStep;
    extern std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

    // Azgalor
    extern const Position AZGALOR_TANK_TRANSITION_POSITION;
    extern const Position AZGALOR_TANK_FINAL_POSITION;
    extern const Position AZGALOR_DOOMGUARD_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> azgalorTankStep;
    struct RainOfFireData
    {
        Position position;
        uint32 spawnTime;
    };
    extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, RainOfFireData>> rainOfFirePosition;
    int GetAzgalorTankStep(PlayerbotAI* botAI, Player* bot);
    bool AnyGroupMemberHasDoom(Player* bot);

    // Archimonde
    struct DoomfireTrailData
    {
        Position position;
        uint32 recordTime;
    };
    extern const Position ARCHIMONDE_INITIAL_POSITION;
    extern std::unordered_map<uint32, std::vector<DoomfireTrailData>> doomfireTrails;
    extern std::unordered_map<ObjectGuid, uint32> doomfireLastSampleTime;
}

#endif
