/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDZULAMANHELPERS_H_
#define _PLAYERBOT_RAIDZULAMANHELPERS_H_

#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace ZulAmanHelpers
{
    enum class ZulAmanSpells : uint32
    {
        // Akil'zon <Eagle Avatar>
        SPELL_ELECTRICAL_STORM          = 43648,

        // Nalorakk <Bear Avatar>
        SPELL_BEARFORM                  = 42377,

        // Hex Lord Malacrass
        SPELL_HEX_LORD_WHIRLWIND        = 43442,
        SPELL_HEX_LORD_SPELL_REFLECTION = 43443,
        SPELL_UNSTABLE_AFFLICTION       = 43522,

        // Zul'jin
        SPELL_ZULJIN_WHIRLWIND          = 17207,
        SPELL_SHAPE_OF_THE_BEAR         = 42594,
        SPELL_SHAPE_OF_THE_EAGLE        = 42606,
        SPELL_SHAPE_OF_THE_LYNX         = 42607,
        SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,
        // SPELL_CLAW_RAGE              = 43149, // Would require getting Zul'jin's bossai

        // Hunter
        SPELL_MISDIRECTION              = 35079,
    };

    enum class ZulAmanNPCs : uint32
    {
        // Trash
        NPC_AMANI_HEALING_WARD          = 23757,
        NPC_AMANI_PROTECTIVE_WARD       = 23822,

        // Jan'alai <Dragonhawk Avatar>
        NPC_AMANI_DRAGONHAWK_HATCHLING  = 23598,
        NPC_AMANISHI_HATCHER            = 23818,
        NPC_FIRE_BOMB                   = 23920,

        // Halazzi <Lynx Avatar>
        NPC_CORRUPTED_LIGHTNING_TOTEM   = 24224,

        // Hex Lord Malacrass
        NPC_HEX_LORD_MALACRASS          = 24239,
        NPC_ALYSON_ANTILLE              = 24240,
        NPC_THURG                       = 24241,
        NPC_SLITHER                     = 24242,
        NPC_LORD_RAADAN                 = 24243,
        NPC_GAZAKROTH                   = 24244,
        NPC_FENSTALKER                  = 24245,
        NPC_DARKHEART                   = 24246,
        NPC_KORAGG                      = 24247,

        // Zul'jin
        NPC_FEATHER_VORTEX              = 24136,
    };

    enum class ZulAmanObjects : uint32
    {
        GO_FREEZING_TRAP                = 186669,
    };

    // General
    constexpr uint32 ZULAMAN_MAP_ID = 568;
    Position FindSafestNearbyPosition(
        Player* bot, const std::vector<Unit*>& hazards, const Position& center,
        float safeZoneRadius, float hazardRadius, bool requireSafePath);
    bool IsPathSafeFromHazards(
        const Position& start, const Position& end,
        const std::vector<Unit*>& hazards, float hazardRadius);
    bool IsPositionSafeFromHazards(
        float x, float y, const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetAllHazardTriggers(
        Player* bot, uint32 entry, float searchRadius);

    // Akil'zon <Eagle Avatar>
    extern const Position AKILZON_TANK_POSITION;
    extern std::unordered_map<uint32, time_t> akilzonStormTimer;
    bool IsInStormWindow(time_t start, time_t now);
    Player* GetElectricalStormTarget(Player* bot);

    // Nalorakk <Bear Avatar>
    extern const Position NALORAKK_TANK_POSITION;

    // Jan'alai <Dragonhawk Avatar>
    extern const Position JANALAI_TANK_POSITION;
    bool HasFireBombNearby(PlayerbotAI* botAI, Player* bot);
    std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI);

    // Halazzi <Lynx Avatar>
    extern const Position HALAZZI_TANK_POSITION;

    // Zul'jin
    extern const Position ZULJIN_TANK_POSITION;
}

#endif
