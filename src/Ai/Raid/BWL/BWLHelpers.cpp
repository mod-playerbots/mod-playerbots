/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BWLHelpers.h"

namespace BlackwingLairHelpers
{
    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot)
    {
        return go &&
               go->GetEntry() == GO_SUPPRESSION_DEVICE &&
               go->GetDistance(bot) < 15.0f &&
               go->GetGoState() == GO_STATE_READY;
    }
}
