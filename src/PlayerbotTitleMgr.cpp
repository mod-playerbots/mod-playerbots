/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <cmath>  // Added for gender choice

enum Titles
{
    /**
     * Classic
     */

    /**
     * PvP
     */

    // Alliance
    PRIVATE = 1,
    CORPORAL = 2,
    SERGEANT = 3,
    MASTER_SERGEANT = 4,
    SERGEANT_MAJOR = 5,
    KNIGHT = 6,
    KNIGHT_LIEUTENANT = 7,
    KNIGHT_CAPTAIN = 8,
    KNIGHT_CHAMPION = 9,
    LIEUTENANT_COMMANDER = 10,
    COMMANDER = 11,
    MARSHAL = 12,
    FIELD_MARSHAL = 13,
    GRAND_MARSHAL = 14,

    // Horde
    SCOUT = 15,
    GRUNT = 16,
    SERGEANT_H = 17,
    SENIOR_SERGEANT = 18,
    FIRST_SERGEANT = 19,
    STONE_GUARD = 20,
    BLOOD_GUARD = 21,
    LEGIONNAIRE = 22,
    CENTURION = 23,
    CHAMPION = 24,
    LIEUTENANT_GENERAL = 25,
    GENERAL = 26,
    WARLORD = 27,
    HIGH_WARLORD = 28,

    /*
     * PvE
     */

    SCARAB_LORD = 46,

    /**
     * The Burning Crusade
     */

    /*
     * PvP
     */

    // Arena

    GLADIATOR = 42,
    DUELIST = 43,
    RIVAL = 44,
    CHALLENGER = 45,
    MERCILESS_GLADIATOR = 62,
    VENGEFUL_GLADIATOR = 71,
    BRUTAL_GLADIATOR = 80,

    /**
     * PvE
     */

    CHAMPION_OF_THE_NAARU = 53,
    OF_THE_SHATTERED_SUN = 63,
    HAND_OF_ADAL = 64,

    /**
     * Wrath of the Lich King
     */

    /**
     * PvP
     */

    BATTLEMASTER = 72,
    ARENA_MASTER = 82,

    // Alliance

    JUSTICAR = 48,

    // Horde

    CONQUEROR = 47,

    /**
     * Seasonal Events
     */

    ELDER = 74,
    FLAME_WARDEN = 75,  // Alliance
    FLAME_KEEPER = 76,  // Horde

    /**
     * Miscellaneous
     */

    THE_EXALTED = 77,   // 40 Exalted reputation
    THE_EXPLORER = 78,  // Explore all areas
    THE_DIPLOMAT = 79,  // Exalted Timbermaw, Sporeggar & Kurenai/Mag'har
    THE_SEEKER = 81,    // Complete 3000 quests
    SALTY = 83,         // Fishing
    CHEF = 84,          // Cooking

    /**
     * Realm first
     */

    /**
     * Unused
     */

    THE_SUPREME = 85,
    OF_THE_TEN_STORMS = 86,
    OF_THE_EMERALD_DREAM = 87,  // Used in retail in Dragonflight only
    PROPHET = 89,
    THE_MALEFIC = 90,
    STALKER = 91,  // Used in retail in Warlords of Draenor only
    OF_THE_EBON_BLADE = 92,
    ARCHMAGE = 93,    // Used in retail in Legion only
    WARBRINGER = 94,  // Used in retail in Legion only
    ASSASSIN = 95,
    GRAND_MASTER_ALCHEMIST = 96,
    GRAND_MASTER_BLACKSMITH = 97,
    IRON_CHEF = 98,
    GRAND_MASTER_ENCHANTER = 99,
    GRAND_MASTER_ENGINEER = 100,
    DOCTOR = 101,
    GRAND_MASTER_ANGLER = 102,

};

class PlayerbotTitleMgr
{
public:
private:
};
