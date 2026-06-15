/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BWLHELPERS_H
#define _PLAYERBOT_BWLHELPERS_H

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

    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot);
}

#endif //_PLAYERBOT_RAIDBWLHELPERS_H
