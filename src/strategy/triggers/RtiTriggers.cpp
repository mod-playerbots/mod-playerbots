/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RtiTriggers.h"

#include "Playerbots.h"

bool NoRtiTrigger::IsActive()
{
    // Do not auto-react to raid icons while out of combat.
    // Out-of-combat RTI usage (explicit chat commands) is handled by chat triggers,
    // not by this generic trigger.
    if (!bot->IsInCombat())
        return false;

    Unit* target = AI_VALUE(Unit*, "rti target");
    return target != nullptr;
}
