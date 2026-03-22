/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOVEMENTHELPER_H
#define _PLAYERBOT_MOVEMENTHELPER_H

#include "SharedDefines.h"
#include "Unit.h"

namespace ai::movement
{
inline bool IsMovementImpaired(Unit* unit)
{
    return unit && (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->IsRooted() || unit->GetSpeedRate(MOVE_RUN) < 1.0f);
}
}

#endif
