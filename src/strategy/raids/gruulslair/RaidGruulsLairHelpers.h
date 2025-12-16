#ifndef RAID_GRUULSLAIRHELPERS_H
#define RAID_GRUULSLAIRHELPERS_H

#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

namespace GruulsLairHelpers
{
    enum GruulsLairSpells
    {
        // High King Maulgar
        SPELL_WHIRLWIND = 33238,

        // Krosh Firehand
        SPELL_SPELL_SHIELD = 33054,

        // Hunter
        SPELL_MISDIRECTION = 35079,

        // Warlock
        SPELL_BANISH = 18647, // Rank 2

        // Gruul the Dragonkiller
        SPELL_GROUND_SLAM_1 = 33525,
        SPELL_GROUND_SLAM_2 = 39187,
    };

    enum GruulsLairNPCs
    {
        NPC_WILD_FEL_STALKER = 18847,
    };

    bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
    bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
    bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);

    struct Location
    {
        float x, y, z;
    };

    namespace GruulsLairLocations
    {
        extern const Location MaulgarTankPosition;
        extern const Location OlmTankPosition;
        extern const Location BlindeyeTankPosition;
        extern const Location KroshTankPosition;
        extern const Location MaulgarRoomCenter;
        extern const Location GruulTankPosition;
    }
}

#endif
