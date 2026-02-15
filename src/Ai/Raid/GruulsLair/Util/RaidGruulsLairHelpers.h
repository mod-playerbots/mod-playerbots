#ifndef RAID_GRUULSLAIRHELPERS_H
#define RAID_GRUULSLAIRHELPERS_H

#include "PlayerbotAI.h"

namespace GruulsLairHelpers
{
    enum GruulsLairSpells
    {
        // High King Maulgar
        SPELL_WHIRLWIND     = 33238,

        // Krosh Firehand
        SPELL_SPELL_SHIELD  = 33054,

        // Hunter
        SPELL_MISDIRECTION  = 35079,

        // Gruul the Dragonkiller
        SPELL_GROUND_SLAM_1 = 33525,
        SPELL_GROUND_SLAM_2 = 39187,
    };

    enum GruulsLairNPCs
    {
        NPC_WILD_FEL_STALKER = 18847,
    };

    constexpr uint32 GRUULS_LAIR_MAP_ID = 565;

    bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
    bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
    bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);

    extern const Position MAULGAR_TANK_POSITION;
    extern const Position OLM_TANK_POSITION;
    extern const Position BLINDEYE_TANK_POSITION;
    extern const Position KROSH_TANK_POSITION;
    extern const Position MAULGAR_ROOM_CENTER;
    extern const Position GRUUL_TANK_POSITION;
}

#endif
