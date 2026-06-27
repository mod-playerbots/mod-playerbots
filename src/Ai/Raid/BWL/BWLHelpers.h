#ifndef PLAYERBOTS_BWLHELPERS_H
#define PLAYERBOTS_BWLHELPERS_H

#include "Player.h"

namespace BlackwingLairHelpers
{
    enum BlackwingLairSpells
    {
        // General
        SPELL_ONYXIA_SCALE_CLOAK = 22683,

        // Chromaggus
        SPELL_BROOD_AFFLICTION_BRONZE = 23170,
        SPELL_HOURGLASS_SAND = 23645,

        // Nefarian
        SPELL_WILD_MAGIC = 23410
    };

    enum BlackwingLairGameObjects
    {
        // General
        GO_SUPPRESSION_DEVICE = 179784
    };

    enum BlackwingLairNPCs
    {
        // Trash
        NPC_DEATH_TALON_WYRMGUARD = 12460
    };

    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot);
}

#endif //_PLAYERBOT_RAIDBWLHELPERS_H
