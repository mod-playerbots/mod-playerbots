/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_UPACTIONS_H
#define _PLAYERBOT_UPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UPTriggers.h"

class AvoidFreezingCloudAction : public MovementAction
{
public:
    AvoidFreezingCloudAction(PlayerbotAI* ai) : MovementAction(ai, "avoid freezing cloud") {}
    bool Execute(Event event) override;
};

class AvoidSkadiWhirlwindAction : public MovementAction
{
public:
    AvoidSkadiWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "avoid skadi whirlwind") {}
    bool Execute(Event event) override;
};

#endif
