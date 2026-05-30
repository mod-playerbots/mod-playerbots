#ifndef _PLAYERBOT_RAIDBWLHELPERS_H
#define _PLAYERBOT_RAIDBWLHELPERS_H

#include "Player.h"

namespace BlackwingLairHelpers
{
    enum BlackwingLairSpells
    {
        // General
        SPELL_ONYXIA_SCALE_CLOAK = 22683,

        // Chromaggus
        SPELL_BROOD_AFFLICTION_BRONZE = 23170,
        SPELL_HOURGLASS_SAND = 23645
    };

    enum BlackwingLairGameObjects
    {
        // General
        GO_SUPPRESSION_DEVICE = 179784
    };

    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot);
}

#endif //_PLAYERBOT_RAIDBWLHELPERS_H
