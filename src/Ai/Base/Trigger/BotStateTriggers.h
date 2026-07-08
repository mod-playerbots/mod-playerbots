/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOTSTATETRIGGERS_H
#define _PLAYERBOT_BOTSTATETRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class CombatStartTrigger : public Trigger
{
public:
    CombatStartTrigger(PlayerbotAI* botAI) : Trigger(botAI, "combat start") {}

    bool IsActive() override;
};

#endif
