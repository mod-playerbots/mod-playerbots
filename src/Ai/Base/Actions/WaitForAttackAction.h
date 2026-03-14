/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WAITFORATTACKACTION_H
#define _PLAYERBOT_WAITFORATTACKACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class WaitForAttackKeepSafeDistanceAction : public MovementAction
{
public:
    WaitForAttackKeepSafeDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "wait for attack keep safe distance") {}

    bool Execute(Event event) override;
};

class SetWaitForAttackTimeAction : public Action
{
public:
    SetWaitForAttackTimeAction(PlayerbotAI* botAI)
        : Action(botAI, "wait for attack time") {}

    bool Execute(Event event) override;
};

#endif
