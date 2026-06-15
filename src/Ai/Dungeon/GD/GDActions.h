/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GDACTIONS_H
#define _PLAYERBOT_GDACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "GDTriggers.h"

class AvoidPoisonNovaAction : public MovementAction
{
public:
    AvoidPoisonNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid poison nova") {}
    bool Execute(Event event) override;
};

class AttackSnakeWrapAction : public AttackAction
{
public:
    AttackSnakeWrapAction(PlayerbotAI* ai) : AttackAction(ai, "attack snake wrap") {}
    bool Execute(Event event) override;
};

class AvoidWhirlingSlashAction : public MovementAction
{
public:
    AvoidWhirlingSlashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid whirling slash") {}
    bool Execute(Event event) override;
};

#endif
